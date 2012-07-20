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

#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QThreadPool>
#include <QTimer>
#include <QVariant>

#include "Ch.h"
#include "DataBase.h"
#include "DateTime.h"
#include "feeds/FeedStorage.h"
#include "JSON.h"
#include "net/SimpleID.h"
#include "NodeLog.h"
#include "Path.h"
#include "sglobal.h"
#include "Storage.h"
#include "tools/Ver.h"

bool DataBase::noMaster = false;
DataBase *DataBase::m_self = 0;

AddHostTask::AddHostTask(Host *host)
    : m_host(*host)
{
}


/*!
 * Запуск задачи.
 */
void AddHostTask::run()
{
  qint64 key = this->key(m_host.hostId);
  if (key == -1)
    add();
  else
    update(key);
}


/*!
 * Получение ключа в таблице \b hosts на основе идентификатора хоста, возвращает -1 если хост не найден.
 */
qint64 AddHostTask::key(const QByteArray &hostId)
{
  QSqlQuery query;
  query.prepare(LS("SELECT id FROM hosts WHERE hostId = :hostId LIMIT 1;"));
  query.bindValue(LS(":hostId"), SimpleID::encode(hostId));
  query.exec();

  if (!query.first())
    return -1;

  return query.value(0).toLongLong();
}


/*!
 * Добавление новой записи.
 */
void AddHostTask::add()
{
  QSqlQuery query;
  query.prepare(LS("INSERT INTO hosts (channel,  hostId,  name,  address,  version,  os,  osName,  tz,  date,  geo,  data)"
                             " VALUES (:channel, :hostId, :name, :address, :version, :os, :osName, :tz, :date, :geo, :data)"));

  query.bindValue(LS(":channel"), m_host.channel);
  query.bindValue(LS(":hostId"),  SimpleID::encode(m_host.hostId));
  query.bindValue(LS(":name"),    m_host.name);
  query.bindValue(LS(":address"), m_host.address);
  query.bindValue(LS(":version"), Ver(m_host.version).toString());
  query.bindValue(LS(":os"),      m_host.os);
  query.bindValue(LS(":osName"),  m_host.osName);
  query.bindValue(LS(":tz"),      m_host.tz);
  query.bindValue(LS(":date"),    m_host.date);
  query.bindValue(LS(":geo"),     JSON::generate(m_host.geo));
  query.bindValue(LS(":data"),    JSON::generate(m_host.data));
  query.exec();
}


/*!
 * Обновление информации об хосте.
 */
void AddHostTask::update(qint64 key)
{
  QSqlQuery query;
  query.prepare(LS("UPDATE hosts SET name = :name, address = :address, version = :version, os = :os, osName = :osName, tz = :tz, date = :date, geo = :geo, data = :data WHERE id = :id;"));
  query.bindValue(LS(":name"),    m_host.name);
  query.bindValue(LS(":address"), m_host.address);
  query.bindValue(LS(":version"), Ver(m_host.version).toString());
  query.bindValue(LS(":os"),      m_host.os);
  query.bindValue(LS(":osName"),  m_host.osName);
  query.bindValue(LS(":tz"),      m_host.tz);
  query.bindValue(LS(":date"),    m_host.date);
  query.bindValue(LS(":geo"),     JSON::generate(m_host.geo));
  query.bindValue(LS(":data"),    JSON::generate(m_host.data));
  query.bindValue(LS(":id"),      key);
  query.exec();
}


/*!
 * База данных сервера.
 */
DataBase::DataBase(QObject *parent)
  : QObject(parent)
{
  m_self = this;
  m_pool = new QThreadPool(this);
  m_pool->setMaxThreadCount(1);
}


int DataBase::start()
{
  QSqlDatabase db = QSqlDatabase::addDatabase(LS("QSQLITE"));
  QDir().mkpath(Storage::varPath());

  db.setDatabaseName(Storage::varPath() + LC('/') + Path::app() + LS(".sqlite"));
  if (!db.open()) {
    SCHAT_LOG_FATAL("Could not open DataBase file" << db.databaseName() << ":" << db.lastError())
    QCoreApplication::exit(-1);
    return -1;
  }

  QSqlQuery query;
  query.exec(LS("PRAGMA synchronous = OFF"));
  QStringList tables = db.tables();

  query.exec(LS(
    "CREATE TABLE IF NOT EXISTS accounts ( "
    "  id         INTEGER PRIMARY KEY,"
    "  channel    INTEGER UNIQUE,"
    "  date       INTEGER DEFAULT ( 0 ),"
    "  cookie     BLOB    NOT NULL UNIQUE,"
    "  provider   TEXT,"
    "  flags      INTEGER DEFAULT ( 0 ),"
    "  groups     TEXT,"
    "  data       BLOB"
    ");"
  ));

  if (!tables.contains(LS("groups"))) {
    query.exec(LS(
    "CREATE TABLE groups ( "
    "  id          INTEGER PRIMARY KEY,"
    "  name        TEXT    NOT NULL UNIQUE ,"
    "  permissions TEXT"
    ");"));

    addGroup(LS("master"));
    addGroup(LS("registered"));
    addGroup(LS("anonymous"));
  }

  if (!tables.contains(LS("channels"))) {
    query.exec(LS(
    "CREATE TABLE channels ( "
    "  id         INTEGER PRIMARY KEY,"
    "  channel    BLOB    NOT NULL UNIQUE,"
    "  normalized BLOB    NOT NULL UNIQUE,"
    "  type       INTEGER DEFAULT ( 73 ),"
    "  gender     INTEGER DEFAULT ( 0 ),"
    "  status     INTEGER DEFAULT ( 0 ),"
    "  account    INTEGER DEFAULT ( 0 ),"
    "  name       TEXT    NOT NULL,"
    "  data       BLOB,"
    "  date       INTEGER DEFAULT ( 0 )"
    ");"));

    noMaster = true;
  }

  query.exec(LS(
    "CREATE TABLE IF NOT EXISTS hosts ( "
    "  id         INTEGER PRIMARY KEY,"
    "  channel    INTEGER,"
    "  hostId     BLOB NOT NULL UNIQUE,"
    "  name       TEXT,"
    "  address    TEXT,"
    "  version    TEXT,"
    "  os         INTEGER,"
    "  osName     TEXT,"
    "  tz         INTEGER,"
    "  date       INTEGER,"
    "  geo        BLOB,"
    "  data       BLOB"
    ");"
  ));

  version();
  return 0;
}


qint64 DataBase::addGroup(const QString &name, const QString &permissions)
{
  QSqlQuery query;
  query.prepare(LS("INSERT INTO groups (name, permissions) VALUES (:name, :permissions);"));
  query.bindValue(LS(":name"), name);
  query.bindValue(LS(":permissions"), permissions);
  query.exec();

  if (query.numRowsAffected() <= 0)
    return -1;

  return query.lastInsertId().toLongLong();
}


bool DataBase::isCollision(const QByteArray &id, const QByteArray &normalized, int type)
{
  QSqlQuery query;
  query.prepare(LS("SELECT channel FROM channels WHERE normalized = :normalized AND type = :type LIMIT 1;"));
  query.bindValue(LS(":normalized"), normalized);
  query.bindValue(LS(":type"), type);
  query.exec();

  if (!query.first())
    return false;

  return query.value(0).toByteArray() != id;
}


/*!
 * Получение канала на основе идентификатора канала или нормализированного имени и типа канала.
 */
ChatChannel DataBase::channel(const QByteArray &id, int type)
{
  qint64 key = channelKey(id, type);
  if (key == -1)
    return ChatChannel();

  return channel(key);
}


/*!
 * Получение канала на основе первичного ключа в таблице \b channels.
 */
ChatChannel DataBase::channel(qint64 id)
{
  QSqlQuery query;
  query.prepare(LS("SELECT channel, gender, status, account, name, data FROM channels WHERE id = :id LIMIT 1;"));
  query.bindValue(LS(":id"), id);
  query.exec();

  if (!query.first())
    return ChatChannel();

  ChatChannel channel(new ServerChannel(query.value(0).toByteArray(), query.value(4).toString()));
  channel->setKey(id);
  channel->gender().setRaw(query.value(1).toLongLong());

  qint64 accountId = query.value(3).toLongLong();
  if (accountId > 0) {
    Account account = DataBase::account(accountId);
    channel->setAccount(&account);
  }

  channel->setData(JSON::parse(query.value(5).toByteArray()).toMap());
  FeedStorage::load(channel.data());

  return channel;
}


/*!
 * Добавление канала.
 * Если канал уже добавлен, он будет обновлён.
 *
 * \param channel Указатель на канал.
 * \return Ключ в таблице \b channels или -1 в случае ошибки.
 *
 * \sa update().
 */
qint64 DataBase::add(ChatChannel channel)
{
  qint64 key = channelKey(channel->id(), channel->type());
  if (key != -1) {
    channel->setKey(key);
    update(channel);
    return key;
  }

  QSqlQuery query;
  query.prepare(LS("INSERT INTO channels (channel, normalized, type, gender, name, data) "
                     "VALUES (:channel, :normalized, :type, :gender, :name, :data);"));

  query.bindValue(LS(":channel"),    channel->id());
  query.bindValue(LS(":normalized"), channel->normalized());
  query.bindValue(LS(":type"),       channel->type());
  query.bindValue(LS(":gender"),     channel->gender().raw());
  query.bindValue(LS(":name"),       channel->name());
  query.bindValue(LS(":data"),       JSON::generate(channel->data()));
  query.exec();

  if (query.numRowsAffected() <= 0) {
    SCHAT_LOG_ERROR("Could not add channel:" << query.lastError())
    return -1;
  }

  key = query.lastInsertId().toLongLong();
  channel->setKey(key);
  return key;
}


/*!
 * Возвращает ключ в таблице \b channels на основе идентификатора канала или нормализированного имени.
 *
 * \param id   Идентификатор по которому будет производится поиск.
 * \param type Тип канала.
 *
 * \return Ключ в таблице или -1 в случае ошибки.
 *
 * \sa Normalize.
 * \sa SimpleID.
 */
qint64 DataBase::channelKey(const QByteArray &id, int type)
{
  if (SimpleID::typeOf(id) == SimpleID::CookieId) {
    qint64 key = accountKey(id);
    if (key == -1)
      return key;

    QSqlQuery query;
    query.prepare(LS("SELECT channel FROM accounts WHERE id = :id LIMIT 1;"));
    query.bindValue(LS(":id"), key);
    query.exec();

    if (!query.first())
      return -1;

    return query.value(0).toLongLong();
  }

  if (Channel::isCompatibleId(id) == 0 && SimpleID::typeOf(id) != SimpleID::NormalizedId)
    return -1;

  QSqlQuery query;

  if (SimpleID::typeOf(id) == SimpleID::NormalizedId) {
    query.prepare(LS("SELECT id FROM channels WHERE normalized = :id AND type = :type LIMIT 1;"));
  }
  else {
    query.prepare(LS("SELECT id FROM channels WHERE channel = :id AND type = :type LIMIT 1;"));
  }

  query.bindValue(LS(":id"),   id);
  query.bindValue(LS(":type"), type);
  query.exec();

  if (!query.first())
    return -1;

  return query.value(0).toLongLong();
}


QString DataBase::nick(qint64 id)
{
  QSqlQuery query;
  query.prepare(LS("SELECT name FROM channels WHERE id = :id LIMIT 1;"));
  query.bindValue(LS(":id"), id);
  query.exec();

  if (!query.first())
    return QString();

  return query.value(0).toString();
}


void DataBase::saveData(Channel *channel)
{
  QSqlQuery query;
  query.prepare(LS("UPDATE channels SET data = :data WHERE id = :id;"));
  query.bindValue(LS(":data"),       JSON::generate(channel->data()));
  query.bindValue(LS(":id"),         channel->key());
  query.exec();
}


/*!
 * Обновление информации об канале.
 * Если канал также содержит пользовательский аккаунт, то он также будет обновлён.
 */
void DataBase::update(ChatChannel channel)
{
  if (!channel)
    return;

  QSqlQuery query;
  query.prepare(LS("UPDATE channels SET channel = :channel, normalized = :normalized, type = :type, gender = :gender, name = :name, data = :data WHERE id = :id;"));
  query.bindValue(LS(":channel"),    channel->id());
  query.bindValue(LS(":normalized"), channel->normalized());
  query.bindValue(LS(":type"),       channel->type());
  query.bindValue(LS(":gender"),     channel->gender().raw());
  query.bindValue(LS(":name"),       channel->name());
  query.bindValue(LS(":data"),       JSON::generate(channel->data()));
  query.bindValue(LS(":id"),         channel->key());
  query.exec();

  Account *account = channel->account();
  if (account && account->id > 0) {
    account->date = DateTime::utc();

    query.prepare(LS("UPDATE accounts SET date = :date, cookie = :cookie, groups = :groups WHERE id = :id;"));
    query.bindValue(LS(":date"),       account->date);
    query.bindValue(LS(":cookie"),     account->cookie);
    query.bindValue(LS(":groups"),     account->groups.toString());
    query.bindValue(LS(":id"),         account->id);
    query.exec();
  }
}


/*!
 * Получение аккаунта пользователя.
 *
 * \param key Ключ в таблице \b accounts.
 */
Account DataBase::account(qint64 key)
{
  QSqlQuery query;
  query.prepare(LS("SELECT channel, date, cookie, groups FROM accounts WHERE id = :id LIMIT 1;"));
  query.bindValue(LS(":id"), key);
  query.exec();

  if (!query.first())
    return Account();

  Account account;
  account.id = key;
  account.channel = query.value(0).toLongLong();
  account.date = query.value(1).toLongLong();
  account.cookie = query.value(2).toByteArray();
  account.groups.set(query.value(3).toString());

  return account;
}


/*!
 * Получение ключа в таблице \b accounts на основе Сookie пользователя.
 */
qint64 DataBase::accountKey(const QByteArray &cookie)
{
  if (SimpleID::typeOf(cookie) != SimpleID::CookieId)
    return -1;

  QSqlQuery query;
  query.prepare(LS("SELECT id FROM accounts WHERE cookie = :cookie LIMIT 1;"));
  query.bindValue(LS(":cookie"), cookie);
  query.exec();

  if (!query.first())
    return -1;

  return query.value(0).toLongLong();
}


/*!
 * Получение ключа в таблице \b accounts на основе номера канала.
 */
qint64 DataBase::accountKey(qint64 channel)
{
  if (channel < 1)
    return -1;

  QSqlQuery query;
  query.prepare(LS("SELECT id FROM accounts WHERE channel = :channel LIMIT 1;"));
  query.bindValue(LS(":channel"), channel);
  query.exec();

  if (!query.first())
    return -1;

  return query.value(0).toLongLong();
}


qint64 DataBase::add(Account *account)
{
  if (!account->channel)
    return -1;

  if (account->cookie.isEmpty())
    account->cookie = Ch::cookie();

  QSqlQuery query;
  query.prepare(LS("INSERT INTO accounts (channel, date, cookie, groups) "
                     "VALUES (:channel, :date, :cookie, :groups);"));

  query.bindValue(LS(":channel"),    account->channel);
  query.bindValue(LS(":date"),       account->date);
  query.bindValue(LS(":cookie"),     account->cookie);
  query.bindValue(LS(":groups"),     account->groups.toString());
  query.exec();

  if (query.numRowsAffected() <= 0) {
    SCHAT_LOG_ERROR("Could not add account:" << query.lastError())
    return -1;
  }

  qint64 key = query.lastInsertId().toLongLong();
  account->id = key;

  query.prepare(LS("UPDATE channels SET account = :account WHERE id = :id;"));
  query.bindValue(LS(":account"), key);
  query.bindValue(LS(":id"),      account->channel);
  query.exec();

  return key;
}


/*!
 * Получения списка хостов связанных с каналом.
 */
QHash<QByteArray, HostInfo> DataBase::hosts(qint64 channel)
{
  QHash<QByteArray, HostInfo> out;

  QSqlQuery query;
  query.prepare(LS("SELECT hostId, name, address, version, os, osName, tz, date, geo, data FROM hosts WHERE channel = :channel;"));
  query.bindValue(LS(":channel"), channel);
  query.exec();

  while (query.next()) {
    HostInfo host(new Host());
    host->channel = channel;
    host->hostId  = SimpleID::decode(query.value(0).toByteArray());
    host->name    = query.value(1).toString();
    host->address = query.value(2).toString();
    host->version = Ver(query.value(3).toString()).toUInt();
    host->os      = query.value(4).toInt();
    host->osName  = query.value(5).toString();
    host->tz      = query.value(6).toInt();
    host->date    = query.value(7).toLongLong();
    host->geo     = JSON::parse(query.value(8).toByteArray()).toMap();
    host->data    = JSON::parse(query.value(9).toByteArray()).toMap();

    out[host->hostId] = host;
  }

  return out;
}


/*!
 * Добавление или обновление информации о хосте.
 */
void DataBase::add(HostInfo host)
{
  AddHostTask *task = new AddHostTask(host.data());
  m_self->m_tasks.append(task);
  if (m_self->m_tasks.size() == 1)
    QTimer::singleShot(0, m_self, SLOT(startTasks()));
}


void DataBase::startTasks()
{
  while (!m_tasks.isEmpty())
    m_pool->start(m_tasks.takeFirst());
}


/*!
 * Обновление схемы базы данных до версии 2.
 *
 * - В таблице \b accounts удаляются столбцы \b name и \b password и добавляются столбцы \b provider и \b data.
 * - В таблицу \b channels добавляется столбец date.
 */
qint64 DataBase::V2()
{
  QSqlQuery query;
  query.exec(LS("BEGIN TRANSACTION;"));
  query.exec(LS("ALTER TABLE accounts RENAME TO accounts_tmp;"));
  query.exec(LS(
    "CREATE TABLE IF NOT EXISTS accounts ( "
    "  id         INTEGER PRIMARY KEY,"
    "  channel    INTEGER UNIQUE,"
    "  date       INTEGER DEFAULT ( 0 ),"
    "  cookie     BLOB    NOT NULL UNIQUE,"
    "  provider   TEXT,"
    "  flags      INTEGER DEFAULT ( 0 ),"
    "  groups     TEXT,"
    "  data       BLOB"
    ");"
  ));

  query.exec(LS("INSERT INTO accounts (channel, date, cookie, groups) SELECT channel, date, cookie, groups FROM accounts_tmp;"));
  query.exec(LS("DROP TABLE accounts_tmp;"));
  query.exec(LS("ALTER TABLE channels ADD date INTEGER DEFAULT ( 0 )"));
  query.exec(LS("PRAGMA user_version = 2"));
  query.exec(LS("COMMIT;"));

  return 2;
}


/*!
 * Добавление в базу информации о версии, в будущем эта информация может быть использована для автоматического обновления схемы базы данных.
 */
void DataBase::version()
{
  QSqlQuery query;

  query.exec(LS("PRAGMA user_version;"));
  if (!query.first())
    return;

  qint64 version = query.value(0).toLongLong();
  if (!version) {
    query.exec(LS("PRAGMA user_version = 2;"));
    version = 2;
  }

  query.finish();

  if (version == 1) version = V2();
}
