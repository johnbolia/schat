/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2013 IMPOMEZIA <schat@impomezia.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Ch.h"
#include "cores/Core.h"
#include "feeds/FeedEvents.h"
#include "feeds/FeedStrings.h"
#include "feeds/FeedStorage.h"
#include "JSON.h"
#include "net/PacketReader.h"
#include "net/packets/FeedNotice.h"
#include "net/packets/Notice.h"
#include "net/SimpleID.h"
#include "NodeFeeds.h"
#include "NodeNotify.h"
#include "sglobal.h"
#include "Sockets.h"
#include "Storage.h"

#define SCHAT_CHECK_ACL(x) CheckResult result = check(x);   \
                           if (result.status != Notice::OK) \
                             return result.status;          \


NodeFeeds::NodeFeeds(Core *core)
  : NodeNoticeReader(Notice::FeedType, core)
  , m_event(0)
  , m_packet(0)
{
}


bool NodeFeeds::read(PacketReader *reader)
{
  if (SimpleID::typeOf(reader->sender()) != SimpleID::UserId)
    return false;

  m_user = Ch::channel(reader->sender(), SimpleID::UserId);
  if (!m_user)
    return false;

  m_channel = Ch::channel(reader->dest(), SimpleID::typeOf(reader->dest()));
  if (!m_channel)
    return false;

  FeedNotice packet(m_type, reader);
  m_packet = &packet;
  m_event  = new FeedEvent(reader->dest(), reader->sender(), m_packet->command());

  const QString &method = m_packet->command();
  int status            = Notice::NotImplemented;

  if (method == FEED_METHOD_GET)
    status = get();
  else if (method == FEED_METHOD_PUT || method == FEED_METHOD_POST || method == FEED_METHOD_DELETE)
    status = query(method);

  if (status != Notice::OK)
    reply(status);

  m_event->status = status;
  FeedEvents::start(m_event);

  return false;
}


NodeFeeds::CheckResult::CheckResult(const QString &text)
  : status(Notice::OK)
{
  if (!text.isEmpty()) {
    QPair<QString, QString> split = FeedNotice::split(text);
    name = split.first;
    path = split.second;
  }

  if (name.isEmpty())
    status = Notice::BadRequest;
}


/*!
 * Обработка запроса пользователя на создание нового фида.
 *
 * Эта функция вызывается \b post запросом содержащим только имя фида.
 */
FeedReply NodeFeeds::add()
{
  FeedReply reply(Notice::OK);
  const QString &name = m_packet->text();
  if (name.contains(LC('*')) || name.contains(LC('/')) || name.contains(LC(' ')))
    return Notice::BadRequest;

  FeedPtr feed = m_channel->feed(name, false);
  if (!feed) {
    feed = m_channel->feed(name, true, false);
    if (!feed)
      return Notice::InternalError;

    feed->head().acl().add(m_user->id());
    reply.status = FeedStorage::save(feed);
  }
  else
    FeedStorage::clone(feed);

  reply.status = FeedStorage::save(feed);
  if (reply.status == Notice::OK)
    reply.date = feed->head().date();

  return reply;
}


FeedReply NodeFeeds::del(const CheckResult &result)
{
  if (result.path.isEmpty()) {
    if (!m_channel->canEdit(m_user))
      return Notice::Forbidden;

    if (result.name == FEED_NAME_ACL)
      return Notice::BadRequest;

    FeedStorage::remove(result.feed);
    m_channel->feeds().remove(m_packet->text());
    return Notice::OK;
  }

  return result.feed->del(result.path, m_user.data());
}


FeedReply NodeFeeds::post(CheckResult &result)
{
  if (result.path.isEmpty()) {
    FeedReply reply = add();
    if (reply.status == Notice::OK) {
      result.feed = m_channel->feed(result.name, false);
      if (!result.feed)
        return Notice::InternalError;
    }

    return reply;
  }
  else if (result.feed)
    return result.feed->post(result.path, m_packet->json(), m_user.data());

  return Notice::NotFound;
}


/*!
 * Получение тела фида или обработка \b get запроса к данным фида.
 */
int NodeFeeds::get()
{
  SCHAT_CHECK_ACL(Acl::Read)

  if (result.name == LS("*"))
    return headers();

  if (!result.path.isEmpty())
    return get(result.feed, result.path);

  if (m_packet->date() == result.feed->head().date())
    return Notice::NotModified;

  FeedPacket packet = FeedNotice::reply(*m_packet, Feed::merge(result.name, result.feed->feed(m_user.data())));
  packet->setDate(result.feed->head().date());
  packet->setCommand(LS("feed"));

  Core::send(packet);
  return Notice::OK;
}


/*!
 * Обработка \b get запроса к данным фида.
 */
int NodeFeeds::get(FeedPtr feed, const QString &request)
{
  const FeedReply reply = feed->get(request, m_packet->json(), m_user.data());
  if (reply.status != Notice::OK)
    return reply.status;

  FeedPacket packet = FeedNotice::reply(*m_packet, reply.json);
  if (reply.date)
    packet->setDate(reply.date);

  Core::send(packet);
  if (!reply.packets.isEmpty())
    Core::send(reply.packets);

  return Notice::OK;
}


/*!
 * Получение заголовков фидов для одиночного пользователя.
 *
 * В случае использования плагина "Raw Feeds" эта функция вызывается командой:
 * /feeds.
 */
int NodeFeeds::headers()
{
  if (m_packet->json().value(LS("compact"), true).toBool())
    m_core->send(m_user->sockets(), FeedNotice::reply(*m_packet, m_channel->feeds().f(m_user.data())));
  else
    m_core->send(m_user->sockets(), FeedNotice::reply(*m_packet, m_channel->feeds().headers(m_user.data())));

  return Notice::OK;
}


/*!
 * Обработка \b put, \b post или \b delete запросов.
 */
int NodeFeeds::query(const QString &verb)
{
  SCHAT_CHECK_ACL(Acl::Write)

  FeedReply reply(Notice::InternalError);
  if (verb == FEED_METHOD_PUT)
    reply = result.feed->put(result.path, m_packet->json(), m_user.data());
  else if (verb == FEED_METHOD_POST)
    reply = post(result);
  else if (verb == FEED_METHOD_DELETE)
    reply = del(result);

  if (reply.status != Notice::OK)
    return reply.status;

  if (reply.date)
    FeedStorage::save(result.feed, reply.date);

  int options = m_packet->json().value(LS("options")).toInt();
  if (options & Feed::Echo)
    reply.json[LS("value")] = m_packet->json().value(LS("value"));

  FeedPacket packet = FeedNotice::reply(*m_packet, reply.json);
  packet->setDate(result.feed->head().date());

  if (options & Feed::Share)
    m_core->send(m_user->sockets(), packet);
  else
    Core::send(packet);

  if (reply.date && options & Feed::Broadcast)
    broadcast(result.feed, true);

  NodeNotify::start(NotifyItem::FeedModified, 0, m_channel->id(), result.name);
  return Notice::OK;
}


/*!
 * Базовая проверка корректности запроса к фиду и проверка прав доступа.
 */
NodeFeeds::CheckResult NodeFeeds::check(int acl)
{
  CheckResult result(m_packet->text());
  m_event->name = result.name;
  m_event->path = result.path;

  if (result.status != Notice::OK)
    return result;

  if (!m_channel->canRead(m_user)) {
    result.status = Notice::Forbidden;
    return result;
  }

  result.feed = m_channel->feed(result.name, false);
  if (!result.feed) {
    if (m_packet->command() == FEED_METHOD_GET && result.name == LS("*")) {
      result.status = Notice::OK;
    }
    else if (m_packet->command() == FEED_METHOD_POST && result.path.isEmpty()) {
      if (!m_channel->canEdit(m_user))
        result.status = Notice::Forbidden;
    }
    else
      result.status = Notice::NotFound;
  }
  else if (!result.feed->can(m_user.data(), static_cast<Acl::ResultAcl>(acl)))
    result.status = Notice::Forbidden;

  return result;
}


/*!
 * Отравка заголовка фида всем если он публично доступен для чтения.
 */
void NodeFeeds::broadcast(FeedPtr feed, bool echo)
{
  QVariantMap json = Feed::merge(LS("f"), feed->head().f());
  if (json.isEmpty())
    return;

  FeedPacket packet = FeedNotice::reply(*m_packet, json);
  packet->setDest(m_channel->id());
  packet->setCommand(FEED_METHOD_GET);
  packet->setText(LS("*"));
  m_core->send(Sockets::all(m_channel, m_user, echo), packet);
}


void NodeFeeds::reply(int status)
{
  m_core->send(m_user->sockets(), FeedNotice::reply(*m_packet, status));
}
