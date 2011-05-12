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

#include <QAction>
#include <QVBoxLayout>

#include "ui/tabs/PrivateTab.h"
#include "ui/TabWidget.h"
#include "ui/UserUtils.h"
#include "User.h"

PrivateTab::PrivateTab(User *user, TabWidget *parent)
  : ChatViewTab(user->id(), PrivateType, parent)
  , m_user(new User(user))
{
  QVBoxLayout *mainLay = new QVBoxLayout(this);
  mainLay->addWidget(m_chatView);
  mainLay->setMargin(0);
  mainLay->setSpacing(0);

  m_icon = UserUtils::icon(user);
  m_action->setText(m_user->nick());
}


bool PrivateTab::update(User *user)
{
  m_user->setNick(user->nick());
  m_user->setRawGender(user->rawGender());
  m_action->setText(m_user->nick());
  m_icon = UserUtils::icon(m_user);

  int index = m_tabs->indexOf(this);
  if (index == -1)
    return false;

  m_tabs->setTabText(index, m_user->nick());
  setOnline(true);
  return true;
}

