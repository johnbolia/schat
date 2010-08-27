/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2010 IMPOMEZIA <schat@impomezia.com>
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

#ifndef PACKET_H_
#define PACKET_H_

#include <QList>
#include <QPair>

class QByteArray;
class QDataStream;

namespace Packet {

/// Типы данных.
enum DataTypes {
  UINT16, ///< quint16
  UINT8,  ///< quint8
  UTF16,  ///< UTF-16 string
  UTF8,   ///< UTF-8 string
};

/// Коды пакетов.
enum PacketCodes {
  HandshakeRequest = 100, ///< v3 name: OpcodeGreeting
};
}


/*!
 * \brief Низкоуровневый класс осуществляющий сборку пакета.
 */
class PacketBuilder
{
public:
  PacketBuilder();
  ~PacketBuilder();
  QByteArray data();
  void add(Packet::DataTypes type, const QString &data);
  void add(Packet::DataTypes type, int data);
  void addPacket(int pcode);

private:
  QByteArray *m_data;               ///< Тело пакета.
  QDataStream *m_stream;            ///< Поток для записи.
  QList<QPair<int, int> > m_pcodes; ///< Список кодов пакетов со смещением относительно начала тела пакета.
};




/*!
 * \brief Низкоуровневый класс осуществляющий чтение пакета.
 */
class PacketReader
{
public:
  PacketReader();
  ~PacketReader();

};

#endif /* PACKET_H_ */
