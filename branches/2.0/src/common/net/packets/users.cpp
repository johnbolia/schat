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

#include "net/packets/users.h"
#include "net/PacketReader.h"
#include "User.h"


/*!
 * Специальный конструктор, формирующий широковещательный пакет.
 *
 * \param stream  ///< Поток записи.
 * \param user    ///< Пользователь.
 */
UserWriter::UserWriter(QDataStream *stream, User *user)
  : PacketWriter(stream, Protocol::UserDataPacket, user->id(), user->channels(), true)
{
  write(user, 0);
}


/*!
 * Основной конструктор.
 *
 * \param stream    ///< Поток записи.
 * \param user      ///< Пользователь.
 * \param destId    ///< Идентификатор назначения.
 * \param channelId ///< Идентификатор канала.
 * \param options   ///< Опции.
 */
UserWriter::UserWriter(QDataStream *stream, User *user, const QByteArray &destId, const QByteArray &channelId, int options)
  : PacketWriter(stream, Protocol::UserDataPacket, user->id(), destId, channelId)
{
  write(user, options);
}


/*!
 * Основной конструктор.
 *
 * \param stream  ///< Поток записи.
 * \param user    ///< Пользователь.
 * \param destId  ///< Идентификатор назначения.
 * \param options ///< Опции.
 */
UserWriter::UserWriter(QDataStream *stream, User *user, const QByteArray &destId, int options)
  : PacketWriter(stream, Protocol::UserDataPacket, user->id(), destId)
{
  write(user, options);
}


void UserWriter::write(User *user, int options)
{
  put<quint8>(options);
  put<quint8>(0);
  put<quint8>(user->rawGender());
  put<quint8>(user->status());
  put(user->nick());

  if (options & StaticData) {
    put(user->userAgent());
    put(user->host());
    put(user->serverNumber());
    put(user->groups().join(QLatin1String(",")));
  }
}


UserReader::UserReader(PacketReader *reader)
{
  options = reader->get<quint8>();
  reader->get<quint8>(); // reserved.

  user.setId(reader->sender());
  user.setRawGender(reader->get<quint8>());
  user.setStatus(reader->get<quint8>());
  user.setNick(reader->text());

  if (options & UserWriter::StaticData) {
    user.setUserAgent(reader->text());
    user.setHost(reader->text());
    user.setServerNumber(reader->get<quint8>());
    user.setGroups(reader->text());
  }
  else if (user.status() == User::OfflineStatus)
    user.setStatus(User::OnlineStatus);
}
