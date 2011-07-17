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

#include <QDebug>

#include <QApplication>
#include <QDir>
#include <QPluginLoader>
#include <QStringList>

#include "ChatCore.h"
#include "ChatSettings.h"
#include "net/SimpleClient.h"
#include "Plugins.h"
#include "plugins/ClientInterface.h"
#include "plugins/CoreInterface.h"

Plugins::Plugins(QObject *parent)
  : QObject(parent)
{
}


void Plugins::load()
{
  foreach (const QString &path, QApplication::libraryPaths())
    load(path);
}


void Plugins::load(const QString &path)
{
  qDebug() << path;

  QDir dir(path);

  foreach (QString fileName, dir.entryList(QDir::Files)) {
    QPluginLoader pluginLoader(dir.absoluteFilePath(fileName));
    QObject *plugin = pluginLoader.instance();

    if (plugin) {
      CoreInterface *core = qobject_cast<CoreInterface *> (plugin);
      if (!core)
        continue;

      qDebug() << core->id() << core->name();

      ClientInterface *client = qobject_cast<ClientInterface *> (plugin);
      if (!client)
        continue;

      client->create(ChatCore::i()->client(), ChatCore::i()->settings());
      m_clientPlugins.append(client);
    }
  }
}
