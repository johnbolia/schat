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

#ifndef DATABASE_H_
#define DATABASE_H_

#include <QObject>
#include <QSqlDatabase>

#include "ServerUser.h"

class ServerSettings;

class DataBase : public QObject
{
  Q_OBJECT

public:
  DataBase(QObject *parent = 0);
  ChatUser user(const QByteArray &id);
  ChatUser user(qint64 key);
  int start();
  qint64 add(ChatUser user);
  void update(ChatUser user);

private:
  QSqlDatabase m_db;
  ServerSettings *m_settings;
};

#endif /* DATABASE_H_ */
