/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2009 IMPOMEZIA <schat@impomezia.com>
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

#include <QtCore>

#include "daemonsettings.h"

/*!
 * \brief Конструктор класса DaemonSettings.
 */
DaemonSettings::DaemonSettings(const QString &filename, QObject *parent)
  : AbstractSettings(filename, parent)
{
}


/*!
 * \brief Чтение настроек
 *
 * Сервер принципиально не поддерживает запись настроек.
 */
void DaemonSettings::read()
{
  setBool("ChannelLog",      false);
  setBool("PrivateLog",      false);
  setBool("LocalServer",     true);
  setBool("Network",         false);
  setBool("RootServer",      false);
  setBool("Motd",            true);
  setBool("Stats",           false);
  setInt("ListenPort",       7666);
  setInt("LogLevel",         0);
  setInt("Numeric",          0);
  setInt("MaxUsers",         100);
  setInt("MaxLinks",         10);
  setInt("MaxUsersPerIp",    0);
  setInt("MotdMaxSize",      2048);
  setInt("StatsInterval",    30);
  setString("ListenAddress", "0.0.0.0");
  setString("NetworkFile",   "network.xml");
  setString("Name",          "");
  setString("MotdFile",      "motd.html");
  setString("StatsFile",     "stats.xml");

  AbstractSettings::read();
}
