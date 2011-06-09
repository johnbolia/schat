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

#include <QDebug>
#include <QComboBox>
#include <QHBoxLayout>
#include <QMenu>
#include <QToolButton>
#include <QToolBar>

#include "ui/fields/GenderField.h"
#include "ChatCore.h"
#include "ChatSettings.h"
#include "ui/UserUtils.h"

GenderField::GenderField(QWidget *parent)
  : QWidget(parent)
  , m_settings(ChatCore::i()->settings())
  , m_user(new User())
{
  m_combo = new QComboBox(this);
  m_combo->addItem(tr("Male"));
  m_combo->addItem(tr("Female"));
  m_combo->setMinimumWidth(m_combo->width());

  m_menu = new QMenu(this);

  addColor(tr("Default"));
  addColor(tr("Black"));
  addColor(tr("Gray"));
  addColor(tr("Green"));
  addColor(tr("Red"));
  addColor(tr("White"));
  addColor(tr("Yellow"));

  m_config = new QToolButton(this);
  m_config->setIcon(SCHAT_ICON(GearIcon));
  m_config->setMenu(m_menu);
  m_config->setPopupMode(QToolButton::InstantPopup);

  m_toolBar = new QToolBar(this);
  m_toolBar->setIconSize(QSize(16, 16));
  m_toolBar->addWidget(m_config);
  m_toolBar->setStyleSheet("QToolBar { margin:0px; border:0px; }" );

  m_user->setRawGender(m_settings->value(ChatSettings::ProfileGender).toInt());
  setState();

  QHBoxLayout *mainLay = new QHBoxLayout(this);
  mainLay->addWidget(m_combo);
  mainLay->addWidget(m_toolBar);
  mainLay->addStretch();
  mainLay->setMargin(0);
  mainLay->setSpacing(0);

  connect(m_combo, SIGNAL(currentIndexChanged(int)), SLOT(indexChanged(int)));
  connect(m_settings, SIGNAL(changed(const QList<int> &)), SLOT(settingsChanged(const QList<int> &)));
}


void GenderField::updateData()
{
  m_settings->updateValue(ChatSettings::ProfileGender, m_user->rawGender());
}


void GenderField::indexChanged(int index)
{
  if (index == -1)
    return;

  if (index == 0)
    m_user->setGender(User::Male);
  else
    m_user->setGender(User::Female);

  setState();
  updateData();
}


void GenderField::setColor()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (!action)
    return;

  m_user->setColor(action->data().toInt());

  setState();
  updateData();
}


void GenderField::settingsChanged(const QList<int> &keys)
{
  if (!keys.contains(ChatSettings::ProfileGender))
    return;

  m_user->setRawGender(m_settings->value(ChatSettings::ProfileGender).toInt());
  setState();
}


void GenderField::addColor(const QString &name)
{
  QAction *action = m_menu->addAction(name, this, SLOT(setColor()));
  action->setCheckable(true);
  action->setData(m_colors.size());
  m_colors.append(action);
}


void GenderField::setIcons()
{
  int gender = m_user->rawGender();


  m_user->setGender(User::Male);
  m_combo->setItemIcon(0, UserUtils::icon(m_user));

  m_user->setGender(User::Female);
  m_combo->setItemIcon(1, UserUtils::icon(m_user));

  m_user->setRawGender(gender);

  for (int i = 0; i < m_colors.size(); ++i) {
    m_user->setColor(i);
    m_colors.at(i)->setIcon(UserUtils::icon(m_user));
  }

  m_user->setRawGender(gender);
}


void GenderField::setState()
{
  setIcons();
  m_config->setEnabled(true);

  if (m_user->gender() == User::Female)
    m_combo->setCurrentIndex(1);
  else if (m_user->gender() == User::Male)
    m_combo->setCurrentIndex(0);
  else {
    m_combo->setCurrentIndex(-1);
    m_config->setEnabled(false);
    return;
  }

  if (m_user->color() < 0 || m_user->color() > User::Yellow)
    return;

  for (int i = 0; i < m_colors.size(); ++i) {
    m_colors.at(i)->setChecked(false);
  }

  m_colors.at(m_user->color())->setChecked(true);
}
