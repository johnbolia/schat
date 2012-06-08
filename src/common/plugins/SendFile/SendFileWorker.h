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

#ifndef SENDFILEWORKER_H_
#define SENDFILEWORKER_H_

#include <QTcpServer>

#include "SendFileTransaction.h"
#include "SendFileTask.h"

class QUdpSocket;
class QBuffer;

namespace SendFile {

class Transaction;

class Worker : public QTcpServer
{
  Q_OBJECT

public:
  Worker(quint16 port, QObject *parent = 0);

public slots:
  void addTask(const QVariantMap &data);

private slots:
  void handshake(const QByteArray &id);

protected:
  void incomingConnection(int socketDescriptor);

private:
  QHash<QByteArray, SendFileTask> m_tasks; ///< Задачи по передаче файлов, по одной на файл.
  quint16 m_port;                          ///< Порт для передачи данных.
};

} // namespace SendFile

#endif /* SENDFILEWORKER_H_ */
