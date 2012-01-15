/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2012 IMPOMEZIA <schat@impomezia.com>
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

#include <QDebug>
#include "debugstream.h"

#include "Ch.h"
#include "cores/Core.h"
#include "events.h"
#include "net/PacketReader.h"
#include "net/packets/ChannelNotice.h"
#include "net/packets/Notice.h"
#include "NodeChannels.h"
#include "Normalize.h"
#include "sglobal.h"
#include "Sockets.h"
#include "Storage.h"

NodeChannels::NodeChannels(Core *core)
  : NodeNoticeReader(Notice::ChannelType, core)
{
}


bool NodeChannels::read(PacketReader *reader)
{
  if (SimpleID::typeOf(reader->sender()) != SimpleID::UserId)
    return false;

  m_user = Ch::channel(reader->sender(), SimpleID::UserId);
  if (!m_user)
    return false;

  ChannelNotice packet(m_type, reader);
  m_packet = &packet;

  QString cmd = m_packet->command();
  qDebug() << "NodeChannels::read" << cmd;

  int status = Notice::NotImplemented;

  if (cmd == LS("info"))
    return info();

  else if (cmd == LS("join"))
    return join();

  else if (cmd == LS("-"))
    return part();

  else if (cmd == LS("quit"))
    return quit();

  else if (cmd == LS("update"))
    status = update();

  else if (cmd == LS("name"))
    return name();

  if (status == Notice::OK)
    return false;

  reply(status);
  return false;
}


void NodeChannels::acceptImpl(ChatChannel user, const AuthResult & /*result*/, QList<QByteArray> &packets)
{
  packets.append(ChannelNotice::channel(user, user, m_core->sendStream()));
}


void NodeChannels::addImpl(ChatChannel user)
{
  m_core->send(Sockets::all(user), ChannelNotice::channel(user, user->id(), m_core->sendStream(), LS("info")));
}


void NodeChannels::releaseImpl(ChatChannel user, quint64 socket)
{
  Q_UNUSED(socket);

  if (user->sockets().size())
    return;

  m_core->send(Sockets::all(user), ChannelNotice::request(user->id(), user->id(), LS("quit"), m_core->sendStream()));

  QList<QByteArray> channels = user->channels().all();
  foreach (QByteArray id, channels) {
    ChatChannel channel = Ch::channel(id);
    if (channel && channel->type() == SimpleID::ChannelId) {
      channel->channels().remove(user->id());
      user->channels().remove(channel->id());
      Ch::gc(channel);
    }
  }
}


/*!
 * Обработка запроса пользователем информации о канале.
 */
bool NodeChannels::info()
{
  if (m_packet->channels().isEmpty())
    return false;

  QList<QByteArray> packets;
  foreach (QByteArray id, m_packet->channels()) {
    ChatChannel channel = Ch::channel(id, SimpleID::typeOf(id));
    if (channel)
      packets += ChannelNotice::channel(channel, m_user, m_core->sendStream(), LS("info"));
  }

  if (packets.isEmpty())
    return false;

  m_core->send(m_user->sockets(), packets);
  return false;
}


/*!
 * Обработка запроса пользователя подключения к каналу.
 */
bool NodeChannels::join()
{
  ChatChannel channel;

  /// Если идентификатор канала корректный функция пытается получить его по этому идентификатору.
  int type = SimpleID::typeOf(m_packet->channelId());
  if (type != SimpleID::InvalidId)
    channel = Ch::channel(m_packet->channelId(), type);

  /// Если канал не удалось получить по идентификатору, будет произведена попытка создать обычный канал по имени.
  if (!channel)
    channel = Ch::channel(m_packet->text());

  if (!channel)
    return false;

  bool notify = !channel->channels().all().contains(m_user->id());
  channel->channels() += m_user->id();
  m_user->channels()  += channel->id();

  m_core->send(m_user->sockets(), ChannelNotice::channel(channel, m_user, m_core->sendStream()));

  /// В случае необходимости всем пользователям в канале будет разослано уведомление в входе нового пользователя.
  if (notify && channel->channels().all().size() > 1 && channel->type() == SimpleID::ChannelId)
    m_core->send(Sockets::channel(channel), ChannelNotice::channel(m_user, channel->id(), m_core->sendStream(), LS("+")));

  return false;
}


bool NodeChannels::name()
{
  if (!Channel::isValidName(m_packet->text()))
    return false;

  ChatChannel channel = Ch::channel(m_packet->channelId(), SimpleID::typeOf(m_packet->channelId()));
  if (!channel)
    return false;

  qDebug() << channel;

  if (channel->name() == m_packet->text())
    return false;

  if (!Ch::rename(channel, m_packet->text()))
    return false;

//  qDebug() << Sockets::channel(channel);
//  m_core->send(Sockets::channel(channel), ChannelPacket::channel(channel, channel, m_core->sendStream(), LS("info")));
  return false;
}


/*!
 * Обработка отключения пользователя от канала.
 */
bool NodeChannels::part()
{
  ChatChannel channel = Ch::channel(m_packet->channelId(), SimpleID::typeOf(m_packet->channelId()));
  if (!channel)
    return false;

  m_user->channels().remove(channel->id());

  if (!channel->channels().all().contains(m_user->id()))
    return false;

  m_core->send(Sockets::channel(channel), ChannelNotice::request(m_user->id(), channel->id(), LS("-"), m_core->sendStream()));
  channel->channels().remove(m_user->id());

  Ch::gc(channel);

  return false;
}


bool NodeChannels::quit()
{
  m_core->send(QList<quint64>() << m_core->packetsEvent()->socket(), QByteArray(), NewPacketsEvent::KillSocketOption);
  return false;
}


/*!
 * Обработка получения обновлённой информации о пользователе.
 */
int NodeChannels::update()
{
  if (!Channel::isValidName(m_packet->text()))
    return Notice::BadRequest;

  if (m_packet->channelStatus() == Status::Offline) {
    m_core->send(QList<quint64>() << m_core->packetsEvent()->socket(), QByteArray(), NewPacketsEvent::KillSocketOption);
    return Notice::OK;
  }

  int updates = 0;

  if (m_user->name() != m_packet->text()) {
    if (!Ch::rename(m_user, m_packet->text()))
      return Notice::ObjectAlreadyExists;
  }

  if (m_user->gender().raw() != m_packet->gender()) {
    m_user->gender() = m_packet->gender();
    updates++;
  }

  if (m_user->status().value() != m_packet->channelStatus()) {
    m_user->status() = m_packet->channelStatus();
    updates++;
  }

  if (!updates)
    return Notice::BadRequest;

  m_core->send(Sockets::all(m_user, true), ChannelNotice::channel(m_user, m_user->id(), m_core->sendStream(), LS("info")));
  return Notice::OK;
}


void NodeChannels::reply(int status)
{
  m_core->send(m_user->sockets(), ChannelNotice::reply(*m_packet, status, m_core->sendStream()));
}
