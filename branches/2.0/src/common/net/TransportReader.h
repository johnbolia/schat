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

#ifndef TRANSPORTREADER_H_
#define TRANSPORTREADER_H_

#include <QDataStream>

/*!
 * Класс, выполняющий чтение транспортного пакета.
 */
class TransportReader
{
public:
  TransportReader(quint32 size, QDataStream *stream);
  inline int options() const { return m_options; }
  inline quint32 available() const { return m_available; }
  inline quint64 sequence() const { return m_sequence; }
  int readHeader();
  QByteArray readOne();
  QList<QByteArray> read();
  void skipAll();

private:
  quint32 createMap();

  QDataStream *m_stream; ///< input stream.
  QList<quint32> m_sizes;    ///< список размеров виртуальных пакетов.
  quint32 m_available;   ///< число не прочитанных байт.
  quint32 m_size;        ///< Size of packet.
  quint64 m_sequence;    ///< счётчик пакетов.
  quint8 m_options;      ///< packet options.
  quint8 m_subversion;   ///< packet subversion.
  quint8 m_type;         ///< packet type.
  quint8 m_version;      ///< packet version.
};

#endif /* TRANSPORTREADER_H_ */
