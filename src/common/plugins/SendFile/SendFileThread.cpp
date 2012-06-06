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
#include <QDateTime>

#include "SendFileThread.h"
#include "SendFileWorker.h"

#define TIMESTAMP QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toLatin1().constData()

namespace SendFile {

Thread::Thread(quint16 port, QObject *parent)
  : QThread(parent)
  , m_port(port)
  , m_worker(0)
{
  qDebug() << TIMESTAMP << "Thread::Thread()                   " << currentThread();

  connect(this, SIGNAL(ready()), SLOT(workerReady()));
}


/*!
 * Добавление транзакции на исполнение.
 *
 * В случае если поток не был запущен, инициализируется его запуск и транзакция добавляется в очередь ожидающую запуск потока.
 */
void Thread::add(SendFileTransaction transaction)
{
  if (!isRunning()) {
    m_pending.append(transaction);
    start();
    return;
  }

  emit addTask(transaction->toMap());
}


void Thread::run()
{
  m_worker = new Worker(m_port);

  emit ready();
  exec();
}


void Thread::workerReady()
{
  connect(this, SIGNAL(addTask(const QVariantMap &)), m_worker, SLOT(addTask(const QVariantMap &)));

  while (!m_pending.isEmpty())
    emit addTask(m_pending.takeFirst()->toMap());
}

} // namespace SendFile
