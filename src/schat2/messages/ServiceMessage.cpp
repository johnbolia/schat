/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2013 IMPOMEZIA <schat@impomezia.com>
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

#include <QTextDocument>

#include "ChatCore.h"
#include "client/ChatClient.h"
#include "client/ClientChannels.h"
#include "DateTime.h"
#include "messages/ServiceMessage.h"
#include "net/SimpleID.h"
#include "NetworkManager.h"
#include "sglobal.h"

ServiceMessage::ServiceMessage(const QString &text, const QByteArray &user)
  : Message(ChatCore::randomId())
{
  m_data[MESSAGE_TYPE] = LS("service");
  m_data[MESSAGE_TEXT] = text;
  m_data[MESSAGE_DATE] = ChatClient::date();
  m_data[MESSAGE_FUNC] = LS("addServiceMessage");

  setAuthor(user);
}


bool ServiceMessage::isValid() const
{
  return !m_data.value(MESSAGE_TEXT).toString().isEmpty();
}


/*!
 * Сообщение о входе пользователя в канал.
 */
ServiceMessage ServiceMessage::joined(const QByteArray &user)
{
  QString text;
  ClientChannel channel = ChatClient::channels()->get(user);

  if (channel) {
    if (channel->gender().value() == Gender::Female)
      text = tr("has joined", "Female");
    else
      text = tr("has joined", "Male");
  }

  return ServiceMessage(text, user);
}


/*!
 * Сообщение о выходе пользователя из канала.
 */
ServiceMessage ServiceMessage::part(const QByteArray &user)
{
  QString text;
  ClientChannel channel = ChatClient::channels()->get(user);

  if (channel) {
    if (channel->gender().value() == Gender::Female)
      text = tr("has left", "Female");
    else
      text = tr("has left", "Male");
  }

  return ServiceMessage(text, user);
}


/*!
 * Сообщение о выходе пользователя из чата.
 */
ServiceMessage ServiceMessage::quit(const QByteArray &user)
{
  QString text;
  ClientChannel channel = ChatClient::channels()->get(user);

  if (channel) {
    if (channel->gender().value() == Gender::Female)
      text = tr("has quit chat", "Female");
    else
      text = tr("has quit chat", "Male");
  }

  return ServiceMessage(text, user);
}


ServiceMessage ServiceMessage::showId(const QByteArray &id)
{
  ServiceMessage message(LS("<b>") + SimpleID::encode(id) + LS("</b>"), id);
  message.data()[MESSAGE_TYPE]  = LS("info");
  message.data()[MESSAGE_DATE]  = 0;
  return message;
}
