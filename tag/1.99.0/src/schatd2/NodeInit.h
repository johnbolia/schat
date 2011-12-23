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

#ifndef NODEINIT_H_
#define NODEINIT_H_

#include <QObject>

class Core;
class NodePlugins;
class Storage;
class WorkerThread;

/*!
 * Загрузчик сервера.
 */
class NodeInit : public QObject
{
  Q_OBJECT

public:
  NodeInit(QObject *parent = 0);
  void quit();

public slots:
  void start();

private:
  Core *m_core;           ///< Указатель на объект Core.
  NodePlugins *m_plugins; ///< Загрузчик плагинов.
  Storage *m_storage;     ///< Хранилище данных.
  WorkerThread *m_thread; ///< Поток обслуживающий подключения.
};

#endif /* NODEINIT_H_ */
