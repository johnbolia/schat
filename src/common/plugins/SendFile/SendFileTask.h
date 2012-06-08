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

#ifndef SENDFILETASK_H_
#define SENDFILETASK_H_

#include <QHostAddress>
#include <QObject>
#include <QSharedPointer>
#include <QVariant>

class QFile;
class Socket;

namespace SendFile {

class Transaction;
class Worker;

class Task : public QObject
{
  Q_OBJECT

public:
  Task(Worker *worker, const QVariantMap &data);
  ~Task();
  bool init();
  inline Socket *socket() const           { return m_socket; }
  inline Transaction *transaction() const { return m_transaction; }
  void discovery();
  void setSocket(Socket *socket);

private slots:
  void accepted();
  void rejected();

private:
  void discovery(const QString &host, quint16 port);

  QFile *m_file;               ///< Открытый файл.
  QList<Socket *> m_discovery; ///< Список сокетов находящихся в состоянии поиска.
  Socket *m_socket;            ///< Сокет для передачи данных.
  Transaction *m_transaction;  ///< Транзакция.
  Worker *m_worker;            ///< Указатель на объект Worker.
};

} // namespace SendFile

typedef QSharedPointer<SendFile::Task> SendFileTask;

#endif /* SENDFILETASK_H_ */
