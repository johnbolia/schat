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

#include "debugstream.h"

#include "client/SimpleClient.h"
#include "client/SimpleClient_p.h"
#include "net/PacketReader.h"
#include "net/packets/auth.h"
#include "net/packets/ChannelNotice.h"
#include "net/packets/Notice.h"
#include "sglobal.h"

SimpleClientPrivate::SimpleClientPrivate()
  : cookieAuth(false)
{
}


SimpleClientPrivate::~SimpleClientPrivate()
{
}


bool SimpleClientPrivate::authReply(const AuthReply &reply)
{
  AbstractClientPrivate::authReply(reply);

  if (reply.status == Notice::OK) {
//    ClientChannel channel = ClientChannel(new Channel(SimpleID::setType(SimpleID::ChannelId, userId), QLatin1String("~") + user->nick()));
//    addChannel(channel);

    account.clear();
    password.clear();
    return true;
  }

  m_authError = AuthError(authType, reply.status);

  if (reply.status == Notice::NickAlreadyUse)
    return false;

  if (authType == AuthRequest::Cookie && (reply.status == Notice::NotImplemented || reply.status == Notice::Forbidden)) {
    cookieAuth = false;
    return false;
  }

  if (authType == AuthRequest::Password)
    setClientState(SimpleClient::ClientError);

  return false;
}


/*!
 * Чтение пакетов типа Protocol::NoticePacket.
 * \sa Notice.
 */
bool SimpleClientPrivate::notice()
{
  Q_Q(SimpleClient);
  quint16 type = reader->get<quint16>();
  m_notice = 0;

  if (type == Notice::GenericType) {
    Notice notice(type, reader);
    if (!notice.isValid())
      return false;

    m_notice = &notice;
    QString cmd = notice.command();

//    if (cmd == "leave")
//      removeUser(reader->sender());

    emit(q->notice(notice));
  }
  else {
    emit(q->notice(type));

//    ChannelPacket notice(type, reader);
//    if (!notice.isValid())
//      return false;
//
//    m_notice = &notice;
//    QString cmd = notice.command();

//    if (cmd == "channel")
//      channel();

//    emit(q->notice(notice));
  }

  return true;
}


SimpleClient::SimpleClient(QObject *parent)
  : AbstractClient(*new SimpleClientPrivate(), parent)
{
}


SimpleClient::SimpleClient(SimpleClientPrivate &dd, QObject *parent)
  : AbstractClient(dd, parent)
{
}


SimpleClient::~SimpleClient()
{
}


const AuthError& SimpleClient::authError() const
{
  Q_D(const SimpleClient);
  return d->m_authError;
}


const QString &SimpleClient::account() const
{
  Q_D(const SimpleClient);
  return d->account;
}


void SimpleClient::leave()
{
  send(ChannelNotice::request(channelId(), channelId(), LS("quit")));

  AbstractClient::leave();
}


void SimpleClient::setAccount(const QString &account, const QString &password)
{
  Q_D(SimpleClient);
  d->account = account;

  if (!password.isEmpty())
    d->password = SimpleID::password(password);
  else
    d->password.clear();
}


void SimpleClient::setCookieAuth(bool allow)
{
  Q_D(SimpleClient);
  d->cookieAuth = allow;
}


/*!
 * Формирование пакета запроса авторизации.
 */
void SimpleClient::requestAuth()
{
  Q_D(SimpleClient);

  d->authType = AuthRequest::Anonymous;

  if (!d->account.isEmpty() && SimpleID::typeOf(d->password) == SimpleID::PasswordId)
    d->authType = AuthRequest::Password;
  else if (d->cookieAuth && !d->cookie.isEmpty())
    d->authType = AuthRequest::Cookie;

  AuthRequest data(d->authType, d->url.toString(), d->channel.data());
  data.uniqueId = d->uniqueId;
  data.cookie = d->cookie;
  data.id = d->authId;
  data.account = d->account;
  data.password = d->password;
  send(data.data(d->sendStream));
}


/*!
 * Обработка пакетов.
 */
void SimpleClient::newPacketsImpl()
{
  Q_D(SimpleClient);
  SCHAT_DEBUG_STREAM(this << "newPacketsImpl()" << d->readQueue.size())

  while (!d->readQueue.isEmpty()) {
    d->readBuffer = d->readQueue.takeFirst();
    PacketReader reader(d->readStream);
    d->reader = &reader;

    switch (reader.type()) {
      case Protocol::AuthReplyPacket:
        d->authReply(AuthReply(d->reader));
        break;

      case Protocol::NoticePacket:
        d->notice();
        break;

      default:
        break;
    }
  }

  emit idle();
}