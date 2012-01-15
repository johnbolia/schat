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

#include <QApplication>
#include <QFile>

#include <QVBoxLayout>

#include "ChatAlerts.h"
#include "ChatCore.h"
#include "messages/ServiceMessage.h"
#include "ui/ChatIcons.h"
#include "ui/tabs/AlertTab.h"
#include "ui/tabs/ChatView.h"

AlertTab::AlertTab(TabWidget *parent)
  : AbstractTab(QByteArray(), AlertType, parent)
{
  QString file = QApplication::applicationDirPath() + "/styles/test/html/ChatView.html";
  if (QFile::exists(file))
    file = QUrl::fromLocalFile(file).toString();
  else
    file = "qrc:/html/ChatView.html";

  m_chatView = new ChatView(QByteArray(), file, this);

  m_deleteOnClose = false;

  QVBoxLayout *mainLay = new QVBoxLayout(this);
  mainLay->addWidget(m_chatView);
  mainLay->setMargin(0);
  mainLay->setSpacing(0);

  setIcon(SCHAT_ICON(InfoBalloon));
  retranslateUi();

  connect(ChatAlerts::i(), SIGNAL(alert(const Alert &)), SLOT(alert(const Alert &)));
}


void AlertTab::alert(const Alert &alert)
{
  if (alert.type() == Alert::Connected)
    m_chatView->add(ServiceMessage::connected());
  else if (alert.type() == Alert::ConnectionLost)
    m_chatView->add(ServiceMessage::connectionLost());
}


void AlertTab::retranslateUi()
{
  setText(tr("Notifications"));
}