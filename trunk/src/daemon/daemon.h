/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008 IMPOMEZIA (http://impomezia.net.ru)
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DAEMON_H_
#define DAEMON_H_

#include <QTcpServer>
#include <QHash>

class ChannelLog;
class DaemonSettings;
class UserUnit;

class Daemon : public QObject
{
  Q_OBJECT

public:
  Daemon(QObject *parent = 0);
  bool start();
  
signals:
  void newUser(const QStringList &list, bool echo);
  void userLeave(const QString &nick, const QString &bye, bool echo);

public slots:
  void incomingConnection();
  
private slots:
  void greeting(const QStringList &list);
  void userLeave(const QString &nick);

private:
  ChannelLog *m_channelLog;
  ChannelLog *m_privateLog;
  DaemonSettings *m_settings;
  QHash<QString, UserUnit *> m_users;
  QTcpServer m_server;
};

#endif /*DAEMON_H_*/
