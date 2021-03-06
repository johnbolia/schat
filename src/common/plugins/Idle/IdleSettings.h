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

#ifndef IDLESETTINGS_H_
#define IDLESETTINGS_H_

#include <QWidget>

class QSpinBox;
class QLabel;

class IdleSettings : public QWidget
{
  Q_OBJECT

public:
  IdleSettings(QWidget *parent = 0);

protected:
  void changeEvent(QEvent *event);

private slots:
  void changed(int value);

private:
  void retranslateUi();

  QLabel *m_label;  ///< Поясняющая надпись.
  QSpinBox *m_time; ///< Виджет выбора времени через которое будет включен статус AutoAway.
};

#endif /* IDLESETTINGS_H_ */
