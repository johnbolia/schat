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

#include "QCheckBox"
#include "QPushButton"
#include <QAction>
#include <QEvent>
#include <QGridLayout>
#include <QTabWidget>

#include "ChatCore.h"
#include "ChatNotify.h"
#include "client/ChatClient.h"
#include "client/SimpleClient.h"
#include "NetworkManager.h"
#include "ui/network/NetworkEditor.h"
#include "ui/network/NetworkTabs.h"
#include "ui/network/NetworkWidget.h"

NetworkEditor::NetworkEditor(QWidget *parent, EditorLayout layout)
  : QWidget(parent)
  , m_layout(layout)
  , m_connect(0)
{
  m_manager = ChatCore::i()->networks();
  m_network = new NetworkWidget(this);
  m_anonymous = new QCheckBox(this);
  m_anonymous->setVisible(false);

  if (m_layout & ConnectButtonLayout) {
    m_connect = new QPushButton(this);
    connect(m_connect, SIGNAL(clicked()), m_network, SLOT(open()));
  }

  m_tabs = new NetworkTabs(this);

  QGridLayout *mainLay = new QGridLayout(this);
  mainLay->addWidget(m_network, 0, 0, 1, 2);
  mainLay->addWidget(m_anonymous, 1, 0);

  if (m_layout & ConnectButtonLayout)
    mainLay->addWidget(m_connect, 1, 1);

  mainLay->addWidget(m_tabs, 2, 0, 1, 2);
  mainLay->setColumnStretch(0, 1);
  mainLay->setMargin(0);
  mainLay->setSpacing(4);

  connect(ChatClient::io(), SIGNAL(clientStateChanged(int, int)), SLOT(reload()));
  connect(m_anonymous, SIGNAL(toggled(bool)), SLOT(anonymousToggled(bool)));

  connect(ChatNotify::i(), SIGNAL(notify(const Notify &)), SLOT(notify(const Notify &)));

  retranslateUi();
}


void NetworkEditor::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();

  QWidget::changeEvent(event);
}


void NetworkEditor::anonymousToggled(bool checked)
{
  m_tabs->setVisible(!checked);
}


void NetworkEditor::notify(const Notify &notify)
{
  if (notify.type() == Notify::NetworkSelected || notify.type() == Notify::NetworkChanged)
    reload();
}


/*!
 * Обновление состояния виджета.
 */
void NetworkEditor::reload()
{
  if (m_layout & ConnectButtonLayout) {
    QAction *action = m_network->connectAction();
    m_connect->setIcon(action->icon());
    m_connect->setText(action->text());
  }

  m_tabs->reload();
  m_anonymous->setEnabled(true);

  Network item = m_manager->item(m_manager->selected());
  m_anonymous->setEnabled(!item->isAuthorized());
  m_anonymous->setChecked(item->account().isEmpty());
}


void NetworkEditor::retranslateUi()
{
  m_anonymous->setText(tr("Anonymous connection"));
  reload();
}
