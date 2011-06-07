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

#ifndef STORAGE_H_
#define STORAGE_H_

#include <QByteArray>
#include <QHash>

#include "ServerUser.h"

class DataBase;
class ServerChannel;
class ServerData;
class ServerSettings;

class Storage : public QObject
{
  Q_OBJECT

public:
  Storage(QObject *parent = 0);
  ~Storage();
  inline static Storage *i() { return m_self; }
  int start();

  // user management.
  bool add(ChatUser user);
  bool remove(ChatUser user);
  bool removeUserFromChannel(const QByteArray &userId, const QByteArray &channelId);
  ChatUser user(const QString &nick, bool normalize) const;
  inline ChatUser user(const QByteArray &id) const { return m_users.value(id); }
  QByteArray makeUserId(int type, const QByteArray &clientId) const;
  QList<quint64> socketsFromUser(ChatUser usr);
  void rename(ChatUser user);

  // channel management.
  bool removeChannel(const QByteArray &id);
  inline ServerChannel* channel(const QByteArray &id) const { return m_channels.value(id); }
  QList<quint64> socketsFromChannel(ServerChannel *channel);
  ServerChannel* addChannel(const QString &name, bool permanent = false);
  ServerChannel* channel(const QString &name, bool normalize) const;

  inline ServerData *serverData() { return m_serverData; }
  inline ServerSettings *settings() { return m_settings; }
  QByteArray session() const;
  QString normalize(const QString &text) const;

private:
  QByteArray makeChannelId(const QString &name);

  DataBase *m_db;                                ///< База данных сервера.
  QHash<QByteArray, ChatUser> m_sessions;        ///< Таблица сессий.
  QHash<QByteArray, ChatUser> m_users;           ///< Таблица пользователей.
  QHash<QByteArray, ServerChannel*> m_channels;  ///< Таблица каналов.
  QHash<QChar, QChar> m_normalize;               ///< Карта замены символов при нормализации ника.
  QHash<QString, ChatUser> m_nicks;              ///< Таблица ников.
  QHash<QString, ServerChannel*> m_channelNames; ///< Имена каналов.
  ServerData *m_serverData;                      ///< Информация о сервере.
  ServerSettings *m_settings;                    ///< Настройки сервера.
  static Storage *m_self;                        ///< Указатель на себя.
};

#endif /* STORAGE_H_ */
