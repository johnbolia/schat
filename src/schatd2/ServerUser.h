/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2011 IMPOMEZIA <schat@impomezia.com>
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

#ifndef SERVERUSER_H_
#define SERVERUSER_H_

#include <QSharedPointer>

#include "plugins/NodeHooks.h"
#include "User.h"

class AuthRequest;
class AuthRequestData;
class QSqlQuery;

class SCHAT_EXPORT ServerUser : public User
{
public:
  ServerUser(const QByteArray &id);
  ServerUser(const QString &normalNick, const QByteArray &id, const AuthRequestData &data, quint64 socketId);
  ServerUser(const User *user);
  ~ServerUser();
  inline bool isOnline() const { return m_online; }
  inline QByteArray cookie() const { return m_cookie; }
  inline QByteArray uniqueId() const { return m_uniqueId; }
  inline qint64 key() const { return m_key; }
  inline QString normalNick() const { return m_normalNick; }
  inline quint64 socketId() const { return m_socketId; }
  inline void setCookie(const QByteArray &cookie) { m_cookie = cookie; }
  inline void setKey(qint64 key) { m_key = key; }
  inline void setNormalNick(const QString &nick) { m_normalNick = nick; }
  inline void setOnline(bool online) { m_online = online; }

private:
  bool m_online;             ///< true если пользователь в сети.
  QByteArray m_cookie;       ///< Cookie.
  QByteArray m_uniqueId;     ///< Уникальный идентификатор пользователя.
  qint64 m_key;              ///< Ключ в таблице users.
  QString m_normalNick;      ///< Нормализованный ник.
  quint64 m_socketId;        ///< Идентификатор сокета.
};

typedef QSharedPointer<ServerUser> ChatUser;


class UserHook : public NodeHook
{
public:
  UserHook(ChatUser user)
  : NodeHook(AcceptedUser)
  , user(user)
  {}

  ChatUser user;
};


class UserReadyHook : public NodeHook
{
public:
  UserReadyHook(ChatUser user, const QString &type, const QByteArray &dest)
  : NodeHook(UserReady)
  , user(user)
  , dest(dest)
  , type(type)
  {}

  ChatUser user;
  QByteArray dest;
  QString type;
};

#endif /* SERVERUSER_H_ */
