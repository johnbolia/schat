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

#include "ui/tabs/ChatView.h"
#include "ui/tabs/ChatViewTab.h"

ChatViewTab::ChatViewTab(const QString &url, const QByteArray &id, TabType type, TabWidget *parent)
  : AbstractTab(id, type, parent)
  , m_alerts(0)
{
  Q_UNUSED(url)

  m_chatView = new ChatView(this);
}


void ChatViewTab::alert(bool start)
{
  if (start)
    m_alerts++;
  else
    m_alerts = 0;
}
