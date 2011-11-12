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

#include <QDebug>

#include "Channel.h"
#include "net/PacketReader.h"
#include "net/packets/channels.h"

ChannelPacket::ChannelPacket(ClientChannel channel, const QByteArray &dest, const QString &command, quint64 time)
  : Notice(channel->id(), dest, command, time)
  , m_channelId(channel->id())
  , m_users(channel->users())
{
  m_type = ChannelType;
  setText(channel->name());
}


ChannelPacket::ChannelPacket(quint16 type, PacketReader *reader)
  : Notice(type, reader)
{
  if (SimpleID::typeOf(reader->sender()) == SimpleID::ChannelId)
    m_channelId = reader->sender();
  else if (SimpleID::typeOf(reader->dest()) == SimpleID::ChannelId)
    m_channelId = reader->dest();

  m_users = reader->idList();
}


QVariantMap ChannelPacket::feeds(ClientChannel channel, ClientUser user)
{
  QVariantMap map;
//  int acl;
//  QHashIterator<QString, FeedPtr> i(channel->feeds());
//
//  while (i.hasNext()) {
//    i.next();
//    acl = i.value()->acl().match(user);
//    if (acl) {
//      QVariantMap header;
//      header["time"] = QString::number(i.value()->time());
//      header["acl"] = acl;
//      map[i.key()] = header;
//    }
//  }

  return map;
}


void ChannelPacket::write(PacketWriter *writer) const
{
  writer->put(m_users);
}
