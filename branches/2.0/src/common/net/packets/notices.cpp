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

#include "net/PacketReader.h"
#include "net/packets/notices.h"
#include "net/packets/message.h"

MessageNotice::MessageNotice(quint16 type, PacketReader *reader)
  : AbstractNotice(type, reader)
  , m_valid(false)
  , m_status(0)
  , m_error(0)
{
  m_status = reader->get<quint8>();
  m_error = reader->get<quint8>();
  m_id = reader->id();

  if (SimpleID::typeOf(m_id) != SimpleID::MessageId)
    return;

  m_valid = true;
}


MessageNotice::MessageNotice(quint8 status, const QByteArray &sender, const QByteArray &dest, const QByteArray &id, quint8 error)
  : AbstractNotice(MessageNoticeType, sender, dest)
  , m_valid(false)
  , m_status(status)
  , m_error(error)
  , m_id(id)
{
  if (SimpleID::typeOf(id) != SimpleID::MessageId)
    return;

  m_valid = true;
}


QByteArray MessageNotice::data(QDataStream *stream) const
{
  PacketWriter writer(stream, Protocol::NoticePacket, m_sender, m_dest);
  writer.put(m_type);
  writer.put(m_fields);
  writer.put(m_status);
  writer.put(m_error);
  writer.putId(m_id);

  return writer.data();
}


TextNotice::TextNotice(quint16 type, PacketReader *reader)
  : AbstractNotice(type, reader)
  , m_subtype(0)
{
  m_subtype = reader->get<quint16>();
  m_text = reader->text();
}


TextNotice::TextNotice(quint16 sybtype, const QByteArray &sender, const QByteArray &dest, const QString &text)
  : AbstractNotice(TextNoticeType, sender, dest)
  , m_subtype(sybtype)
  , m_text(text)
{
}


QByteArray TextNotice::data(QDataStream *stream) const
{
  PacketWriter writer(stream, Protocol::NoticePacket, m_sender, m_dest);
  writer.put(m_type);
  writer.put(m_fields);
  writer.put(m_subtype);
  writer.put(m_text);

  return writer.data();
}
