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

#include <QApplication>
#include <QtGui>

#include "abstractprofile.h"
#include "emoticonsreader.h"
#include "icondefreader.h"
#include "networkreader.h"
#include "schatwindow.h"
#include "settings.h"
#include "version.h"

/*!
 * \brief Конструктор класса Settings.
 */
Settings::Settings(const QString &filename, QObject *parent)
  : AbstractSettings(filename, parent)
{
  m_profile = new AbstractProfile(this);

  QString defaultConf = qApp->applicationDirPath() + "/default.conf";
  if (QFile::exists(defaultConf))
    m_default = new QSettings(defaultConf, QSettings::IniFormat, this);
  else
    m_default = 0;
}


/*!
 * \brief Возвращает список всех смайликов которые были найдены в строке.
 */
QList<Emoticons> Settings::emoticons(const QString &text) const
{
  QList<Emoticons> out;

  if (!m_emoticons.isEmpty() && !text.isEmpty()) {
    QMapIterator <QString, QStringList> i(m_emoticons);
    while (i.hasNext()) {
      i.next();
      QStringList list = i.value();
      foreach (QString name, list) {
        if (text.contains(name)) {
          out << Emoticons(name, i.key());
        }
      }
    }
  }

  return out;
}


QStandardItem* Settings::findItem(const QStandardItemModel *model, const QString &text, Qt::MatchFlags flags, int column)
{
  QList<QStandardItem *> items;

  items = model->findItems(text, flags, column);
  if (items.size() > 0)
    return items[0];
  else
    return 0;
}


/*!
 * \brief Создаёт карту смайликов.
 */
void Settings::createEmoticonsMap()
{
  QString emoticonsPath = qApp->applicationDirPath() + "/emoticons/" + getString("EmoticonTheme");
  bool err = true;

  if (QFile::exists(emoticonsPath + "/icondef.xml")) {
    IconDefReader reader(&m_emoticons);
    if (reader.readFile(emoticonsPath + "/icondef.xml")) {
      err = false;
      if (reader.refresh())
        setInt("EmoticonsRefreshTime", reader.refresh());
      else
        setInt("EmoticonsRefreshTime", 50);
    }
  }
  else if (QFile::exists(emoticonsPath + "/emoticons.xml")) {
    EmoticonsReader reader(&m_emoticons);
    if (reader.readFile(emoticonsPath + "/emoticons.xml"))
      err = false;
  }

  if (err) {
    m_emoticons.clear();
    setBool("UseEmoticons", false);
  }
}


void Settings::notify(int notify)
{
  bool readyToInstall = false;

  switch (notify) {
    case UpdateError:
    case UpdateAvailable:
    case UpdateNoAvailable:
    case UpdateReady:
      if (m_update)
        m_update->deleteLater();

      if (notify == UpdateReady)
        readyToInstall = true;

      setBool("Updates/ReadyToInstall", readyToInstall);
      break;

    default:
      break;
  }

  emit changed(notify);
}


void Settings::notify(int notify, int index)
{
  switch (notify) {
    case NetworksModelIndexChanged:
      emit networksModelIndexChanged(index);
      break;

    default:
      break;
  }
}


/*!
 * Чтение настроек.
 */
void Settings::read()
{
  m_pos  = m_settings->value("Pos", QPoint(-999, -999)).toPoint();
  m_size = m_settings->value("Size", QSize(640, 430)).toSize();
  m_splitter = m_settings->value("Splitter").toByteArray();

  setBool("HideWelcome",            false);
  setBool("FirstRun",               true);
  setBool("EmoticonsRequireSpaces", true);
  setBool("UseEmoticons",           true);
  setBool("UseAnimatedEmoticons",   true);
  setInt("EmoticonsRefreshTime",    50);
  setString("Style",                "Plastique");
  setString("EmoticonTheme",        "kolobok");
  setString("Network",              "SimpleNet.xml");
  setList("RecentServers",          QStringList());

  #ifndef SCHAT_NO_UPDATE
    int interval = m_settings->value("Updates/CheckInterval", 60).toInt(); /// \todo Не учитывается default.conf
    if (interval < 5)
      interval = 5;
    else if (interval > 1440)
      interval = 1440;

    setInt("Updates/CheckInterval", interval);
    setInt("Updates/LevelQt", UpdateLevelQt);
    setInt("Updates/LevelCore", UpdateLevelCore);
    setInt("Updates/DownloadSize", 0);
    setBool("Updates/AutoClean", true);
    setBool("Updates/AutoDownload", true);
    setString("Updates/LastVersion", QApplication::applicationVersion());
    setString("Updates/Url", "http://192.168.5.1/schat/updates/update.xml"); /// \todo Удалить

    setList("Updates/Mirrors", QStringList() << "http://192.168.5.1/schat/updates/update.xml"); /// \todo Адрес должен быть в интернете.
  #endif

  if (m_default)
    AbstractSettings::read(m_default);

  AbstractSettings::read();

  qApp->setStyle(getString("Style"));

  network.fromConfig(getString("Network"));
  createServerList();

  m_settings->beginGroup("Profile");
  m_profile->setNick(m_settings->value("Nick",     QDir::home().dirName()).toString());
  m_profile->setFullName(m_settings->value("Name", "").toString());
  m_profile->setGender(m_settings->value("Gender", "male").toString());
  m_profile->setByeMsg(m_settings->value("Bye",    "IMPOMEZIA Simple Chat").toString());
  m_settings->endGroup();

  createEmoticonsMap();
}


/*!
 * Запись настроек.
 */
void Settings::write()
{
  m_ro << "EmoticonsRefreshTime"
       << "Updates/Mirrors"
       << "Updates/LevelQt"
       << "Updates/LevelCore"
       << "Updates/LastVersion"
       << "Updates/DownloadSize";

  setBool("FirstRun", false);
  setString("Network", network.config());
  saveRecentServers();

  AbstractSettings::write();

  m_settings->setValue("Size", m_size);
  m_settings->setValue("Pos", m_pos);
  m_settings->setValue("Splitter", m_splitter);

  m_settings->beginGroup("Profile");
  m_settings->setValue("Nick", m_profile->nick());
  m_settings->setValue("Name", m_profile->fullName());
  m_settings->setValue("Gender", m_profile->gender());
  m_settings->setValue("Bye", m_profile->byeMsg());
  m_settings->endGroup();
}


void Settings::update()
{
  if (!m_update) {
    m_update = new Update(this);
    m_update->execute();
  }
}


/*!
 * Создаёт список сетей и одиночных серверов.
 */
void Settings::createServerList()
{
  QString networksPath = qApp->applicationDirPath() + "/networks/";

  QDir directory(networksPath);
  directory.setNameFilters(QStringList() << "*.xml");
  QStringList files = directory.entryList(QDir::Files | QDir::NoSymLinks);
  NetworkReader network;

  foreach (QString file, files)
    if (network.readFile(networksPath + file)) {
      if (!findItem(&networksModel, network.networkName())) {
        QStandardItem *item = new QStandardItem(QIcon(":/images/network.png"), network.networkName());
        item->setData(file, Qt::UserRole);
        networksModel.appendRow(item);
      }
    }

  QStringList recent = getList("RecentServers");

  if (!recent.isEmpty())
    foreach (QString server, recent) {
      QStringList list = server.split(':');
      if (list.size() == 2) {
        QStandardItem *item = new QStandardItem(QIcon(":/images/host.png"), list.at(0));
        item->setData(list.at(1).toInt(), Qt::UserRole);
        networksModel.appendRow(item);
      }
    }
}


/*!
 * Функция проходится по модели \a networksModel и выделяет из неё одиночные серверы,
 * для записи в файл настроек (ключ "RecentServers").
 */
void Settings::saveRecentServers()
{
  QStringList list;

  for (int row = 0; row < networksModel.rowCount(); ++row) {
    QStandardItem *item = networksModel.item(row);
    if (item->data(Qt::UserRole).type() == QVariant::Int)
      list << (item->text() + ':' + item->data(Qt::UserRole).toString());
  }

  setList("RecentServers", list);
}
