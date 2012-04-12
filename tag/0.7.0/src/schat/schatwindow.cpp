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

#include <QtGui>
#include <QtNetwork>

#include "aboutdialog.h"
#include "abstractprofile.h"
#include "channellog.h"
#include "chatwindow/chatview.h"
#include "clientservice.h"
#include "mainchannel.h"
#include "popup/popupmanager.h"
#include "schatwindow.h"
#include "schatwindow_p.h"
#include "settings.h"
#include "settingsdialog.h"
#include "simplechatapp.h"
#include "soundaction.h"
#include "privatetab.h"
#include "trayicon.h"
#include "welcomedialog.h"
#include "widget/sendwidget.h"
#include "widget/userview.h"

QMap<QString, QString> SChatWindowPrivate::cmds;


/*!
 * \brief Конструктор класса SChatWindowPrivate.
 */
SChatWindowPrivate::SChatWindowPrivate(SChatWindow *parent)
  : motd(true), q(parent)
{
}


SChatWindowPrivate::~SChatWindowPrivate()
{
}


/*!
 * Обработка команд.
 */
bool SChatWindowPrivate::parseCmd(AbstractTab *tab, const QString &message)
{
  QString text     = ChannelLog::toPlainText(message).trimmed();
  QString textFull = text;
  text = text.toLower();

  /// /away
  if (text == "/away") {
    if (profile->status() == schat::StatusAway)
      sendStatus(schat::StatusNormal);
    else
      sendStatus(schat::StatusAway);
  }
  /// /bye
  else if (text == "/bye") {
    clientService->quit();
  }
  else if (text.startsWith("/bye ")) {
    clientService->sendByeMsg(textFull.mid(textFull.indexOf(QChar(' '))));
    clientService->quit();
  }
  /// /clear
  else if (text == "/clear") {
    tab->clear();
  }
  else if (text == "/dnd") {
    if (profile->status() == schat::StatusDnD)
      sendStatus(schat::StatusNormal);
    else
      sendStatus(schat::StatusDnD);
  }
  /// /exit
  else if (text == "/exit" || text == "/quit") {
    closeChat();
  }
  else if (text.startsWith("/google ")) {
    QString query = textFull.mid(textFull.indexOf(' ')).simplified().left(1000);
    sendMsg("<b style='color:#0039b6'>G</b><b style='color:#c41200'>o</b>"
            "<b style='color:#f3c518'>o</b><b style='color:#0039b6'>g</b>"
            "<b style='color:#30a72f'>l</b><b style='color:#c41200'>e</b>: "
            "<b><a href='http://www.google.com/search?q=" + QUrl::toPercentEncoding(query)
            + "'>" + Qt::escape(query) + "</a></b>", false);
  }
  /// /help
  else if (text == "/help") {
    cmdHelp(tab, "");
  }
  else if (text.startsWith("/help ")) {
    cmdHelp(tab, textFull.mid(textFull.indexOf(' ')).trimmed());
  }
  /// /log
  else if (text == "/log") {
    QDesktopServices::openUrl(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/log"));
  }
  /// /nick
  else if (text.startsWith("/nick ")) {
    QString newNick = textFull.mid(textFull.indexOf(' '));
    if (AbstractProfile::isValidNick(newNick) && profile->nick() != newNick) {
      profile->setNick(newNick);
      clientService->sendNewProfile();
    }
  }
  else if (text == "/ping") {
    pingTime.start();
    return false;
  }
  else
    return false;

  send->clear();
  return true;
}


/*!
 * Формирует универсальный пакет для отправки статуса.
 */
bool SChatWindowPrivate::sendStatus(quint32 status)
{
  if (clientService->isReady())
    return clientService->sendUniversal(schat::UniStatus, QList<quint32>() << status, QStringList());
  else {
    profile->setStatus(status);
    return true;
  }
}


/*!
 * При необходимости вызывает уведомление о новом сообщении.
 * Механизм уведомления запускается, если номер текущей вкладки не равен номеру вкладки
 * с уведомлением или окно чата неактивно.
 *
 * Функция вызывается при получении нового сообщения в главном канале или при приватном сообщении.
 *
 * \param index Номер вкладки, в которой есть новое сообщение.
 * \param key   Ключ для звукового уведомления.
 *
 * \return \a true если механизм уведомлений запущен.
 */
bool SChatWindowPrivate::startNotice(int index, const QString &key)
{
  if (index == -1)
    return false;

  bool notice = (tabs->currentIndex() != index || !q->isActiveWindow());
  if ((notice || pref->getBool("Sound/AlwaysPlay")) && !soundAction->isMute())
    tray->playSound(key, true);

  if (notice) {
    AbstractTab *tab = static_cast<AbstractTab *>(tabs->widget(index));

    if (!tab->notice()) {
      tab->notice(true);
      tabs->setTabIcon(index, QIcon(":/images/notice.png"));
      tray->notice(true);
    }
    return true;
  }

  return false;
}


/*!
 * Создаёт новую вкладку с приватом.
 */
QPair<int, AbstractTab *> SChatWindowPrivate::createPrivateTab(const QString &nick)
{
  AbstractProfile prof(users->profile(nick));
  AbstractTab *tab = new PrivateTab(QIcon(":/images/" + prof.gender() + ".png"), q);
  tab->setChannel(nick);
  int index = tabs->addTab(tab, tab->icon(), nick);
  tabs->setTabToolTip(index, UserView::userToolTip(prof));

  return QPair<int, AbstractTab *>(index, tab);
}


/*!
 * Выполняет поиск вкладки с заданным текстом и типом.
 */
QPair<int, AbstractTab *> SChatWindowPrivate::tabFromName(const QString &text, AbstractTab::Type type) const
{
  int count = tabs->count();

  if (count > 0)
    for (int i = 0; i < count; ++i)
      if (tabs->tabText(i) == text) {
        AbstractTab *tab = static_cast<AbstractTab *>(tabs->widget(i));
        if (tab->type() == type)
          return QPair<int, AbstractTab *>(i, tab);
      }

  return QPair<int, AbstractTab *>(-1, 0);
}


/*!
 * Обновляет вкладку.
 */
QPair<int, AbstractTab *> SChatWindowPrivate::updatePrivateTab(const AbstractProfile &prof)
{
  QPair<int, AbstractTab *> tab = tabFromName(prof.nick());
  if (tab.first != -1) {
    tabs->setTabToolTip(tab.first, UserView::userToolTip(prof));
    tab.second->setIcon(QIcon(":/images/" + prof.gender() + ".png"));

    if (!tab.second->notice())
      tabs->setTabIcon(tab.first, tab.second->icon());
  }
  return tab;
}


/*!
 * Определяет получателя сообщения.
 */
QString SChatWindowPrivate::channel()
{
  AbstractTab *tab = static_cast<AbstractTab *>(tabs->currentWidget());
  if (tab->type() == AbstractTab::Private)
    return tabs->tabText(tabs->currentIndex());
  else
    return "";
}


QString SChatWindowPrivate::colorizedPing() const
{
  int ms = pingTime.elapsed();

  #ifdef Q_WS_WIN
    if (ms == 0)
      ms = 8;
  #endif

  QString color = "6bb521";
  if (ms > 200 && ms < 500)
    color = "ff9900";
  else if (ms >= 500)
    color = "da251d";

  return QObject::tr("Ping до сервера <b style='color:#%1;'>%2 мс</b>").arg(color).arg(ms);
}


/*!
 * Отображает подсказку по командам.
 */
void SChatWindowPrivate::cmdHelp(AbstractTab *tab, const QString &cmd)
{
  if (cmds.isEmpty()) {
    cmds.insert("away",   QObject::tr("<b>/away</b><span class='info'> — включает/выключает статус <b>Отсутствую</b>.</span>"));
    cmds.insert("bye",    QObject::tr("<b>/bye [текст сообщения]</b><span class='info'> — отключится от сервера/сети, опционально можно указать альтернативное сообщение о выходе.</span>"));
    cmds.insert("clear",  QObject::tr("<b>/clear</b><span class='info'> — очистка окна чата.</span>"));
    cmds.insert("dnd",    QObject::tr("<b>/dnd</b><span class='info'> — включает/выключает статус <b>Не беспокоить</b>.</span>"));
    cmds.insert("exit",   QObject::tr("<b>/exit</b><span class='info'> — выход из чата.</span>"));
    cmds.insert("google", QObject::tr("<b>/google &lt;строка поиска&gt;</b><span class='info'> — формирует ссылку с заданной строкой для поиска в Google.</span>"));
    cmds.insert("help",   QObject::tr("<b>/help</b><span class='info'> — отображает подсказу о командах.</span>"));
    cmds.insert("log",    QObject::tr("<b>/log</b><span class='info'> — открывает папку с файлами журнала чата.</span>"));
    cmds.insert("me",     QObject::tr("<b>/me &lt;текст сообщения&gt;</b><span class='info'> — отправка сообщения о себе от третьего лица, например о том что вы сейчас делаете.</span>"));
    cmds.insert("motd",   QObject::tr("<b>/motd</b><span class='info'> — показ <i>Message Of The Day</i> сообщения сервера.</span>"));
    cmds.insert("nick",   QObject::tr("<b>/nick &lt;новый ник&gt;</b><span class='info'> — позволяет указать новый ник, если указанный ник уже занят, произойдёт автоматическое отключение.</span>"));
    cmds.insert("ping",   QObject::tr("<b>/ping</b><span class='info'> — определение времени задержки до сервера.</span>"));
    cmds.insert("server", QObject::tr("<b>/server</b><span class='info'> — просмотр информации о сервере.</span>"));
  }

  if (cmd.isEmpty()) {
    QString out = QObject::tr("<b class='info'>Доступные команды:</b><br />");
    out += "<b>";

    QMapIterator<QString, QString> i(cmds);
    while (i.hasNext()) {
      i.next();
      out += ('/' + i.key() + "<br />");
    }

    out += "</b>";
    out += QObject::tr("<span class='info'>Используйте <b>/help команда</b>, для просмотра подробной информации о команде.</span>");
    tab->msg(out);
    return;
  }

  QString command = cmd.toLower();
  if (command.startsWith('/')) {
    command = command.mid(1);
  }

  if (cmds.contains(command)) {
    tab->msg(cmds.value(command));
  }
  else
    tab->msg("<span class='statusUnknownCmd'>" + QObject::tr("Неизвестная команда: <b>%1</b>").arg(command) + "</span>");
}


/*!
 * Завершение работы программы.
 */
void SChatWindowPrivate::closeChat(bool update)
{
  #ifdef SCHAT_NO_UPDATE
    Q_UNUSED(update)
  #endif

  clientService->quit();
  saveGeometry();
  pref->write();

  #ifndef SCHAT_NO_UPDATE
    if (update)
      Settings::install();
  #endif

  QApplication::quit();
}


void SChatWindowPrivate::createStatusBar()
{
  connectLabel = new QLabel(q);
  connectLabel->setVisible(false);

  connectMovie = new QLabel(q);
  connectMovie->setMovie(new QMovie(":/images/load.gif", QByteArray(), q));
  connectMovie->setVisible(false);

  statusLabel = new QLabel(QObject::tr("Нет подключения"), q);
  statusLabel->setWordWrap(true);

  #ifndef SCHAT_WINCE
  statusCombo = new QComboBox(q);
  statusCombo->setIconSize(QSize(14, 14));
  statusCombo->addItem(QIcon(":/images/status-online.png"),  QObject::tr("В сети"));
  statusCombo->addItem(QIcon(":/images/status-away.png"),    QObject::tr("Отсутствую"));
  statusCombo->addItem(QIcon(":/images/status-dnd.png"),     QObject::tr("Не беспокоить"));
  statusCombo->addItem(QIcon(":/images/status-offline.png"), QObject::tr("Не в сети"));
  statusCombo->setFocusPolicy(Qt::NoFocus);
  #endif

  statusBar = new QStatusBar(q);
  statusBar->addWidget(connectLabel);
  statusBar->addWidget(connectMovie);
  statusBar->addWidget(statusLabel, 1);
  #ifndef SCHAT_WINCE
  statusBar->addPermanentWidget(statusCombo);
  #endif
  statusBar->setStyleSheet("QStatusBar::item { border-width: 0; }");
  q->setStatusBar(statusBar);
}


/*!
 * Инициализирует поддержку системного трея.
 *
 * \note Для Windows Mobile не происходит создания меню трея,
 * вместо этого создаётся меню приложения.
 */
void SChatWindowPrivate::createTrayIcon()
{
  #ifndef Q_OS_WINCE
  QMenu *menu = new QMenu(q);
  #else
  QMenuBar *menu = q->menuBar();
  #endif

  menu->addAction(send->aboutAction());
  menu->addAction(settingsAction); /// \todo Необходимо изменить название пункта меню на \b Параметры.

  QMenu *statusMenu = new QMenu(QObject::tr("Статус"), q);
  statusMenu->addAction(onlineAction);
  statusMenu->addAction(awayAction);
  statusMenu->addAction(dndAction);
  statusMenu->addSeparator();
  statusMenu->addAction(offlineAction);

  statusAction = menu->addMenu(statusMenu);
  statusAction->setIcon(QIcon(":/images/status-online.png"));

  #ifdef Q_WS_WIN
  if (QFile::exists(QApplication::applicationDirPath() + "/schatd-ui.exe")) {
  #else
  if (QFile::exists(QApplication::applicationDirPath() + "/schatd-ui")) {
  #endif
    menu->addSeparator();
    menu->addAction(daemonAction);
  }

  menu->addSeparator();
  menu->addAction(send->quitAction());

  tray = new TrayIcon(q);
  #ifndef Q_OS_WINCE
  tray->setContextMenu(menu);
  #endif
  tray->show();
}


/*!
 * Отображает в тексте приватов статусы.
 * Формирует HTML строку в зависимости от статуса и пола для отображения в тексте.
 * Если происходит изменение собственного статуса, то сообщение добавляется
 * во все открытые приваты, иначе происходит поиск вкладки с приватом и в
 * случае успеха сообщение добавляется туда и происходит обновление
 * всплывающей подсказки.
 *
 * \param status Статус.
 * \param nick   Ник пользователя.
 */
void SChatWindowPrivate::displayStatus(quint32 status, const QString &nick)
{
  if (!users->isUser(nick))
    return;

  bool statusMessages = pref->getBool("StatusMessages");
  QString html;
  if (statusMessages) {
    html = QString("<span class='away'><a href='nick:%1'>%2</a> ").arg(QLatin1String(nick.toUtf8().toHex())).arg(Qt::escape(nick));
    if (users->profile(nick).isFemale())
      html += QObject::tr("сменила статус на:");
    else
      html += QObject::tr("сменил статус на:");

    html += " <b>";

    if (status == schat::StatusAutoAway || status == schat::StatusAway)
      html += QObject::tr("Отсутствую");
    else if (status == schat::StatusDnD)
      html += QObject::tr("Не беспокоить");
    else
      html += QObject::tr("В сети");

    html += "</b></span>";
  }

  if (nick != profile->nick()) {
    QPair<int, AbstractTab *> tab = tabFromName(nick);
    if (tab.first != -1) {
      if (statusMessages)
        tab.second->msg(html);
      tabs->setTabToolTip(tab.first, UserView::userToolTip(users->profile(nick)));
    }
  }
  else if (statusMessages)
    msgToAllPrivateTabs(html);
}


/*!
 * Скрывает окна чата.
 */
void SChatWindowPrivate::hideChat()
{
  if (settingsDialog)
    settingsDialog->hide();

  if (about)
    about->hide();

  q->hide();
}


/*!
 * Добавляет сообщение во все открытие приваты.
 */
void SChatWindowPrivate::msgToAllPrivateTabs(const QString &msg)
{
  int count = tabs->count();

  if (count > 0)
    for (int i = 0; i < count; ++i) {
      AbstractTab *tab = static_cast<AbstractTab *>(tabs->widget(i));
      if (tab->type() == AbstractTab::Private)
        tab->msg(msg);
    }
}


/*!
 * Восстанавливает геометрию окна.
 */
void SChatWindowPrivate::restoreGeometry()
{
  q->resize(pref->size());
  QPoint pos = pref->pos();
  if (pos.x() != -999 && pos.y() != -999)
    q->move(pos);
}


/*!
 * Сохраняет геометрию окна.
 */
void SChatWindowPrivate::saveGeometry()
{
  pref->setPos(q->pos());
  pref->setSize(q->size());
}


/*!
 * Обработка отправки сообщения.
 * Получатель определяется в зависимости от типа вкладки.
 *
 * \param msg Сообщение.
 * \param cmd Включает обработку команд.
 */
void SChatWindowPrivate::sendMsg(const QString &msg, bool cmd)
{
  AbstractTab *tab = static_cast<AbstractTab *>(tabs->currentWidget());

  if (cmd)
    if (parseCmd(tab, msg))
      return;

  if (exitAwayOnSend) { /// \todo Изменить название опции.
    if (profile->status() != schat::StatusNormal)
      sendStatus(schat::StatusNormal);
  }

  if (clientService->sendMessage(channel(), msg))
    send->clear();
}


void SChatWindowPrivate::setAwayOptions()
{
  autoAway = pref->getBool("AutoAway");
  exitAwayOnSend = pref->getBool("ExitAwayOnSend");
  autoAwayTime = pref->getInt("AutoAwayTime");
  if (autoAwayTime < 1)
    autoAwayTime = 1;

  if (autoAway) {
    if (!idleDetector.isActive())
      idleDetector.start();
  }
  else if (idleDetector.isActive())
    idleDetector.stop();
}


/*!
 * Показ окна чата.
 */
void SChatWindowPrivate::showChat()
{
  q->setWindowState(q->windowState() & ~Qt::WindowMinimized);
  q->show();
  q->activateWindow();

  if (about)
    about->show();

  if (settingsDialog)
    settingsDialog->show();
}


/*!
 * Отображение состояния успешного подключения к серверу/сети.
 */
void SChatWindowPrivate::statusAccessGranted(const QString &network, const QString &server)
{
  connectMovie->movie()->setPaused(true);
  connectMovie->setVisible(false);
  connectLabel->setVisible(true);
  connectLabel->setPixmap(QPixmap(":/images/network_connect.png"));
  updateStatus(StatusOnline);

  if (network.isEmpty()) {
    statusLabel->setText(QObject::tr("Сервер %1").arg(server));
    main->msg("<span class='ready'>" + QObject::tr("Успешно подключены к серверу %1").arg(server) + "</span>");
    q->setWindowTitle(QApplication::applicationName());
  }
  else {
    statusLabel->setText(QObject::tr("Сеть %1 (%2)").arg(network).arg(server));
    main->msg("<span class='ready'>" + QObject::tr("Успешно подключены к сети <b>%1</b> (%2)").arg(Qt::escape(network)).arg(server) + "</span>");
    q->setWindowTitle(QApplication::applicationName() + " — " + network);
  }

  if (enableMotd && motd) {
    motd = false;
    clientService->sendMessage("", "/motd");
  }

  if (autoAway && profile->status() == schat::StatusNormal && !idleDetector.isActive())
    idleDetector.start();
}


/*!
 * Отображение состояния подключения к серверу/сети.
 */
void SChatWindowPrivate::statusConnecting(const QString &server, bool network)
{
  connectMovie->movie()->setPaused(false);
  connectMovie->setVisible(true);
  connectLabel->setVisible(false);
  updateStatus(StatusOffline);

  if (network)
    statusLabel->setText(QObject::tr("Подключение к сети %1...").arg(server));
  else
    statusLabel->setText(QObject::tr("Подключение к серверу %1...").arg(server));

  main->displayChoiceServer(false);
}


/*!
 * Отображение состояния отсутствия подключения к серверу/сети.
 */
void SChatWindowPrivate::statusUnconnected(bool echo)
{
  connectMovie->movie()->setPaused(true);
  connectMovie->setVisible(false);
  connectLabel->setVisible(true);
  connectLabel->setPixmap(QPixmap(":/images/network_disconnect.png"));
  updateStatus(StatusOffline);

  statusLabel->setText(QObject::tr("Нет подключения"));
  users->clear();

  soundAction->mute();

  if (echo)
    main->msg("<span class='disconnect'>" + QObject::tr("Соединение разорвано") + "</span>");
}


/*!
 * Создаёт уникальный ник.
 * Ник + случайное число от 0 до 99.
 *
 * Специальные случаи:
 * - Если длина ника равна максимальной допустимой, то отрезаются
 * два последних символа, для нормализации длинны.
 * - Если ник на один символ короче максимума, то случайное число
 * генерируется в диапазоне от 0 до 9.
 */
void SChatWindowPrivate::uniqueNick()
{
  int max = 99;
  QString nick = profile->nick();
  if (nick.size() == AbstractProfile::MaxNickLength)
    nick = nick.left(AbstractProfile::MaxNickLength - 2);
  else if (nick.size() == AbstractProfile::MaxNickLength - 1)
    max = 9;

  profile->setNick(nick + QString().setNum(qrand() % max));
}


/*!
 * Обработка изменения статуса пользователей.
 *
 * - Если собственный ник, содержится в \p data2, то устанавливаем в профиле статус.
 * - Обновляем список пользователей.
 * - Если включено эхо, то выполняем функцию displayAway(quint32 status, const QString &nick)
 * для первого ника.
 * - Иначе происходит обновление всплывающих подсказок приватов.
 */
void SChatWindowPrivate::universalStatus(const QList<quint32> &data1, const QStringList &data2)
{
  quint32 status = data1.at(0);

  // Проверка наличия собственного ника в списке data2.
  if (data2.contains(profile->nick())) {
    soundAction->mute();
    profile->setStatus(data1.at(0));

    if (status == schat::StatusAutoAway || status == schat::StatusAway)
      updateStatus(StatusAway);
    else if (status == schat::StatusDnD) {
      updateStatus(StatusDnD);
      if (pref->getBool("Sound/MuteInDnD") && soundAction->data().toBool())
        soundAction->mute(true);
    }
    else
      updateStatus(StatusOnline);

    if (autoAway && status == schat::StatusNormal && !idleDetector.isActive())
      idleDetector.start();
  }

  // Обновление списка пользователей.
  users->setStatus(status, data2);

  if (data1.size() > 1)
    if (data1.at(1))
      if (users->isUser(data2.at(0))) {
        displayStatus(status, data2.at(0));
        return;
      }

  // Обновление всплывающих подсказок приватов.
  int count = tabs->count();
  if (count > 0) {
    QHash<QString, int> privateTabs;

    for (int i = 0; i < count; ++i) {
      AbstractTab *tab = static_cast<AbstractTab *>(tabs->widget(i));
      if (tab->type() == AbstractTab::Private)
        privateTabs.insert(tabs->tabText(i), i);
    }

    if (!privateTabs.isEmpty()) {
      foreach (QString user, data2) {
        if (users->isUser(user))
          if (privateTabs.contains(user))
            tabs->setTabToolTip(privateTabs.value(user), UserView::userToolTip(users->profile(user)));
      }
    }
  }
}


/*!
 * Обновление визуальной информации о статусе.
 * Устанавливается индекс в \a statusCombo
 * и иконка для \a statusCombo.
 */
void SChatWindowPrivate::updateStatus(int status)
{
  #ifndef SCHAT_WINCE
  statusCombo->setCurrentIndex(status);
  #endif

  switch (status) {
    case StatusOnline:
      statusAction->setIcon(QIcon(":/images/status-online.png"));
      onlineAction->setChecked(true);
      break;

    case StatusAway:
      statusAction->setIcon(QIcon(":/images/status-away.png"));
      awayAction->setChecked(true);
      break;

    case StatusDnD:
      statusAction->setIcon(QIcon(":/images/status-dnd.png"));
      dndAction->setChecked(true);
      break;

    case StatusOffline:
      statusAction->setIcon(QIcon(":/images/status-offline.png"));
      offlineAction->setChecked(true);
      break;
  }
}


/*!
 * Создаёт кнопки.
 */
#ifndef Q_OS_WINCE
void SChatWindowPrivate::createToolButtons()
{

  toolBar->setIconSize(QSize(16, 16));
  toolBar->setStyleSheet("QToolBar { margin:0px; border:0px; }");

  #if QT_VERSION < 0x040500
  toolBar->addAction(QIcon(":/images/tab_close.png"), QObject::tr("Закрыть вкладку"), q, SLOT(closeTab()));
  toolBar->addSeparator();
  #endif

  QToolButton *settingsButton = send->settingsButton();
  if (settingsButton)
    toolBar->addWidget(settingsButton)->setVisible(true);

  if (send->soundAction().second)
    toolBar->addAction(soundAction);

  toolBar->addAction(send->aboutAction());
  tabs->setCornerWidget(toolBar);
}
#endif


/*!
 * \brief Конструктор класса SChatWindow.
 */
SChatWindow::SChatWindow(QWidget *parent)
  : QMainWindow(parent), d(new SChatWindowPrivate(this))
{
  d->pref        = new Settings(QApplication::applicationDirPath() + "/schat.conf", this);
  d->profile     = d->pref->profile();
  d->pref->read();
  d->enableMotd  = d->pref->getBool("MotdEnable");

  d->send        = new SendWidget(this);
  d->central     = new QWidget(this);
  d->tabs        = new QTabWidget(this);
  d->tabs->installEventFilter(this);
  d->users       = new UserView(d->profile, this);
  d->mainLay     = new QVBoxLayout(d->central);
  #ifndef SCHAT_WINCE
  d->toolBar     = new QToolBar(this);
  #endif

  /// \note Для Windows Mobile поле отправки находится сверху,
  /// для предотвращения его перекрытия виртуальной клавиатурой.
  #ifdef SCHAT_WINCE
    d->mainLay->addWidget(d->send);
    d->mainLay->setContentsMargins(0, 0, 0, 0);
  #endif
  d->mainLay->addWidget(d->tabs);
  #ifndef SCHAT_WINCE
    d->mainLay->addWidget(d->send);
    d->mainLay->setContentsMargins(3, 3, 3, 0);
  #endif
  d->mainLay->setSpacing(1);
  d->mainLay->setStretchFactor(d->tabs, 999);
  d->mainLay->setStretchFactor(d->send, 1);

  setCentralWidget(d->central);
  d->createStatusBar();
  d->popupManager = new PopupManager(this);
  connect(d->popupManager, SIGNAL(openChat(const QString &, bool, bool)), SLOT(openChat(const QString &, bool, bool)));

  setWindowTitle(QApplication::applicationName());

  d->tabs->setElideMode(Qt::ElideRight);
  #if QT_VERSION >= 0x040500
    d->tabs->setTabsClosable(true);
    d->tabs->setMovable(true);
    connect(d->tabs, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
  #endif

  d->restoreGeometry();
  createActions();
  #ifndef SCHAT_WINCE
  d->createToolButtons();
  #endif
  d->createTrayIcon();
  createService();

  connect(d->send, SIGNAL(sendMsg(const QString &)), SLOT(sendMsg(const QString &)));
  connect(d->send, SIGNAL(needCopy()), SLOT(copy()));
  connect(d->send, SIGNAL(about()), SLOT(about()));
  connect(d->send, SIGNAL(closeChat()), SLOT(closeChat()));
  connect(d->send, SIGNAL(statusShortcut(int)), SLOT(statusShortcut(int)));
  connect(d->send, SIGNAL(showSettingsPage(int)), SLOT(showSettingsPage(int)));
  connect(d->users, SIGNAL(addTab(const QString &)), SLOT(addTab(const QString &)));
  connect(d->users, SIGNAL(insertNick(const QString &)), d->send, SLOT(insertHtml(const QString &)));
  connect(d->users, SIGNAL(showSettings()), SLOT(showSettingsPage()));
  connect(d->tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
  connect(d->tabs, SIGNAL(currentChanged(int)), SLOT(stopNotice(int)));
  connect(d->pref, SIGNAL(changed(int)), SLOT(settingsChanged(int)));
  #ifndef SCHAT_WINCE
  connect(d->statusCombo, SIGNAL(activated(int)), SLOT(statusChangedByUser(int)));
  #endif

  #ifndef SCHAT_NO_UPDATE
    connect(d->tray, SIGNAL(messageClicked()), SLOT(messageClicked()));
  #endif

  d->main = new MainChannel(QIcon(":/images/main.png"), d->users, d->tabs);
  connect(d->main, SIGNAL(nickClicked(const QString &)), d->users, SLOT(nickClicked(const QString &)));
  connect(d->main, SIGNAL(emoticonsClicked(const QString &)), d->send, SLOT(insertHtml(const QString &)));
  connect(d->main, SIGNAL(popupMsg(const QString &, const QString &, const QString &, bool)), d->popupManager, SLOT(popupMsg(const QString &, const QString &, const QString &, bool)));

  qsrand(QDateTime(QDateTime::currentDateTime()).toTime_t());

  if (Settings::isNewYear())
    setWindowIcon(QIcon(":/images/logo-ny.png"));
  #ifndef Q_WS_WIN
  else
    setWindowIcon(QIcon(":/images/logo.png"));
  #endif

  // Показ модального диалога приветствия.
  if (!d->pref->getBool("HideWelcome") || d->pref->getBool("FirstRun")) {
    WelcomeDialog welcome(this);
    if (!welcome.exec()) {
      d->statusUnconnected(false);
      fatal();
    }
    else
      d->clientService->connectToHost();
  }
  else
    d->clientService->connectToHost();

  connect(&d->idleDetector, SIGNAL(secondsIdle(int)), SLOT(onSecondsIdle(int)));
  d->setAwayOptions();

  if (d->pref->getBool("Updates/Enable") && d->pref->getBool("Updates/CheckOnStartup"))
    QTimer::singleShot(0, d->pref, SLOT(updatesCheck()));
}


SChatWindow::~SChatWindow()
{
  delete d;
}


/*!
 * \brief Обработка события закрытия чата.
 */
void SChatWindow::closeEvent(QCloseEvent *event)
{
  saveGeometry();
  d->pref->write();
  d->hideChat();

  QMainWindow::closeEvent(event);
}


void SChatWindow::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape)
    d->hideChat();
  else
    QMainWindow::keyPressEvent(event);
}


/*!
 * Обработка событий.
 */
bool SChatWindow::event(QEvent *event)
{
  if (event->type() == QEvent::WindowActivate)
    stopNotice(d->tabs->currentIndex());

  return QMainWindow::event(event);
}


/** [private slots]
 *
 */
void SChatWindow::about()
{
  if (isHidden())
    show();

  if (!d->about) {
    d->about = new AboutDialog(this);
    d->about->show();
  }

  d->about->activateWindow();
}


/** [private slots]
 *
 */
void SChatWindow::accessDenied(quint16 reason)
{
  bool notify = true;

  switch (reason) {
    case ErrorNickAlreadyUse:
      d->uniqueNick();
      d->clientService->connectToHost();
      break;

    case ErrorOldClientProtocol:
      d->main->msg("<span class='oldClientProtocol'>" + tr("Ваш чат использует устаревшую версию протокола, подключение не возможно, пожалуйста обновите программу.") + "</span>");
      break;

    case ErrorOldServerProtocol:
      d->main->msg("<span class='oldServerProtocol'>" + tr("Сервер использует устаревшую версию протокола, подключение не возможно.") + "</span>");
      break;

    case ErrorBadNickName:
      d->main->msg("<span class='badNickName'>" + tr("Выбранный ник: <b>%2</b>, не допустим в чате, выберите другой").arg(Qt::escape(d->profile->nick())) + "</span>");
      break;

    case ErrorUsersLimitExceeded:
    case ErrorLinksLimitExceeded:
    case ErrorMaxUsersPerIpExceeded:
      notify = false;
      break;

    default:
      d->main->msg("<span class='accessDenied'>" + tr("При подключении произошла критическая ошибка с кодом: <b>%1</b>").arg(reason) + "</span>");
      break;
  }

  if (notify)
    unconnected(false);
}


/*!
 * Слот вызывается кода \a d->clientService получает пакет с опкодом \a OpcodeAccessGranted,
 * что означает успешное подключение к серверу/сети.
 */
void SChatWindow::accessGranted(const QString &network, const QString &server, quint16 level)
{
  Q_UNUSED(level)

  d->statusAccessGranted(network, server);
}


/*!
 * Открытие нового привата по инициативе пользователя.
 * Если вкладка не существует, то она будет создана, индекс
 * устанавливается на эту вкладку.
 *
 * \param nick Ник удалённого пользователя.
 */
void SChatWindow::addTab(const QString &nick)
{
  int index = d->tabFromName(nick).first;

  if (index == -1) {
    QPair<int, AbstractTab *> newTab = d->createPrivateTab(nick);
    index = newTab.first;
    connect(newTab.second, SIGNAL(nickClicked(const QString &)), d->users, SLOT(nickClicked(const QString &)));
    connect(newTab.second, SIGNAL(emoticonsClicked(const QString &)), d->send, SLOT(insertHtml(const QString &)));
    connect(newTab.second, SIGNAL(popupMsg(const QString &, const QString &, const QString &, bool)), d->popupManager, SLOT(popupMsg(const QString &, const QString &, const QString &, bool)));
  }

  d->tabs->setCurrentIndex(index);
}


/*!
 * Завершение работы программы.
 */
void SChatWindow::closeChat(bool update)
{
  d->closeChat(update);
}


/*!
 * Обработка закрытия вкладки.
 *
 * Если вкладка является основным каналом, то соединение разрывается,
 * также эта вкладка становится активной,
 * Вкладки-приваты закрываются и удаляются.
 *
 * \param tab Индекс вкладки, которую нужно закрыть, по умолчанию -1.
 */
void SChatWindow::closeTab(int tab)
{
  int index = tab;
  if (index == -1)
    index = d->tabs->currentIndex();

  QWidget *widget = d->tabs->widget(index);
  if (widget != d->main) {
    d->tabs->removeTab(index);
    QTimer::singleShot(0, widget, SLOT(deleteLater()));
  }
  else {
    d->clientService->quit();
    d->tabs->setCurrentIndex(index);
  }
}


/*!
 * Слот вызывается когда d->clientService пытается подключится к серверу.
 *
 * \param server  Название сети если network = true, либо адрес сервера.
 * \param network Нодключение к сети \a true либо к серверу \a false.
 */
void SChatWindow::connecting(const QString &server, bool network)
{
  d->statusConnecting(server, network);
}


void SChatWindow::copy()
{
  AbstractTab *tab = static_cast<AbstractTab *>(d->tabs->currentWidget());
  if (!tab->copy())
    d->send->copy();
}


/*!
 * Запуск программы управления сервером.
 */
void SChatWindow::daemonUi()
{
#ifdef Q_WS_WIN
  QProcess::startDetached('"' + QApplication::applicationDirPath() + "/schatd-ui.exe\"", QStringList() << "-show");
#else
  QProcess::startDetached(QApplication::applicationDirPath() + "/schatd-ui", QStringList() << "-show");
#endif
}


void SChatWindow::fatal()
{
  d->main->displayChoiceServer(true);
}


void SChatWindow::handleMessage(const QString &message)
{
  QStringList args = message.split(", ");

  if (args.contains("-exit")) {
    closeChat();
    return;
  }

  d->showChat();
}


void SChatWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::MiddleClick:
      if (isHidden())
        d->showChat();
      else
        d->hideChat();

    default:
      break;
  }
}


void SChatWindow::linkLeave(quint8 /*numeric*/, const QString &network, const QString &name)
{
  d->main->msg("<span class='linkLeave'>" + tr("Сервер <b>%1</b> отключился от сети <b>%2</b>").arg(Qt::escape(name)).arg(Qt::escape(network)) + "</span>");
}


/*!
 * Получение сообщения в основной канал.
 *
 * \param sender Ник отправителя.
 * \param msg    Сообщение.
 */
void SChatWindow::message(const QString &sender, const QString &msg)
{
  bool selfMsg = d->profile->nick() == sender;
  d->main->addMsg(sender, msg,
      (selfMsg ? ChatView::MsgSend : ChatView::MsgRecived) | ChatView::MsgPublic,
      selfMsg ? false : d->startNotice(d->tabs->indexOf(d->main), "Message"));
}


void SChatWindow::newLink(quint8 /*numeric*/, const QString &network, const QString &name)
{
  d->main->msg("<span class='newLink'>" + tr("Сервер <b>%1</b> подключился к сети <b>%2</b>").arg(Qt::escape(name)).arg(Qt::escape(network)) + "</span>");
}


/*!
 * Обработка изменения ника пользователя.
 */
void SChatWindow::newNick(quint8 gender, const QString &nick, const QString &newNick, const QString &name)
{
  if (d->users->isUser(nick)) {
    d->users->rename(nick, newNick);

    QPair<int, AbstractTab *> oldTab = d->tabFromName(nick);
    QPair<int, AbstractTab *> newTab = d->tabFromName(newNick);
    QString html = ChatView::statusChangedNick(gender, nick, newNick);

    if (oldTab.first != -1) {

      if (newTab.first == -1) {
        d->tabs->setTabText(oldTab.first, newNick);
        oldTab.second->setChannel(newNick);
      }
      else {
        newTab.second->msg(html);
        if (d->tabs->currentIndex() == oldTab.first)
          d->tabs->setCurrentIndex(newTab.first);
      }

      oldTab.second->msg(html);
    }

    newProfile(gender, newNick, name);
    d->main->msg(html);
  }
}


/*!
 * Обработка изменения профиля пользователя.
 * При наличии пользователя в списке функция обновляет информацию о пользователе,
 * а также если имеются открытые приваты, то вкладка также обновляется.
 *
 * \param gender Новый пол пользователя.
 * \param nick   Ник пользователя.
 * \param name   Новое полное имя.
 */
void SChatWindow::newProfile(quint8 gender, const QString &nick, const QString &name)
{
  if (d->users->isUser(nick)) {
    AbstractProfile profile(d->users->profile(nick));
    profile.setGender(gender);
    profile.setNick(nick);
    profile.setFullName(name);
    d->users->update(nick, profile);
    d->updatePrivateTab(profile);
  }
}


/*!
 * Добавление нового пользователя.
 *
 * Если не удалось добавить пользователя в \a d->users, то выходим из функции.
 * Если ник равен нашему собственному нику, то принудительно отключаем эхо.
 * Если включено эхо, добавляем в основной канал и приваты, сообщение о новом участнике
 * и если есть открытый приват вкладка также обновляется.
 *
 * Слот вызывается по событию в \a d->clientService.
 *
 * \param list    Стандартный список, содержащий в себе полные данные пользователя.
 * \param echo    Необходимость добавить в канал уведомление о новом пользователе при echo == 1.
 * \param numeric Не используется.
 */
void SChatWindow::newUser(const QStringList &list, quint8 echo, quint8 /*numeric*/)
{
  AbstractProfile profile(list);
  QString nick = profile.nick();

  if (!d->users->add(profile))
    return;

  if (d->profile->nick() == nick)
    echo = 0;

  QPair<int, AbstractTab *> tab = d->updatePrivateTab(profile);

  if (echo == 1) {
    d->main->addNewUser(profile.genderNum(), nick);

    if (tab.first != -1)
      tab.second->msg(ChatView::statusNewUser(profile.genderNum(), nick));
  }
}


void SChatWindow::onSecondsIdle(int seconds)
{
  // if activity is detected
  if (seconds == 0) {
    if (d->profile->status() == schat::StatusAutoAway)
      d->sendStatus(schat::StatusNormal);
  }

  if (seconds == d->autoAwayTime * 60)
    d->sendStatus(schat::StatusAutoAway);
}



/*!
 * Получение от извещателя запроса на показ окна чата.
 */
void SChatWindow::openChat(const QString &nick, bool pub, bool open)
{
  if (!pub) {
    QPair<int, AbstractTab *> tab = d->tabFromName(nick);
    if (tab.first != -1) {
      if (open)
        d->tabs->setCurrentIndex(tab.first);
      else
        stopNotice(tab.first);
    }
  }
  else {
    if (open)
      d->tabs->setCurrentWidget(d->main);
    else
      stopNotice(d->tabs->indexOf(d->main));
  }

  if (open)
    d->showChat();
}


/*!
 * Получение от клиентского сервиса нового приватного сообщения.
 *
 * \param flag Флаг эха, если равен \a 1 то это собственное сообщение.
 * \param nick Ник пользователя, с которым идёт разговор.
 * \param msg  Сообщение.
 */
void SChatWindow::privateMessage(quint8 flag, const QString &nick, const QString &msg)
{
  if (!d->users->isUser(nick))
    return;

  QPair<int, AbstractTab *> tabPair = d->tabFromName(nick);

  int index = tabPair.first;
  AbstractTab *tab = tabPair.second;

  if (index == -1) {
    QPair<int, AbstractTab *> newTab = d->createPrivateTab(nick);
    index = newTab.first;
    tab = newTab.second;
    d->tabs->setCurrentIndex(index);
    connect(tab, SIGNAL(nickClicked(const QString &)), d->users, SLOT(nickClicked(const QString &)));
    connect(tab, SIGNAL(emoticonsClicked(const QString &)), d->send, SLOT(insertHtml(const QString &)));
    connect(tab, SIGNAL(popupMsg(const QString &, const QString &, const QString &, bool)), d->popupManager, SLOT(popupMsg(const QString &, const QString &, const QString &, bool)));
  }

  if (tab) {
    if (flag == 1)
      tab->addMsg(d->profile->nick(), msg);
    else
      tab->addMsg(nick, msg, ChatView::MsgRecived, d->startNotice(index, "PrivateMessage"));
  }
}


void SChatWindow::sendMsg(const QString &msg)
{
  d->sendMsg(msg, true);
}


/*!
 * Показ универсального сообщения от сервера.
 *
 * \param msg Сообщение.
 */
void SChatWindow::serverMessage(const QString &msg)
{
  AbstractTab *tab = static_cast<AbstractTab *>(d->tabs->currentWidget());
  if (msg == "/pong") {
    tab->msg(d->colorizedPing());
    return;
  }

  tab->addFilteredMsg(msg);
}


/*!
 * Отображает выбранную страницу настроек.
 */
void SChatWindow::showSettingsPage(int page)
{
  if (isHidden())
    show();

  if (!d->settingsDialog) {
    d->settingsDialog = new SettingsDialog(this);
    d->settingsDialog->show();
  }

  d->settingsDialog->setPage(page);
  d->settingsDialog->activateWindow();
}


/*!
 * Обработка изменения статуса пользователем.
 */
void SChatWindow::statusChangedByUser()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (action)
    statusChangedByUser(action->data().toInt());
}


/*!
 * Обработка изменения статуса пользователем.
 */
void SChatWindow::statusChangedByUser(int index)
{
  quint32 status = d->profile->status();

  if (index == SChatWindowPrivate::StatusOffline) {
    d->clientService->quit();
    return;
  }

  if (index == SChatWindowPrivate::StatusAway) {
    if (status == schat::StatusAway)
      return;

    d->idleDetector.stop();
    d->sendStatus(schat::StatusAway);
  }
  else if (index == SChatWindowPrivate::StatusDnD) {
    if (status == schat::StatusDnD)
      return;

    d->idleDetector.stop();
    d->sendStatus(schat::StatusDnD);
  }
  else if (d->profile->status() != schat::StatusNormal)
    d->sendStatus(schat::StatusNormal);

  if (!d->clientService->isReady())
    d->clientService->connectToHost();
}


/*!
 * Обработка изменения статуса пользователем.
 */
void SChatWindow::statusChangedByUser(QAction *action)
{
  statusChangedByUser(action->data().toInt());
}


/*!
 * Обработка клавиатурных сочетаний Ctrl+1, Ctrl+2, Ctrl+3 и Ctrl+0
 * для изменения статуса.
 */
void SChatWindow::statusShortcut(int key)
{
  if (key == 0)
    statusChangedByUser(SChatWindowPrivate::StatusOffline);
  else
    statusChangedByUser(--key);
}


/*!
 * Обработка изменений настроек и прочих событий.
 */
void SChatWindow::settingsChanged(int notify)
{
  switch (notify) {
    case Settings::NetworkSettingsChanged:
    case Settings::ServerChanged:
      d->motd = true;
      d->clientService->connectToHost();
      break;

    case Settings::ProfileSettingsChanged:
      d->clientService->sendNewProfile();
      break;

    case Settings::ByeMsgChanged:
      d->clientService->sendByeMsg();
      break;

    case Settings::AwaySettingsChanged:
      d->setAwayOptions();
      break;

    default:
      break;
  }
}


/*!
 * При необходимости сбрасывает уведомление с текущей вкладки и общее уведомление в трее.
 * Если нет ни одной вкладки с уведомление о новом сообщении, то сбрасываем уведомление в трее.
 *
 * Слот вызывается при изменении текущей вкладки и при событии QEvent::WindowActivate.
 *
 * \param index Номер текущей вкладки.
 */
void SChatWindow::stopNotice(int index)
{
  if (index == -1)
    return;

  AbstractTab *tab = static_cast<AbstractTab *>(d->tabs->widget(index));
  if (tab->notice()) {
    d->tabs->setTabIcon(index, tab->icon());
    tab->notice(false);
  }

  int count = d->tabs->count();
  for (int i = 0; i < count; ++i) {
    AbstractTab *t = static_cast<AbstractTab *>(d->tabs->widget(i));
    if (t->notice())
      return;
  }

  d->tray->notice(false);
}


/*!
 * Обработка завершения синхронизации списка пользователей.
 * При необходимости высылается статус.
 */
void SChatWindow::syncUsersEnd()
{
  if (d->profile->status()) {
    d->sendStatus(d->profile->status());
    d->profile->setStatus(schat::StatusNormal);
  }
}


/*!
 * Слот вызывается когда в `d->clientService` нет активного подключения.
 */
void SChatWindow::unconnected(bool echo)
{
  d->statusUnconnected(echo);
}


/*!
 * Обработка универсального пакета.
 */
void SChatWindow::universal(quint16 sub, const QList<quint32> &data1, const QStringList &data2)
{
  if (sub == schat::UniStatusList && !data1.isEmpty() && !data2.isEmpty())
    d->universalStatus(data1, data2);
}


/*!
 * Выход удалённого пользователя из чата.
 *
 * \param nick Ник пользователя.
 * \param bye  Опциональное сообщение при выходе.
 * \param flag Флаг эха, в случае если равен \a 1, то сообщение
 *             о выходе будет добавлено в основной канал и в приват.
 */
void SChatWindow::userLeave(const QString &nick, const QString &bye, quint8 flag)
{
  if (d->users->isUser(nick)) {

    if (flag == 1) {
      AbstractProfile profile(d->users->profile(nick));
      QPair<int, AbstractTab *> tab = d->tabFromName(nick);

      d->main->addUserLeft(profile.genderNum(), nick, bye);

      if (tab.first != -1)
        tab.second->msg(ChatView::statusUserLeft(profile.genderNum(), nick, bye));
    }

    d->users->remove(nick);
  }
}


/*!
 * Обработка щелчка мыши по сообщению в трее.
 */
#ifndef SCHAT_NO_UPDATE
void SChatWindow::messageClicked()
{
  if (d->tray->message() == TrayIcon::UpdateReady)
    closeChat(true);
}
#endif


/*!
 * Фильтр событий.
 *
 * \todo Добавить контекстное меню, для заголовков вкладок.
 */
bool SChatWindow::eventFilter(QObject *object, QEvent *event)
{
  if (d->tabs == object) {
    if (event->type() == QEvent::MouseButtonRelease) {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      int index = -1;
      QTabBar *tabBar = qFindChild<QTabBar*>(d->tabs);
      for (int i = 0; i < tabBar->count(); ++i) {
        if (tabBar->tabRect(i).contains(mouseEvent->pos())) {
          index = i;
          break;
        }
      }
      if (index > -1 && (mouseEvent->button() == Qt::MidButton)) {
        closeTab(index);
        return true;
      }
    }
  }

  return QMainWindow::eventFilter(object, event);
}



/*!
 * Создание действий.
 */
void SChatWindow::createActions()
{
  // Настройка...
  d->settingsAction = new QAction(QIcon(":/images/configure.png"), tr("Настройка..."), this);
  d->settingsAction->setData(SettingsDialog::ProfilePage);
  connect(d->settingsAction, SIGNAL(triggered()), SLOT(showSettingsPage()));

  // Включить/выключить звук
  d->soundAction = d->send->soundAction().first;

  // Управление сервером...
  d->daemonAction = new QAction(QIcon(":/images/network.png"), tr("Управление сервером..."), this);
  connect(d->daemonAction, SIGNAL(triggered()), SLOT(daemonUi()));

  QActionGroup *statusActionGroup = new QActionGroup(this);
  connect(statusActionGroup, SIGNAL(triggered(QAction *)), SLOT(statusChangedByUser(QAction *)));

  d->onlineAction = statusActionGroup->addAction(QIcon(":/images/status-online.png"), tr("В сети"));
  d->onlineAction->setData(SChatWindowPrivate::StatusOnline);
  d->onlineAction->setShortcut(tr("Ctrl+1"));
  d->onlineAction->setCheckable(true);

  d->awayAction = statusActionGroup->addAction(QIcon(":/images/status-away.png"), tr("Отсутствую"));
  d->awayAction->setData(SChatWindowPrivate::StatusAway);
  d->awayAction->setShortcut(tr("Ctrl+2"));
  d->awayAction->setCheckable(true);

  d->dndAction = statusActionGroup->addAction(QIcon(":/images/status-dnd.png"), tr("Не беспокоить"));
  d->dndAction->setData(SChatWindowPrivate::StatusDnD);
  d->dndAction->setShortcut(tr("Ctrl+3"));
  d->dndAction->setCheckable(true);

  d->offlineAction = statusActionGroup->addAction(QIcon(":/images/status-offline.png"), tr("Не в сети"));
  d->offlineAction->setData(SChatWindowPrivate::StatusOffline);
  d->offlineAction->setShortcut(tr("Ctrl+0"));
  d->offlineAction->setCheckable(true);
}


/*!
 * Создаёт клиентский сервис.
 */
void SChatWindow::createService()
{
  d->pref->setApplicationProxy();

  d->clientService = new ClientService(d->profile, &d->pref->network, this);
  connect(d->clientService, SIGNAL(connecting(const QString &, bool)), SLOT(connecting(const QString &, bool)));
  connect(d->clientService, SIGNAL(unconnected(bool)), SLOT(unconnected(bool)));
  connect(d->clientService, SIGNAL(newUser(const QStringList &, quint8, quint8)), SLOT(newUser(const QStringList &, quint8, quint8)));
  connect(d->clientService, SIGNAL(accessGranted(const QString &, const QString &, quint16)), SLOT(accessGranted(const QString &, const QString &, quint16)));
  connect(d->clientService, SIGNAL(userLeave(const QString &, const QString &, quint8)), SLOT(userLeave(const QString &, const QString &, quint8)));
  connect(d->clientService, SIGNAL(accessDenied(quint16)), SLOT(accessDenied(quint16)));
  connect(d->clientService, SIGNAL(message(const QString &, const QString &)), SLOT(message(const QString &, const QString &)));
  connect(d->clientService, SIGNAL(privateMessage(quint8, const QString &, const QString &)), SLOT(privateMessage(quint8, const QString &, const QString &)));
  connect(d->clientService, SIGNAL(fatal()), SLOT(fatal()));
  connect(d->clientService, SIGNAL(syncUsersEnd()), SLOT(syncUsersEnd()));
  connect(d->clientService, SIGNAL(serverMessage(const QString &)), SLOT(serverMessage(const QString &)));
  connect(d->clientService, SIGNAL(newNick(quint8, const QString &, const QString &, const QString &)), SLOT(newNick(quint8, const QString &, const QString &, const QString &)));
  connect(d->clientService, SIGNAL(newProfile(quint8, const QString &, const QString &)), SLOT(newProfile(quint8, const QString &, const QString &)));
  connect(d->clientService, SIGNAL(newLink(quint8, const QString &, const QString &)), SLOT(newLink(quint8, const QString &, const QString &)));
  connect(d->clientService, SIGNAL(linkLeave(quint8, const QString &, const QString &)), SLOT(linkLeave(quint8, const QString &, const QString &)));
  connect(d->clientService, SIGNAL(universal(quint16, const QList<quint32> &, const QStringList &)), SLOT(universal(quint16, const QList<quint32> &, const QStringList &)));
}