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

#ifndef MASTERNODE_H_
#define MASTERNODE_H_

#include "cores/GenericCore.h"

class MasterNode : public GenericCore
{
  Q_OBJECT

public:
  MasterNode(QObject *parent = 0);
  inline bool isSlave(const QByteArray &id) { return m_slaves.contains(id); }
  void addSlave(const QByteArray &id);

protected:
  bool checkPacket();
  void socketReleaseEvent(SocketReleaseEvent *event);

private:
  QList<QByteArray> m_slaves; /// Список вторичных серверов.
};

#endif /* MASTERNODE_H_ */
