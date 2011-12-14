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
#include "client/ClientCmd.h"
#include "client/ClientHooks.h"
#include "client/ClientMessages.h"
#include "client/SimpleClient.h"
#include "DateTime.h"
#include "net/SimpleID.h"
#include "text/PlainTextFilter.h"

ClientMessages::ClientMessages(QObject *parent)
  : QObject(parent)
  , m_hooks(0)
  , m_client(ChatClient::io())
{
  m_hooks = new Hooks::Messages(this);

  connect(m_client, SIGNAL(notice(int)), SLOT(notice(int)));
  connect(ChatClient::channels(), SIGNAL(channels(const QList<QByteArray> &)), SLOT(channels(const QList<QByteArray> &)));
}


QByteArray ClientMessages::randomId() const
{
  return SimpleID::randomId(SimpleID::MessageId, m_client->channelId());
}


/*!
 * Отправка текстового сообщения, если в тексте будут команды, то они будут обработаны.
 */
bool ClientMessages::send(const QByteArray &dest, const QString &text)
{
  if (text.isEmpty())
    return false;

  m_destId = dest;
  QString plain = PlainTextFilter::filter(text);

  if (!plain.startsWith('/'))
    return sendText(dest, text);

  if (m_hooks->command(dest, text, plain))
    return true;

  /// Для обработки обычных команд используется хук: Hooks::Messages::command(const QByteArray &dest, const ClientCmd &cmd).
  QStringList commands = (" " + plain).split(" /", QString::SkipEmptyParts);
  for (int i = 0; i < commands.size(); ++i) {
    ClientCmd cmd(commands.at(i));
    if (cmd.isValid())
      m_hooks->command(dest, cmd);
  }

  return true;
}


/*!
 * Отправка текстового сообщения, команды не обрабатываются.
 */
bool ClientMessages::sendText(const QByteArray &dest, const QString &text, const QString &command)
{
  MessagePacket packet(ChatClient::id(), dest, text, DateTime::utc(), randomId());
  if (!command.isEmpty())
    packet.setCommand(command);

  if (m_client->send(packet, true)) {
    m_hooks->sendText(packet);

    return true;
  }

  return false;
}


void ClientMessages::channels(const QList<QByteArray> &channels)
{
  foreach (QByteArray id, channels) {
    if (m_pending.contains(id)) {
      QList<MessagePacket> packets = m_pending.value(id);

      for (int i = 0; i < packets.size(); ++i)
        m_hooks->readText(packets.at(i));

      m_pending.remove(id);
    }
  }
}


/*!
 * Чтение входящего сообщения.
 */
void ClientMessages::notice(int type)
{
  if (type != Notice::MessageType)
    return;

  MessagePacket packet(type, ChatClient::io()->reader());
  if (!packet.isValid())
    return;

  m_packet = &packet;
  m_packet->setDate(ChatClient::io()->date());

  /// В случае если отправитель сообщения неизвестен клиенту, то будет произведён вход в канал
  /// этого пользователя для получения информации о нём, само сообщения будет добавлено в очередь
  /// до момента получения информации об отправителе.
  ClientChannel user = ChatClient::channels()->get(m_packet->sender());
  if (!user || !user->isSynced()) {
    ChatClient::channels()->join(m_packet->sender());
    m_pending[m_packet->sender()].append(packet);
    return;
  }

  m_hooks->readText(packet);
}
