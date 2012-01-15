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

#include <QVBoxLayout>

#include "client/ChatClient.h"
#include "client/ClientChannels.h"
#include "messages/ServiceMessage.h"
#include "ui/ChatIcons.h"
#include "ui/tabs/ChatView.h"
#include "ui/tabs/PrivateTab.h"
#include "ui/TabWidget.h"

PrivateTab::PrivateTab(ClientChannel channel, TabWidget *parent)
  : ChannelBaseTab(channel, PrivateType, parent)
  , m_joined(true)
{
  QVBoxLayout *mainLay = new QVBoxLayout(this);
  mainLay->addWidget(m_chatView);
  mainLay->setMargin(0);
  mainLay->setSpacing(0);

  setText(m_channel->name());

  ChatClient::channels()->join(id());

  connect(ChatClient::channels(), SIGNAL(channel(const ChannelInfo &)), SLOT(channel(const ChannelInfo &)));
  connect(ChatClient::channels(), SIGNAL(quit(const QByteArray &)), SLOT(quit(const QByteArray &)));
  connect(ChatClient::i(), SIGNAL(online()), SLOT(online()));
}


bool PrivateTab::bindMenu(QMenu *menu)
{
  Q_UNUSED(menu)
//  UserMenu *builder = new UserMenu(m_user, this);
//  builder->bind(menu);
  return true;
}


void PrivateTab::channel(const ChannelInfo &info)
{
  if (info.id() != id())
    return;

  if (m_joined)
    return;

  m_chatView->add(ServiceMessage::joined(id()));
  m_joined = true;
}


void PrivateTab::online()
{
  m_chatView->add(ServiceMessage::joined(id()));
}


void PrivateTab::quit(const QByteArray &user)
{
  if (id() != user)
    return;

  reload();

  m_chatView->add(ServiceMessage::quit(user));
  m_joined = false;
}