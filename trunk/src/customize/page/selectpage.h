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
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SELECTPAGE_H_
#define SELECTPAGE_H_

#include <QWizardPage>

class QCheckBox;
class QLabel;
class QLineEdit;
class QSpinBox;

class SelectPage : public QWizardPage
{
  Q_OBJECT

public:
  SelectPage(QWidget *parent = 0);

private slots:
  void clickedCore(bool checked);

private:
  QCheckBox *m_core;
  QCheckBox *m_overrideLevels;
  QCheckBox *m_runtime;
  QCheckBox *m_save;
  QLabel *m_coreLabel;
  QLabel *m_runtimeLabel;
  QLineEdit *m_suffix;
  QLineEdit *m_version;
  QSpinBox *m_coreLevel;
  QSpinBox *m_runtimeLevel;
};

#endif /* SELECTPAGE_H_ */
