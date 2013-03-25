/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2013 IMPOMEZIA <schat@impomezia.com>
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

#include <QApplication>
#include <QFile>
#include <QTimer>

#include "ChatAlerts.h"
#include "ChatNotify.h"
#include "ChatUrls.h"
#include "client/ChatClient.h"
#include "client/ClientChannels.h"
#include "client/SimpleClient.h"
#include "debugstream.h"
#include "hooks/ChannelMenu.h"
#include "JSON.h"
#include "messages/AlertMessage.h"
#include "messages/ServiceMessage.h"
#include "net/SimpleID.h"
#include "sglobal.h"
#include "ui/AlertsPixmap.h"
#include "ui/ChatIcons.h"
#include "ui/tabs/ChannelBaseTab.h"
#include "ui/tabs/ChatView.h"
#include "ui/tabs/FindWidget.h"
#include "ui/TabWidget.h"
#include "WebBridge.h"

ChannelBaseTab::ChannelBaseTab(ClientChannel channel, const QString &type, TabWidget *parent)
  : AbstractTab(channel->id(), type, parent)
  , m_joined(true)
  , m_channel(channel)
{
  m_options |= CanSendMessage;
  m_options |= CanBePinned;

  QString file = QApplication::applicationDirPath() + "/styles/test/html/" + page();
  if (QFile::exists(file))
    file = QUrl::fromLocalFile(file).toString();
  else
    file = "qrc:/html/" + page();

  m_chatView = new ChatView(channel->id(), file, this);
  m_findWidget = new FindWidget(this);
  m_findWidget->setVisible(false);
  setIcon(channelIcon());

  m_serverId = ChatClient::serverId();

  connect(ChatClient::channels(), SIGNAL(channel(const ChannelInfo &)), SLOT(channel(const ChannelInfo &)));
  connect(ChatClient::io(), SIGNAL(setup()), SLOT(setup()));
  connect(ChatClient::i(), SIGNAL(offline()), SLOT(offline()));
  connect(ChatAlerts::i(), SIGNAL(countChanged(int,int,QByteArray)), SLOT(countChanged(int,int,QByteArray)));
  connect(ChatNotify::i(), SIGNAL(notify(const Notify &)), SLOT(notify(const Notify &)));
  connect(m_findWidget, SIGNAL(find(QString,bool)), SLOT(find(QString,bool)));
  connect(m_findWidget, SIGNAL(hidden()), SLOT(hidden()));

  ChatNotify::start(Notify::ChannelTabCreated, id(), true);
}


ChannelBaseTab::~ChannelBaseTab()
{
  SCHAT_DEBUG_STREAM(this << "~ChannelBaseTab()" << text())

  ChatAlerts::remove(id());
  ChatClient::channels()->part(id());
}


bool ChannelBaseTab::bindMenu(QMenu *menu)
{
  Hooks::ChannelMenu::bind(menu, c(), Hooks::TabScope);
  return true;
}


int ChannelBaseTab::alerts() const
{
  return ChatAlerts::count(id());
}


void ChannelBaseTab::add(const Message &message)
{
  m_chatView->add(message);
}


void ChannelBaseTab::copy()
{
  m_chatView->copy();
}


void ChannelBaseTab::setOnline(bool online)
{
  if (online) {
    if (isForbidden())
      AlertMessage::show(tr("<b>Access denied!</b> You cannot join into this channel."), ALERT_MESSAGE_ERROR, id());

    AbstractTab::setOnline(online);
    return;
  }

  m_online = false;
  m_joined = false;
  setIcon(channelIcon());
}


void ChannelBaseTab::stopAlert()
{
  ChatAlerts::remove(id());
  setIcon(channelIcon());
}


void ChannelBaseTab::channel(const ChannelInfo &info)
{
  if (info.id() != id())
    return;

  reload();

  if (!m_online)
    m_online = true;
}


void ChannelBaseTab::countChanged(int total, int count, const QByteArray &channel)
{
  Q_UNUSED(total)
  Q_UNUSED(count)

  if (id() == channel)
    setIcon(channelIcon());
}


void ChannelBaseTab::find(const QString &text, bool forward)
{
  m_findWidget->setPalette(m_chatView->find(text, forward));

  if (!m_findWidget->isVisible())
    m_findWidget->show();

  if (!m_findWidget->hasFocus())
    m_findWidget->setFocus();
}


void ChannelBaseTab::hidden()
{
  m_chatView->find(QString());
}


void ChannelBaseTab::notify(const Notify &notify)
{
  if (notify.type() == Notify::ShowID) {
    if (notify.data().toByteArray() == id())
      AlertMessage::show(LS("<b>Id:</b> ") + SimpleID::encode(id()), ALERT_MESSAGE_INFO, id());
  }
  else if (notify.type() == Notify::Find && notify.data().toByteArray() == id()) {
    find(m_findWidget->text());
  }
  else if (notify.type() == Notify::FindPrevious && notify.data().toByteArray() == id()) {
    find(m_findWidget->text(), false);
  }
}


void ChannelBaseTab::offline()
{
  setOnline(false);
  add(ServiceMessage::quit(ChatClient::id()));
}


void ChannelBaseTab::setup()
{
  if (m_serverId != ChatClient::serverId())
    m_tabs->closeTab(m_tabs->indexOf(this));
}


bool ChannelBaseTab::isForbidden()
{
  return (SimpleID::typeOf(id()) == SimpleID::ChannelId && c()->data().value(LS("status"), Notice::OK) != Notice::OK);
}


/*!
 * Защищённый доступ к каналу, гарантирует доступ к актуальному указателю.
 */
ClientChannel ChannelBaseTab::c()
{
  ClientChannel channel = ChatClient::channels()->get(id());
  if (channel && channel != m_channel)
    m_channel = channel;

  return m_channel;
}


QIcon ChannelBaseTab::channelIcon()
{
  int alerts = ChatAlerts::count(id());
  ClientChannel channel = c();

  if (alerts)
    return AlertsPixmap::icon(ChatIcons::icon(channel, ChatIcons::OfflineStatus), alerts);
  else
    return ChatIcons::icon(channel);
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
  setText(c()->name());
  setIcon(channelIcon());
}
