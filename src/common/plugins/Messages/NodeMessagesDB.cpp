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

#include <QDebug>

#include <QSqlDatabase>
#include <QSqlQuery>

#include "cores/Core.h"
#include "FileLocations.h"
#include "net/packets/MessageNotice.h"
#include "NodeMessagesDB.h"
#include "sglobal.h"
#include "Storage.h"
#include "text/PlainTextFilter.h"

bool NodeMessagesDB::m_isOpen = true;
QString NodeMessagesDB::m_id = LS("messages");

NodeMessagesDB::NodeMessagesDB()
{
}


bool NodeMessagesDB::open()
{
  QSqlDatabase db = QSqlDatabase::addDatabase(LS("QSQLITE"), m_id);
  db.setDatabaseName(Storage::locations()->path(FileLocations::VarPath) + LS("/messages.sqlite"));
  if (!db.open())
    return false;

  QSqlQuery query(db);
  query.exec(LS("PRAGMA synchronous = OFF"));

  query.exec(LS(
    "CREATE TABLE IF NOT EXISTS messages ( "
    "  id         INTEGER PRIMARY KEY,"
    "  messageId  BLOB,"
    "  senderId   BLOB,"
    "  destId     BLOB,"
    "  status     INTEGER DEFAULT ( 200 ),"
    "  date       INTEGER,"
    "  command    TEXT,"
    "  text       TEXT,"
    "  plain      TEXT"
    ");"));

  m_isOpen = true;
  return true;
}


QList<MessageId> NodeMessagesDB::ids(QSqlQuery &query)
{
  if (!query.isActive())
    return QList<MessageId>();

  QList<MessageId> out;
  while (query.next())
    out.prepend(MessageId(query.value(2).toLongLong(), query.value(1).toByteArray()));

  return out;
}


/*!
 * Получение идентификаторов последних сообщений для обычного канала или собственного канала пользователя.
 *
 * \param channel Идентификатор канала.
 * \param limit   Максимальное количество сообщений.
 */
QList<MessageId> NodeMessagesDB::last(const QByteArray &channel, int limit)
{
  QSqlQuery query(QSqlDatabase::database(m_id));

  int type = SimpleID::typeOf(channel);
  if (type == SimpleID::ChannelId) {
    query.prepare(LS("SELECT id, messageId, date FROM messages WHERE destId = :destId ORDER BY id DESC LIMIT :limit;"));
  }
  else if (type == SimpleID::UserId) {
    query.prepare(LS("SELECT id, messageId, date FROM messages WHERE senderId = :senderId AND destId = :destId ORDER BY id DESC LIMIT :limit;"));
    query.bindValue(LS(":senderId"), channel);
  }
  else
    return QList<MessageId>();

  query.bindValue(LS(":destId"), channel);
  query.bindValue(LS(":limit"), limit);
  query.exec();

  return ids(query);
}


/*!
 * Получение идентификаторов последних приватных сообщений между двумя пользователями.
 *
 * \param user1 Идентификатор одного пользователя.
 * \param user2 Идентификатор другого пользователя.
 * \param limit Максимальное количество сообщений.
 */
QList<MessageId> NodeMessagesDB::last(const QByteArray &user1, const QByteArray &user2, int limit)
{
  QSqlQuery query(QSqlDatabase::database(m_id));
  query.prepare(LS("SELECT id, messageId, date FROM messages WHERE (senderId = :id1 AND destId = :id2) OR (senderId = :id3 AND destId = :id4) ORDER BY id DESC LIMIT :limit;"));
  query.bindValue(LS(":id1"), user1);
  query.bindValue(LS(":id2"), user2);
  query.bindValue(LS(":id3"), user2);
  query.bindValue(LS(":id4"), user1);
  query.bindValue(LS(":limit"), limit);
  query.exec();

  return ids(query);
}


QVariantList NodeMessagesDB::lastDeprecated(const QByteArray &channel, int limit)
{
  QSqlQuery query(QSqlDatabase::database(m_id));

  int type = SimpleID::typeOf(channel);
  if (type == SimpleID::ChannelId) {
    query.prepare(LS("SELECT id, messageId, senderId, destId, status, date, command, text FROM messages WHERE destId = :destId ORDER BY id DESC LIMIT :limit;"));
  }
  else if (type == SimpleID::UserId) {
    query.prepare(LS("SELECT id, messageId, senderId, destId, status, date, command, text FROM messages WHERE senderId = :senderId AND destId = :destId ORDER BY id DESC LIMIT :limit;"));
    query.bindValue(LS(":senderId"), channel);
  }
  else
    return QVariantList();

  query.bindValue(LS(":destId"), channel);
  query.bindValue(LS(":limit"), limit);
  query.exec();

  return messages(query);
}


QVariantList NodeMessagesDB::lastDeprecated(const QByteArray &user1, const QByteArray &user2, int limit)
{
  QSqlQuery query(QSqlDatabase::database(m_id));
  query.prepare(LS("SELECT id, messageId, senderId, destId, status, date, command, text FROM messages WHERE (senderId = :id1 AND destId = :id2) OR (senderId = :id3 AND destId = :id4) ORDER BY id DESC LIMIT :limit;"));
  query.bindValue(LS(":id1"), user1);
  query.bindValue(LS(":id2"), user2);
  query.bindValue(LS(":id3"), user2);
  query.bindValue(LS(":id4"), user1);
  query.bindValue(LS(":limit"), limit);
  query.exec();

  return messages(query);
}


QVariantList NodeMessagesDB::messages(QSqlQuery &query)
{
  if (!query.isActive())
    return QVariantList();

  QVariantList out;

  while (query.next()) {
    QVariantList data;
    data.append(query.value(1)); // 0 messageId
    data.append(query.value(2)); // 1 senderId
    data.append(query.value(3)); // 2 destId
    data.append(query.value(4)); // 3 status
    data.append(query.value(5)); // 4 date
    data.append(query.value(6)); // 5 command
    data.append(query.value(7)); // 6 text
    data.append(query.value(0)); // 7 id
    out.push_front(data);
  }

  return out;
}


QVariantList NodeMessagesDB::offline(const QByteArray &user)
{
  QSqlQuery query(QSqlDatabase::database(m_id));
  query.prepare(LS("SELECT id, messageId, senderId, destId, status, date, command, text FROM messages WHERE destId = :destId AND status = :status ORDER BY id DESC;"));
  query.bindValue(LS(":destId"), user);
  query.bindValue(LS(":status"), Notice::ChannelOffline);
  query.exec();

  return messages(query);
}


void NodeMessagesDB::add(const MessageNotice &packet, int status)
{
  QSqlQuery query(QSqlDatabase::database(m_id));
  query.prepare(LS("INSERT INTO messages (messageId, senderId, destId, status, date, command, text, plain) "
                     "VALUES (:messageId, :senderId, :destId, :status, :date, :command, :text, :plain);"));

  query.bindValue(LS(":messageId"), packet.id());
  query.bindValue(LS(":senderId"),  packet.sender());
  query.bindValue(LS(":destId"),    packet.dest());
  query.bindValue(LS(":status"),    status);
  query.bindValue(LS(":date"),      Core::date());
  query.bindValue(LS(":command"),   packet.command());
  query.bindValue(LS(":text"),      packet.text());
  query.bindValue(LS(":plain"),     PlainTextFilter::filter(packet.text()));
  query.exec();
}


void NodeMessagesDB::markAsRead(const QVariantList &data)
{
  if (data.isEmpty())
    return;

  QSqlDatabase db = QSqlDatabase::database(m_id);
  QSqlQuery query(db);
  db.transaction();
  query.prepare(LS("UPDATE messages SET status = 200 WHERE id = :id;"));

  for (int i = 0; i < data.size(); ++i) {
    QVariantList msg = data.at(i).toList();
    if (msg.isEmpty())
      continue;

    query.bindValue(LS(":id"), msg.value(7));
    query.exec();
  }

  db.commit();
}