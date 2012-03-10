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

#include <QApplication>
#include <QFile>

#include <QTextDocument>

#include "ChatAlerts.h"
#include "ChatNotify.h"
#include "ChatUrls.h"
#include "client/ChatClient.h"
#include "client/ClientChannels.h"
#include "hooks/ChannelMenu.h"
#include "JSON.h"
#include "messages/ServiceMessage.h"
#include "net/SimpleID.h"
#include "sglobal.h"
#include "ui/ChatIcons.h"
#include "ui/tabs/ChannelBaseTab.h"
#include "ui/tabs/ChatView.h"
#include "ui/TabWidget.h"
#include "WebBridge.h"

ChannelBaseTab::ChannelBaseTab(ClientChannel channel, TabType type, TabWidget *parent)
  : AbstractTab(channel->id(), type, parent)
  , m_joined(true)
  , m_channel(channel)
  , m_alerts(0)
{
  QString file = QApplication::applicationDirPath() + "/styles/test/html/" + page();
  if (QFile::exists(file))
    file = QUrl::fromLocalFile(file).toString();
  else
    file = "qrc:/html/" + page();

  m_chatView = new ChatView(channel->id(), file, this);
  setIcon(channelIcon());

  connect(ChatClient::channels(), SIGNAL(channel(const ChannelInfo &)), SLOT(channel(const ChannelInfo &)));
  connect(ChatClient::i(), SIGNAL(offline()), SLOT(offline()));
  connect(ChatAlerts::i(), SIGNAL(alert(const Alert &)), SLOT(alert(const Alert &)));
  connect(ChatNotify::i(), SIGNAL(notify(const Notify &)), SLOT(notify(const Notify &)));
}


ChannelBaseTab::~ChannelBaseTab()
{
  ChatClient::channels()->part(id());
}


bool ChannelBaseTab::bindMenu(QMenu *menu)
{
  Hooks::ChannelMenu::bind(menu, m_channel);
  return true;
}


void ChannelBaseTab::alert(bool start)
{
  if (start) {
    m_alerts++;
    ChatAlerts::add(id());
  }
  else {
    m_alerts = 0;
    ChatAlerts::remove(id());
  }

  if (m_alerts > 1)
    return;

  setIcon(channelIcon());
}


void ChannelBaseTab::add(const Message &message)
{
  m_chatView->add(message);
}


void ChannelBaseTab::setOnline(bool online)
{
  if (online) {
    AbstractTab::setOnline(online);
    return;
  }

  m_online = false;
  m_joined = false;
  setIcon(channelIcon());
}


void ChannelBaseTab::alert(const Alert &alert)
{
  if (alert.type() == Alert::PublicMessage || alert.type() == Alert::PrivateMessage) {
    if (alert.tab() != id())
      return;

    if (m_tabs->currentIndex() == m_tabs->indexOf(this) && m_tabs->parentWidget()->isActiveWindow())
      return;

    this->alert();
  }
}


void ChannelBaseTab::channel(const ChannelInfo &info)
{
  if (info.option() != ChannelInfo::Updated)
    rename(info.id());

  if (info.id() != id())
    return;

  reload();

  if (!m_online)
    m_online = true;
}


void ChannelBaseTab::notify(const Notify &notify)
{
  if (notify.type() == Notify::ShowID) {
    if (notify.data().toByteArray() == id())
      add(ServiceMessage::showId(notify.data().toByteArray()));
  }
}


void ChannelBaseTab::offline()
{
  setOnline(false);
  add(ServiceMessage::quit(ChatClient::id()));
}


QIcon ChannelBaseTab::channelIcon() const
{
  if (m_alerts) {
    if (m_channel->type() == SimpleID::ChannelId)
      return SCHAT_ICON(ChannelAlert);

    return ChatIcons::icon(ChatIcons::icon(m_channel, ChatIcons::OfflineStatus), LS(":/images/message-small.png"));
  }
  else
    return ChatIcons::icon(m_channel);
}


/*!
 * Определение базового каркаса страницы в зависимости от типа канала.
 */
QString ChannelBaseTab::page() const
{
  if (SimpleID::typeOf(id()) == SimpleID::UserId)
    return LS("User.html");
  else if (SimpleID::typeOf(id()) == SimpleID::ChannelId)
    return LS("Channel.html");
  else
    return LS("Server.html");
}


void ChannelBaseTab::reload()
{
  setIcon(channelIcon());
  setText(m_channel->name());
}


void ChannelBaseTab::rename(const QByteArray &id)
{
  m_chatView->evaluateJavaScript(LS("Messages.updateChannelName(") + JSON::quote(JSON::generate(WebBridge::channel(id))) + LS(");"));
}
