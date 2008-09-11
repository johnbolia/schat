/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008 IMPOMEZIA (http://impomezia.com)
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

#ifndef DAEMONUI_H_
#define DAEMONUI_H_

#include <QDialog>

class QAction;
class QLabel;
class QMenu;
class QSystemTrayIcon;
class QToolButton;

/*!
 * \brief Диалог управления сервером
 */
class DaemonUi : public QDialog
{
  Q_OBJECT

public:
  DaemonUi(QWidget *parent = 0);

private:
  void createActions();
  void createTray();

  QAction *m_quitAction;
  QAction *m_quitAllAction;
  QAction *m_restartAction;
  QAction *m_startAction;
  QAction *m_stopAction;
  QLabel *m_aboutLabel;
  QMenu *m_menu;
  QPushButton *m_hideButton;
  QPushButton *m_settingsButton;
  QSystemTrayIcon *m_tray;
  QToolButton *m_restartButton;
  QToolButton *m_startButton;
  QToolButton *m_stopButton;
};

#endif /*DAEMONUI_H_*/
