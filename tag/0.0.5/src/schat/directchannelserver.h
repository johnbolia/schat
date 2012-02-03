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

#ifndef DIRECTCHANNELSERVER_H_
#define DIRECTCHANNELSERVER_H_

#include <QWidget>
#include <QPointer>

#include "abstracttab.h"
#include "serversocket.h"

class QVBoxLayout;

class DirectChannelServer : public AbstractTab {
  Q_OBJECT

public:
  enum ConnectionState {
    Disconnected,
    Connected,
  };
  
  DirectChannelServer(Profile *p, ServerSocket *s, QWidget *parent = 0);
  virtual ~DirectChannelServer();
  void changeSocket(ServerSocket *s);
  void sendText(const QString &text);
  
signals:
  void newDirectMessage();
  
private slots:
  void newMessage(const QString &nick, const QString &message);
  void removeConnection();
  
private:
  void initSocket();
  
  ConnectionState state;
  Profile *profile;
  QPointer<ServerSocket> socket;
  QVBoxLayout *mainLayout;
};

#endif /*DIRECTCHANNELSERVER_H_*/