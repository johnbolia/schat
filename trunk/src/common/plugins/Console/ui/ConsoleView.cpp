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

#include <QApplication>
#include <QWebFrame>

#include "ChatNotify.h"
#include "client/ChatClient.h"
#include "sglobal.h"
#include "Tr.h"
#include "ui/ConsoleView.h"
#include "WebBridge.h"

class ConsoleTr : public Tr
{
  Q_DECLARE_TR_FUNCTIONS(ConsoleTr)

public:
  ConsoleTr() : Tr() { m_prefix = LS("console_"); }

protected:
  QString valueImpl(const QString &key) const
  {
    if (key == LS("bad_server")) return tr("This server does not support remote management.");
    return QString();
  }
};


ConsoleView::ConsoleView(QWidget *parent)
  : QWebView(parent)
{
  m_tr = new ConsoleTr();
  setAcceptDrops(false);

  connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(populateJavaScriptWindowObject()));
  connect(ChatNotify::i(), SIGNAL(notify(const Notify &)), SLOT(notify(const Notify &)));
}


ConsoleView::~ConsoleView()
{
  delete m_tr;
}


void ConsoleView::notify(const Notify &notify)
{
  int type = notify.type();
  if (type == Notify::FeedData || type == Notify::FeedReply || type == Notify::QueryError) {
    const FeedNotify &n = static_cast<const FeedNotify &>(notify);
    if (n.channel() != ChatClient::serverId())
      return;

    QVariantMap json = WebBridge::feed(n);
    if (json.isEmpty())
      return;

    emit feed(json);
  }
}


void ConsoleView::populateJavaScriptWindowObject()
{
  page()->mainFrame()->addToJavaScriptWindowObject(LS("SimpleChat"), WebBridge::i());
  page()->mainFrame()->addToJavaScriptWindowObject(LS("ConsoleView"), this);
}
