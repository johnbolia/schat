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

#include <QtCore>
#include <QtTest>

//#define SCHAT_DEBUG

#include "protocol4.h"
#include "simpleclienthandler.h"

/*!
 * Конструктор класса SimpleClientHandler.
 */
SimpleClientHandler::SimpleClientHandler(boost::shared_ptr<Connection> connection)
  : AbstractProtocolHandler(connection, AbstractProtocolHandler::SimpleClientServer)
{
}


SimpleClientHandler::~SimpleClientHandler()
{
}


/*!
 * Обработка нового пакета.
 */
void SimpleClientHandler::append(quint16 opcode, const QByteArray &data)
{
  DEBUG_OUT(" | OPCODE:   | " << opcode)
  DEBUG_OUT(" | SIZE:     | " << data.size() << "bytes")
  DEBUG_OUT(" | RAW BODY: | " << data.toHex())

  using namespace protocol;

  if (opcode == protocol::Greeting) {
    int err = greeting(data);
    if (err > 0) {
      m_connection->send(PacketTool::create(packet::GreetingReply(err)));
//      m_connection->close();
    }
    else if (err == -1)
      m_connection->close();
  }
}


/*!
 * Обработка приветствия.
 */
int SimpleClientHandler::greeting(const QByteArray &data)
{
  DEBUG_OUT("SimpleClientHandler::greeting()" << this);

  protocol::packet::Greeting packet(data);
  if (packet.error)
    return -1;

//  UserData out;
  return protocol::ErrorBadNickName;
//  out.nick = UserTools::nick(out.nick);
//  if (!UserTools::isValidNick(out.nick))
//    return ErrorBadNickName;

  return 0;
}
