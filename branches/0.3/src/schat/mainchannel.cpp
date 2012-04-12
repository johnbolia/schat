/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008 IMPOMEZIA <schat@impomezia.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui>

#include "mainchannel.h"
#include "networkwidget.h"
#include "settings.h"

/*!
 * \brief Конструктор класса MainChannel.
 */
MainChannel::MainChannel(const QIcon &icon, Settings *settings, QWidget *parent)
  : AbstractTab(icon, settings, parent)
{
  m_type = Main;

  m_settings = settings;
  m_browser->setChannel("#main");

  createActions();

  m_connectCreateButton->setVisible(false);

  m_networkWidget = new NetworkWidget(settings, this);
  m_networkWidget->setVisible(false);

  m_networkLayout = new QHBoxLayout;
  m_networkLayout->addWidget(m_networkWidget);
  m_networkLayout->addWidget(m_connectCreateButton);
  m_networkLayout->addStretch();
  m_networkLayout->setMargin(0);

  m_mainLayout = new QVBoxLayout;
  m_mainLayout->addLayout(m_networkLayout);
  m_mainLayout->addWidget(m_browser);
  m_mainLayout->setMargin(0);
  m_mainLayout->setSpacing(2);
  setLayout(m_mainLayout);

  connect(m_networkWidget, SIGNAL(validServer(bool)), m_connectCreateButton, SLOT(setEnabled(bool)));
}


/*!
 * Включает/выключает показ виджета выбора сервера/сети.
 *
 * \param display \a true Показать виджет, \a Скрыть.
 */
void MainChannel::displayChoiceServer(bool display)
{
  if (display)
    m_networkLayout->setContentsMargins(4, 2, 4, 0);

    m_networkWidget->setVisible(display);
    m_connectCreateButton->setVisible(display);

  if (!display)
    m_networkLayout->setMargin(0);

  m_browser->scroll();
}


/** [private slots]
 *
 */
void MainChannel::serverChanged()
{
  m_networkWidget->save();
  m_settings->notify(Settings::ServerChanged);
}


/** [private]
 *
 */
void MainChannel::createActions()
{
  m_connectCreateButton = new QToolButton(this);
  m_connectCreateAction = new QAction(QIcon(":/images/connect_creating.png"), tr("Подключится"), this);
  m_connectCreateButton->setDefaultAction(m_connectCreateAction);
  m_connectCreateButton->setAutoRaise(true);
  connect(m_connectCreateAction, SIGNAL(triggered()), this, SLOT(serverChanged()));
}