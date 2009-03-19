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

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <QByteArray>
#include <QDataStream>
#include <QObject>
#include <QQueue>

#include "asio/asio.hpp"
#include "protocol.h"

class AbstractProfile;

/*!
 * Represents a single connection from a client.
 */
class Connection
  : public QObject,
  public boost::enable_shared_from_this<Connection>,
  private boost::noncopyable
{
  Q_OBJECT

public:
  /// Состояние соединения.
  enum State {
    WaitGreeting, ///< Ожидание рукопожатия.
    WaitAccess,   ///< Ожидание подтверждения авторизации.
    Ready,        ///< Соединение активно.
    WaitClose     ///< Соединение закрывается.
  };

  explicit Connection(asio::io_service &ioService, QObject *parent = 0);
  ~Connection();

  asio::ip::tcp::socket& socket();
  bool send(const QByteArray &data);
  void start();

private:
  bool opcodeGreeting();
  quint16 verifyGreeting(quint16 version);
  void checkGreeting();
  void close();
  void handleReadBody(const asio::error_code &e, int bytes);
  void handleReadHeader(const asio::error_code &e, int bytes);
  void handleWrite(const asio::error_code &e, int bytes);
  void send();

  AbstractProfile *m_profile;       ///< Профиль подключённого пользователя.
  asio::deadline_timer m_timer;     ///< Таймер, для разрыва соединения если за заданное время не инициировано рукопожатие.
  asio::ip::tcp::socket m_socket;   ///< Socket for the connection.
  char m_body[8192];                ///< Буфер для чтения тела пакета.
  char m_header[schat::headerSize]; ///< Буфер для заголовка пакета.
  char m_send[8192];                ///< Буфер отправки пакета.
  QQueue<QByteArray> m_sendQueue;   ///< Очередь пакетов для отправки.
  quint16 m_bodySize;               ///< Размер тела пакета.
  quint16 m_opcode;                 ///< Опкод пакета.
  quint8 m_flag;                    ///< Флаг.
  quint8 m_numeric;                 ///< Уникальный номер удалённого сервера, при использовании подключения в качестве линка.
  State m_state;                    ///< Статус соединения.
};

typedef boost::shared_ptr<Connection> ConnectionPtr;

#endif /* CONNECTION_H_ */
