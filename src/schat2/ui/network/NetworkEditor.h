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

#ifndef NETWORKEDITOR_H_
#define NETWORKEDITOR_H_

#include <QWidget>

class NetworkWidget;
class QCheckBox;
class QPushButton;
class LoginWidget;
class QTabWidget;
class SignUpWidget;

class NetworkEditor : public QWidget
{
  Q_OBJECT

public:
  /// Настройка внешнего вида виджета.
  enum EditorLayout {
    BasicLayout = 0,
    ConnectButtonLayout = 1 ///< Добавить кнопку подключения.
  };

  NetworkEditor(QWidget *parent = 0, EditorLayout layout = BasicLayout);

protected:
  void changeEvent(QEvent *event);

private slots:
  void clientStateChanged();

private:
  void retranslateUi();

  EditorLayout m_layout;       ///< Настройка внешнего вида виджета.
  LoginWidget *m_login;        ///< Виджет входа.
  NetworkWidget *m_network;    ///< Виджет выбора сети.
  QCheckBox *m_anonymous;      ///< Флажок анонимного подключения.
  QPushButton *m_connect;      ///< Кнопка Connect.
  QTabWidget *m_tabs;          ///< Вкладки.
  SignUpWidget *m_signup;      ///< Виджет регистрации.
};

#endif /* NETWORKEDITOR_H_ */
