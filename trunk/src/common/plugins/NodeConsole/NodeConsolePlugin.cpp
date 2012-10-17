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

#include <QtPlugin>

#include "NodeConsolePlugin.h"
#include "NodeConsolePlugin_p.h"

NodeConsoleImpl::NodeConsoleImpl(QObject *parent)
  : NodePlugin(parent)
{
}


NodePlugin *NodeConsolePlugin::create()
{
  m_plugin = new NodeConsoleImpl(this);
  return m_plugin;
}

#if QT_VERSION < 0x050000
  Q_EXPORT_PLUGIN2(NodeConsole, NodeConsolePlugin);
#endif
