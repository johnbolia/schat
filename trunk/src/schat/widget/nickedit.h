/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2009 IMPOMEZIA <schat@impomezia.com>
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

#ifndef NICKEDIT_H_
#define NICKEDIT_H_

#include <QWidget>

class QHBoxLayout;
class QLineEdit;
class QToolButton;

/*!
 * \brief Обеспечивает редактирование ника и пола пользователя.
 */
class NickEdit : public QWidget
{
  Q_OBJECT

public:
  enum OptionsFlag {
    NoOptions    = 0x0,
    GenderButton = 0x1,
    ApplyButton  = 0x2
  };

  Q_DECLARE_FLAGS(Options, OptionsFlag)

  NickEdit(QWidget *parent = 0, Options options = NoOptions);
  QString nick() const;
  void reset();
  void setMargin(int margin);

signals:
  void validNick(bool valid);

public slots:
  int save(int notify = true);

protected:
  void showEvent(QShowEvent *event);

private slots:
  void validateNick(const QString &text);

private:
  QHBoxLayout *m_mainLay;
  QLineEdit *m_edit;
  QToolButton *m_applyButton;
  QToolButton *m_genderButton;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NickEdit::Options)

#endif /* NICKEDIT_H_ */
