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

#include "client/ChatClient.h"
#include "client/ClientChannels.h"
#include "client/SimpleClient.h"
#include "net/packets/channels.h"
#include "net/SimpleID.h"

ClientChannels::ClientChannels(QObject *parent)
  : QObject(parent)
  , m_client(ChatClient::io())
{
  connect(m_client, SIGNAL(notice(int)), SLOT(notice(int)));
}


/*!
 * Получение канал по идентификатору.
 *
 * \param id Идентификатор канала.
 * \return Канал или пустой канал, если поиск неудачен.
 */
ClientChannel ClientChannels::get(const QByteArray &id)
{
  return m_channels.value(id);
}


/*!
 * Подключение к обычному каналу по имени.
 *
 * \param name Имя канала.
 */
void ClientChannels::join(const QString &name)
{
  if (!Channel::isValidName(name))
    return;

  m_client->send(ChannelPacket::join(ChatClient::id(), QByteArray(), name, m_client->sendStream()));
}


/*!
 * Обработка получения нового уведомления.
 *
 * \param type Тип уведомления, должен быть равен Notice::ChannelType.
 */
void ClientChannels::notice(int type)
{
  if (type != Notice::ChannelType)
    return;

  ChannelPacket packet(type, ChatClient::io()->reader());
  if (!packet.isValid())
    return;

  m_packet = &packet;
  QString cmd = m_packet->command();

  if (cmd == "channel")
    channel();

  emit notice(packet);
}


/*!
 * Добавление канала в таблицу каналов.
 */
ClientChannel ClientChannels::add()
{
  QByteArray id = m_packet->channelId();
  if (!Channel::isCompatibleId(id))
    return ClientChannel();

  ClientChannel channel = m_channels.value(id);
  if (!channel) {
    channel = ClientChannel(new Channel(id, m_packet->text()));
    m_channels[id] = channel;
  }

  channel->setName(m_packet->text());
  channel->gender() = m_packet->gender();
  channel->status() = m_packet->status();

  return channel;
}


/*!
 * Чтение заголовка канала.
 */
void ClientChannels::channel()
{
  ClientChannel channel = add();
  if (!channel)
    return;

  if (channel->type() == SimpleID::ChannelId) {
    channel->channels() = m_packet->channels();
  }

  qDebug() << "CHANNELS SIZE:" << channel->channels().all().size();

  emit this->channel(channel->id());
}
