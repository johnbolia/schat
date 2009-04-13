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

#include "daemonsettings.h"
#include "daemonsettingsdialog.h"
#include "network.h"
#include "networkwriter.h"
#include "serviceinstaller.h"

/*!
 * \brief Конструктор класса DaemonSettingsDialog.
 */
DaemonSettingsDialog::DaemonSettingsDialog(QWidget *parent)
  : AbstractSettingsDialog(parent)
{
  DaemonNetSettings *netPage = new DaemonNetSettings(this);

  createPage(QIcon(":/images/daemonsettings.png"),        tr("Общие"),  new DaemonCommonSettings(this));
  createPage(QIcon(":/images/applications-internet.png"), tr("Сеть"),   netPage);
  #ifndef SCHATD_NO_SERVICE
    createPage(QIcon(":/images/windows-service.png"),     tr("Сервис"), new DaemonServiceSettings(this));
  #endif

  connect(netPage, SIGNAL(validInput(bool)), m_okButton, SLOT(setEnabled(bool)));
}


void DaemonSettingsDialog::accept()
{
  emit save();
  DaemonSettingsInstance->write();
  close();
}


/*!
 * Конструктор класса DaemonCommonSettings.
 */
DaemonCommonSettings::DaemonCommonSettings(QWidget *parent)
  : AbstractSettingsPage(DaemonSettingsDialog::CommonPage, parent)
{
  m_listen = new QComboBox(this);
  m_listen->setToolTip(tr("Адрес на котором сервер будет ожидать подключения"));

  createListenList();

  m_port = new QSpinBox(this);
  m_port->setRange(1, 65536);
  m_port->setValue(DaemonSettingsInstance->getInt("ListenPort"));
  m_port->setToolTip(tr("Порт на котором сервер будет ожидать подключения"));

  QLabel *listenLabel = new QLabel(tr("&Адрес:"), this);
  listenLabel->setBuddy(m_listen);

  QLabel *portLabel = new QLabel(tr("&Порт:"), this);
  portLabel->setBuddy(m_port);

  QGroupBox *listenGroup = new QGroupBox(tr("Интерфейс сервера"), this);
  QGridLayout *listenLay = new QGridLayout(listenGroup);
  listenLay->addWidget(listenLabel, 0, 0);
  listenLay->addWidget(m_listen, 0, 1);
  listenLay->addWidget(portLabel, 0, 2);
  listenLay->addWidget(m_port, 0, 3);
  listenLay->setColumnStretch(1, 1);
  listenLay->setMargin(6);
  listenLay->setSpacing(4);

  m_logLevel = new QSpinBox(this);
  m_logLevel->setRange(-1, 0);
  m_logLevel->setValue(DaemonSettingsInstance->getInt("LogLevel"));
  m_logLevel->setToolTip(tr("Уровень детализации журнала\n-1 журналирование отключено"));

  QLabel *logLabel = new QLabel(tr("&Уровень журналирования:"), this);
  logLabel->setBuddy(m_logLevel);

  m_channelLog = new QCheckBox(tr("Вести журнал &главного канала"), this);
  m_channelLog->setChecked(DaemonSettingsInstance->getBool("ChannelLog"));
  m_channelLog->setToolTip(tr("Управляет режимом записи событий основного\nканала в специальный журнал"));

  QGroupBox *logGroup = new QGroupBox(tr("Журналирование"), this);
  QGridLayout *logLay = new QGridLayout(logGroup);
  logLay->addWidget(logLabel, 0, 0);
  logLay->addWidget(m_logLevel, 0, 1, 1, 2);
  logLay->addWidget(m_channelLog, 1, 0, 1, 3);
  logLay->setColumnStretch(3, 1);
  logLay->setMargin(6);
  logLay->setSpacing(4);

  m_maxUsers = new QSpinBox(this);
  m_maxUsers->setRange(0, 10000);
  m_maxUsers->setValue(DaemonSettingsInstance->getInt("MaxUsers"));
  m_maxUsers->setToolTip(tr("Ограничение максимального количества\nпользователей которые могут быть подключены к серверу\n0 - без ограничений"));

  m_maxUsersPerIp = new QSpinBox(this);
  m_maxUsersPerIp->setRange(0, 10000);
  m_maxUsersPerIp->setValue(DaemonSettingsInstance->getInt("MaxUsersPerIp"));
  m_maxUsersPerIp->setToolTip(tr("Ограничение максимального количества\nпользователей с одного адреса\n0 - без ограничений"));

  QLabel *maxUsersLabel = new QLabel(tr("&Лимит пользователей:"), this);
  maxUsersLabel->setBuddy(m_maxUsers);
  QLabel *maxUsersPerIpLabel = new QLabel(tr("Лимит подключений с &одного адреса:"), this);
  maxUsersPerIpLabel->setBuddy(m_maxUsersPerIp);

  QGroupBox *limitsGroup = new QGroupBox(tr("Ограничения"), this);
  QGridLayout *limitsLay = new QGridLayout(limitsGroup);
  limitsLay->addWidget(maxUsersLabel, 0, 0);
  limitsLay->addWidget(m_maxUsers, 0, 1);
  limitsLay->addWidget(maxUsersPerIpLabel, 1, 0);
  limitsLay->addWidget(m_maxUsersPerIp, 1, 1);
  limitsLay->setColumnStretch(2, 1);
  limitsLay->setMargin(6);
  limitsLay->setSpacing(4);

  QVBoxLayout *mainLay = new QVBoxLayout(this);
  mainLay->addWidget(listenGroup);
  mainLay->addWidget(logGroup);
  mainLay->addWidget(limitsGroup);
  mainLay->addStretch();
  mainLay->setContentsMargins(3, 3, 3, 0);
}


/*!
 * Сброс настроек на стандартные.
 *
 * \param page Номер текущей страницы настроек, только если номер страницы равен \a m_id, производится сброс настроек.
 */
void DaemonCommonSettings::reset(int page)
{
  if (page == m_id) {
    m_listen->setCurrentIndex(0);
    m_port->setValue(7666);
    m_logLevel->setValue(0);
    m_channelLog->setChecked(false);
    m_maxUsers->setValue(0);
    m_maxUsersPerIp->setValue(0);
  }
}


/*!
 * \brief Сохранение настроек.
 *
 * Настройки записываются \a m_settings и фактически будут записаны немного позднее при закрытии диалога.
 *
 * \sa DaemonUiSettings
 */
void DaemonCommonSettings::save()
{
  DaemonSettingsInstance->setString("ListenAddress", m_listen->currentText());
  DaemonSettingsInstance->setInt("ListenPort",       m_port->value());
  DaemonSettingsInstance->setInt("LogLevel",         m_logLevel->value());
  DaemonSettingsInstance->setBool("ChannelLog",      m_channelLog->isChecked());
  DaemonSettingsInstance->setInt("MaxUsers",         m_maxUsers->value());
  DaemonSettingsInstance->setInt("MaxUsersPerIp",    m_maxUsersPerIp->value());
}


/*!
 * Добавляет в список доступных интерфейсов, для опции "ListenAddress",
 * адреса локальных интерфейсов, также первым пунктом всегда идёт "0.0.0.0".
 */
void DaemonCommonSettings::createListenList()
{
  m_listen->addItem("0.0.0.0");

  QList<QHostAddress> list = QNetworkInterface::allAddresses();
  foreach (QHostAddress addr, list)
    m_listen->addItem(addr.toString());

  QString listenAddress = DaemonSettingsInstance->getString("ListenAddress");
  int index = m_listen->findText(listenAddress);
  if (index == -1)
    m_listen->setCurrentIndex(0);
  else
    m_listen->setCurrentIndex(index);
}


/*!
 * Конструктор класса DaemonNetSettings.
 */
DaemonNetSettings::DaemonNetSettings(QWidget *parent)
  : AbstractSettingsPage(DaemonSettingsDialog::NetPage, parent)
{
  m_network = new QCheckBox(tr("Разрешить поддержку &сети"), this);
  m_network->setChecked(DaemonSettingsInstance->getBool("Network"));
  m_network->setToolTip(tr("Включить поддержку взаимодействия с другими серверами"));
  connect(m_network, SIGNAL(clicked(bool)), SLOT(enableAll()));

  m_root = new QCheckBox(tr("&Корневой сервер"), this);
  m_root->setChecked(DaemonSettingsInstance->getBool("RootServer"));
  m_root->setToolTip(tr("Определяет роль этого сервера в сети"));
  connect(m_root, SIGNAL(clicked(bool)), SLOT(changeRole(bool)));

  m_netName = new QLineEdit("Unknown Network", this);
  m_netName->setMaxLength(Network::MaxName);
  m_netName->setToolTip(tr("Название сети"));
  connect(m_netName, SIGNAL(textChanged(const QString &)), SLOT(inputChanged(const QString &)));
  QLabel *netNameLabel = new QLabel(tr("&Название:"), this);
  netNameLabel->setBuddy(m_netName);

  m_key = new QLineEdit(this);
  m_key->setMaxLength(Network::MaxKey);
  m_key->setEchoMode(QLineEdit::Password);
  m_key->setToolTip(tr("Уникальный ключ сети\nДолжен быть одинаков на всех серверах"));
  connect(m_key, SIGNAL(textChanged(const QString &)), SLOT(inputChanged(const QString &)));
  QLabel *keyLabel = new QLabel(tr("&Ключ:"), this);
  keyLabel->setBuddy(m_key);

  m_rootAddr = new QLineEdit(this);
  m_rootAddr->setToolTip(tr("Адрес корневого сервера\nЭто поле не используется корневым сервером"));
  connect(m_rootAddr, SIGNAL(textChanged(const QString &)), SLOT(inputChanged(const QString &)));
  QLabel *rootLabel = new QLabel(tr("Ко&рневой сервер:"), this);
  rootLabel->setBuddy(m_rootAddr);

  m_netGroup = new QGroupBox(tr("Сеть"), this);
  QGridLayout *netLay = new QGridLayout(m_netGroup);
  netLay->addWidget(netNameLabel, 0, 0);
  netLay->addWidget(m_netName, 0, 1);
  netLay->addWidget(keyLabel, 1, 0);
  netLay->addWidget(m_key, 1, 1);
  netLay->addWidget(rootLabel, 2, 0);
  netLay->addWidget(m_rootAddr, 2, 1);
  netLay->setMargin(6);
  netLay->setSpacing(4);

  m_name = new QLineEdit("Unknown Server", this);
  if (!DaemonSettingsInstance->getString("Name").isEmpty())
    m_name->setText(DaemonSettingsInstance->getString("Name"));
  m_name->setMaxLength(Network::MaxName);
  m_name->setToolTip(tr("Имя данного сервера\nРекомендуется указывать реальное DNS имя"));
  connect(m_name, SIGNAL(textChanged(const QString &)), SLOT(inputChanged(const QString &)));
  QLabel *nameLabel = new QLabel(tr("Имя:"), this);
  nameLabel->setBuddy(m_name);
  QHBoxLayout *nameLay = new QHBoxLayout;
  nameLay->addWidget(nameLabel);
  nameLay->addWidget(m_name);
  nameLay->setMargin(0);
  nameLay->setSpacing(4);

  m_numeric = new QSpinBox(this);
  m_numeric->setRange(1, 255);
  m_numeric->setValue(DaemonSettingsInstance->getInt("Numeric"));
  m_numeric->setToolTip(tr("Уникальный для сети номер сервера"));
  QLabel *numericLabel = new QLabel(tr("Уникальный &номер:"), this);
  numericLabel->setBuddy(m_numeric);

  m_limit = new QSpinBox(this);
  m_limit->setRange(0, 255);
  m_limit->setValue(DaemonSettingsInstance->getInt("MaxLinks"));
  m_limit->setToolTip(tr("Ограничение количества серверов которые могут быть подключены\nИспользуется только корневым сервером"));
  QLabel *limitLabel = new QLabel(tr("&Максимум серверов:"), this);
  limitLabel->setBuddy(m_limit);

  m_daemonGroup = new QGroupBox(tr("Сервер"), this);
  QGridLayout *daemonLay = new QGridLayout(m_daemonGroup);
  daemonLay->addLayout(nameLay, 0, 0, 1, 3);
  daemonLay->addWidget(numericLabel, 1, 0);
  daemonLay->addWidget(m_numeric, 1, 1);
  daemonLay->addWidget(limitLabel, 2, 0);
  daemonLay->addWidget(m_limit, 2, 1);
  daemonLay->setColumnStretch(2, 1);
  netLay->setMargin(6);
  netLay->setSpacing(4);

  QVBoxLayout *mainLay = new QVBoxLayout(this);
  mainLay->addWidget(m_network);
  mainLay->addWidget(m_root);
  mainLay->addWidget(m_netGroup);
  mainLay->addWidget(m_daemonGroup);
  mainLay->addStretch();
  mainLay->setContentsMargins(3, 3, 3, 0);

  m_normal = m_netName->palette();
  m_red = m_normal;
  m_red.setColor(QPalette::Active, QPalette::Base, QColor("#f66"));

  changeRole(m_root->isChecked());
  readNetwork();

  QTimer::singleShot(0, this, SLOT(enableAll()));
}


/*!
 * \brief Сброс настроек на стандартные.
 *
 * \param page Номер текущей страницы настроек, только если номер страницы равен \a m_id, производится сброс настроек.
 */
void DaemonNetSettings::reset(int page)
{
  if (page == m_id) {
    m_network->setChecked(false);
    m_root->setChecked(false);
    m_netName->setText("Unknown Server");
    m_key->setText("");
    m_rootAddr->setText("");
    m_name->setText("Unknown Server");
    m_numeric->setValue(1);
    m_limit->setValue(10);
    enableAll();
    changeRole(false);
  }
}


/*!
 * \brief Сохранение настроек.
 *
 * Настройки записываются \a m_settings и фактически будут записаны немного позднее при закрытии диалога.
 * Настройки сети (xml-файл), записываются сразу же, используется класс NetworkWriter.
 *
 * \sa DaemonUiSettings, NetworkWriter
 */
void DaemonNetSettings::save()
{
  DaemonSettingsInstance->setBool("Network", m_network->isChecked());

  if (m_network->isChecked()) {
    DaemonSettingsInstance->setBool("RootServer", m_root->isChecked());
    DaemonSettingsInstance->setInt("Numeric",     m_numeric->value());
    DaemonSettingsInstance->setInt("MaxLinks",    m_limit->value());
    DaemonSettingsInstance->setString("Name",     m_name->text());

    m_meta.insert("name", m_netName->text());
    m_meta.insert("key", m_key->text());
    QString server;
    if (!m_root->isChecked())
      server = m_rootAddr->text();

    NetworkWriter writer(m_meta, server.isEmpty() ? QStringList() : QStringList() << server);
    writer.writeFile(QApplication::applicationDirPath() + "/" + DaemonSettingsInstance->getString("NetworkFile"));
  }
}


/*!
 * Изменение роли сервера, если сервер является корневым, происходит отключение выбора адреса
 * корневого сервера, иначе происходит отключение управления максимальным количеством серверов.
 */
void DaemonNetSettings::changeRole(bool root)
{
  m_rootAddr->setEnabled(!root);
  m_limit->setEnabled(root);
}


/*!
 * Управляет состоянием виджетов в зависимости от выбранных настроек.
 * Если поддержка сети разрешена, то устанавливает красную палитру для виджетов с не корректным вводом.
 * При отключенной поддержки сети, все виджеты отключаются.
 * В конце высылается сигнал validInput(bool valid)
 */
void DaemonNetSettings::enableAll()
{
  bool enable = m_network->isChecked();

  if (enable) {
    if (m_netName->text().isEmpty())
      m_netName->setPalette(m_red);
    if (m_key->text().isEmpty())
      m_key->setPalette(m_red);
    if (m_rootAddr->text().isEmpty())
      m_rootAddr->setPalette(m_red);
    if (m_name->text().isEmpty())
      m_name->setPalette(m_red);
  }
  m_root->setEnabled(enable);
  m_netGroup->setEnabled(enable);
  m_daemonGroup->setEnabled(enable);
  emit validInput(revalidate());
}


/*!
 * Универсальный слот вызываемый при изменении текста в одном из полей ввода.
 * Если новый текст пустой виджету устанавливается красная палитра \a m_red.
 * В конце высылается сигнал validInput(bool valid)
 */
void DaemonNetSettings::inputChanged(const QString &/*text*/)
{
  QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());

  if (lineEdit) {
    if (lineEdit->text().isEmpty())
      lineEdit->setPalette(m_red);
    else
      lineEdit->setPalette(m_normal);
  }

  emit validInput(revalidate());
}


/*!
 *\return \a true если ввод корректный и \a false если не корректный и требуется запретить кнопку "OK".
 */
bool DaemonNetSettings::revalidate()
{
  if (!m_network->isChecked())
    return true;

  if (m_netName->text().isEmpty() || m_key->text().isEmpty() || m_name->text().isEmpty())
    return false;

  if (m_rootAddr->text().isEmpty() && !m_root->isChecked())
    return false;

  return true;
}


/*!
 * \brief Чтение файла сети.
 *
 * \sa Network
 */
void DaemonNetSettings::readNetwork()
{
  Network network(QApplication::applicationDirPath(), this);
  network.setSingle(true);
  network.setFailBack(false);
  if (network.fromFile(DaemonSettingsInstance->getString("NetworkFile"))) {
    if (!network.name().isEmpty())
      m_netName->setText(network.name());
    m_key->setText(network.key());

    m_meta.insert("description", network.description());
    m_meta.insert("site", network.site());

    ServerInfo info = network.server();
    QString server = info.address;
    if (info.port != 7666)
      server += (":" + QString().number(info.port));

    m_rootAddr->setText(server);
  }
}


/*!
 * Конструктор класса DaemonCommonSettings.
 */
#ifndef SCHATD_NO_SERVICE
DaemonServiceSettings::DaemonServiceSettings(QWidget *parent)
  : AbstractSettingsPage(DaemonSettingsDialog::CommonPage, parent),
  m_installer(new ServiceInstaller(this)),
  m_status(Unknown)
{
  QLabel *state = new QLabel(tr("Windows сервис:"), this);
  m_state = new QLabel(tr("<b style='color:#c00;'>не определено</b>"), this);

  QLabel *instsrvExe = new QLabel("instsrv.exe:", this);
  m_instsrvExe = new QLabel(this);

  QLabel *srvanyExe = new QLabel("srvany.exe:", this);
  m_srvanyExe = new QLabel(this);

  QGroupBox *infoGroup = new QGroupBox(tr("Информация"), this);
  QGridLayout *infoLay = new QGridLayout(infoGroup);
  infoLay->addWidget(state, 0, 0);
  infoLay->addWidget(m_state, 0, 1);
  infoLay->addWidget(instsrvExe, 1, 0);
  infoLay->addWidget(m_instsrvExe, 1, 1);
  infoLay->addWidget(srvanyExe, 2, 0);
  infoLay->addWidget(m_srvanyExe, 2, 1);
  infoLay->setColumnStretch(1, 1);
  infoLay->setMargin(6);
  infoLay->setSpacing(4);

  QLabel *serviceName = new QLabel(tr("Имя сервиса:"), this);
  m_serviceName = new QLineEdit(DaemonSettingsInstance->getString("Service/Name"), this);
  m_serviceName->setMaxLength(128);

  m_install = new QCommandLinkButton(tr("Установить"), tr("Установить сервер как сервис"), this);

  m_installGroup = new QGroupBox(tr("Установка/Удаление"), this);
  QGridLayout *installLay = new QGridLayout(m_installGroup);
  installLay->addWidget(serviceName, 0, 0);
  installLay->addWidget(m_serviceName, 0, 1);
  installLay->addWidget(m_install, 1, 0, 1, 2);
  installLay->setMargin(6);
  installLay->setSpacing(4);

  m_info = new QLabel(tr("Для установки сервиса необходимо скопировать файлы <b>instsrv.exe</b> и <b>srvany.exe</b> "
      "из комплекта <b>Windows Resource Kit</b> в папку с сервером.<br />"
      "<a href='http://simple.impomezia.com/Windows_Service' style='text-decoration:none; color:#1a4d82;'>Подробности</a>."), this);
  m_info->setStyleSheet("border: 1px solid #7581a9;"
      "border-radius: 3px;"
      "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f5f6ff, stop:1 #f2f2ff)");
  m_info->setOpenExternalLinks(true);
  m_info->setWordWrap(true);

  QVBoxLayout *mainLay = new QVBoxLayout(this);
  mainLay->addWidget(infoGroup);
  mainLay->addWidget(m_installGroup);
  mainLay->addWidget(m_info);
  mainLay->addStretch();
  mainLay->setContentsMargins(3, 3, 3, 0);

  detect();

  connect(m_install, SIGNAL(clicked(bool)), SLOT(clicked()));
  connect(m_serviceName, SIGNAL(textChanged(const QString &)), SLOT(serviceNameChanged(const QString &)));
}


/*!
 * Сброс настроек на стандартные.
 */
void DaemonServiceSettings::reset(int page)
{
  if (page == m_id) {
  }
}


/*!
 * Сохранение настроек.
 */
void DaemonServiceSettings::save()
{
}


void DaemonServiceSettings::clicked()
{
  if (m_status == ReadyToInstall)
    m_installer->install(m_serviceName->text());
  else if (m_status == Installed)
    m_installer->uninstall(m_serviceName->text());
}


void DaemonServiceSettings::serviceNameChanged(const QString &text)
{
  if (text.isEmpty() || ServiceInstaller::exists(text))
    m_install->setEnabled(false);
  else
    m_install->setEnabled(true);
}


/*!
 * Проверка на наличие необходимого файла из Windows Resource Kit.
 *
 * \param label Указатель на объект QLabel, для отображения состояния.
 * \param file  Имя файла, который нужно найти (без пути).
 *
 * \return \a true если файл найден.
 */
bool DaemonServiceSettings::exist(QLabel *label, const QString &file) const
{
  if (!QFile::exists(QApplication::applicationDirPath() + "/" + file)) {
    label->setText(tr("<b style='color:#c00;'>не найден</b>"));
    return false;
  }

  label->setText(tr("<b style='color:#090;'>найден</b>"));
  return true;
}


/*!
 * Определение текущего состояния сервиса.
 */
void DaemonServiceSettings::detect()
{
  m_status = Unknown;

  if (!exist(m_instsrvExe, "instsrv.exe"))
    m_status = Invalid;

  if (!exist(m_srvanyExe, "srvany.exe"))
    m_status = Invalid;

  if (m_status == Invalid) {
    m_installGroup->setEnabled(false);
    m_info->setVisible(true);
    setCommandLinkState();
    return;
  }

  if (DaemonSettingsInstance->getBool("Service/Installed")) {
    m_serviceName->setEnabled(false);
    m_state->setText(tr("<b style='color:#090;'>установлен</b>"));
    m_status = Installed;
  }
  else {
    m_serviceName->setEnabled(true);
    m_state->setText(tr("<b style='color:#c00;'>не установлен</b>"));
    m_status = ReadyToInstall;
  }

  m_installGroup->setEnabled(true);
  m_info->setVisible(false);
  setCommandLinkState();
}


void DaemonServiceSettings::setCommandLinkState()
{
  if (m_status != Installed) {
    m_install->setText(tr("Установить"));
    m_install->setDescription(tr("Установить сервер как сервис"));
  }
  else {
    m_install->setText(tr("Удалить"));
    m_install->setDescription(tr("Удалить сервис сервера чата"));
  }
}
#endif
