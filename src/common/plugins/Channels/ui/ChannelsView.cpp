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

#include <QWebFrame>
#include <QContextMenuEvent>
#include <QMenu>

#include "ChatNotify.h"
#include "client/ChatClient.h"
#include "client/ClientChannels.h"
#include "net/SimpleID.h"
#include "sglobal.h"
#include "ui/ChannelsView.h"
#include "ui/TabWidget.h"
#include "WebBridge.h"
#include "ui/tabs/ChatView.h"

ChannelsView::ChannelsView(QWidget *parent)
  : WebView(parent)
{
  setAcceptDrops(false);

  connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(populateJavaScriptWindowObject()));
  connect(ChatNotify::i(), SIGNAL(notify(const Notify &)), SLOT(notify(const Notify &)));

  retranslateUi();
}


void ChannelsView::join(const QString &name)
{
  if (name.size() == 34) {
    const QByteArray id = SimpleID::decode(name);
    if (SimpleID::typeOf(id) != SimpleID::ChannelId)
      return;

    if (!TabWidget::i()->channelTab(id, false, true))
      ChatClient::channels()->join(id);
  }
  else
    ChatClient::channels()->join(name);
}


void ChannelsView::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu(this);

  QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
  bool selected = r.isContentSelected();
  bool editable = r.isContentEditable();

  if (selected) {
    if (editable)
      menu.addAction(pageAction(QWebPage::Cut));

    menu.addAction(pageAction(QWebPage::Copy));
  }

  const QUrl url = r.linkUrl();
  if (!url.isEmpty() && url.scheme() != LS("chat") && url.scheme() != LS("qrc"))
    menu.addAction(pageAction(QWebPage::CopyLinkToClipboard));

  if (editable) {
    if (ChatView::canPaste())
      menu.addAction(pageAction(QWebPage::Paste));
  }

  menu.addAction(pageAction(QWebPage::SelectAll));

  menu.exec(event->globalPos());
}


void ChannelsView::notify(const Notify &notify)
{
  int type = notify.type();
  if (type == Notify::FeedData || type == Notify::FeedReply) {
    const FeedNotify &n = static_cast<const FeedNotify &>(notify);
    if (n.channel() != ChatClient::serverId())
      return;

    QVariantMap json = WebBridge::feed(n);
    if (json.isEmpty())
      return;

    emit feed(json);
  }
}


void ChannelsView::populateJavaScriptWindowObject()
{
  page()->mainFrame()->addToJavaScriptWindowObject(LS("SimpleChat"), WebBridge::i());
  page()->mainFrame()->addToJavaScriptWindowObject(LS("ChannelsView"), this);
}
