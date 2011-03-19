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

#ifndef PACKETWRITER_H_
#define PACKETWRITER_H_

#include <QByteArray>
#include <QCryptographicHash>
#include <QDataStream>

#include "net/Protocol.h"

/*!
 * Класс выполняющий запись виртуального пакета.
 */
class PacketWriter
{
public:
  /*!
   * Создание виртуального пакета и запись заголовка.
   *
   * \param stream   output stream.
   * \param type     Тип пакета.
   */
  inline PacketWriter(QDataStream *stream, quint16 type)
    : m_stream(stream)
  {
    m_device = stream->device();
    m_device->seek(0);

    *stream << type << quint8(0) << quint8(0) << quint8(Protocol::BasicHeader);
  }

  /*!
   * Создание виртуального пакета и запись заголовка.
   *
   * \param stream   output stream.
   * \param type     Тип пакета.
   * \param sender   Идентификатор отправителя.
   * \param dest     Идентификатор получателя.
   */
  inline PacketWriter(QDataStream *stream, quint16 type, const QByteArray &sender, const QByteArray &dest = QByteArray())
    : m_stream(stream)
  {
    m_device = stream->device();
    m_device->seek(0);

    quint8 headerOption = Protocol::BasicHeader;
    if (!sender.isEmpty())
      headerOption |= Protocol::SenderField;

    if (!dest.isEmpty())
      headerOption |= Protocol::DestinationField;

    *stream << type << quint8(0) << quint8(0) << headerOption;

    if (headerOption & Protocol::SenderField)
      putId(sender);

    if (headerOption & Protocol::DestinationField)
      putId(dest);
  }

  /*!
   * Возвращает результат работы класса, тело пакета.
   */
  inline QByteArray data() const
  {
    int size = m_device->pos();
    m_device->seek(0);
    return m_device->peek(size);
  }

  /// Запись UTF-8 строки.
  inline void put(const QString &text) {
    *m_stream << text.toUtf8();
  }

  template<class T>
  inline void put(const T &data) {
    *m_stream << data;
  }

  /// Запись Id.
  inline void putId(const QByteArray &data)
  {
    if (data.size() == Protocol::IdSize) {
      m_device->write(data);
    }
    else {
      m_device->write(QCryptographicHash::hash("", QCryptographicHash::Sha1) += Protocol::InvalidId);
    }
  }

  /// Запись Id.
  inline void putId(const QByteArray &data, quint8 idType)
  {
    QByteArray id = data;
    putId(id += idType);
  }

  /// Запись Id.
  inline void putId(const QList<QByteArray> &data) {
    if (data.isEmpty())
      return;

    put<quint32>(data.size());
    for (int i = 0; i < data.size(); ++i) {
      putId(data.at(i));
    }
  }

private:
  QDataStream *m_stream; ///< output stream.
  QIODevice *m_device;   ///< output device.
};

#endif /* PACKETWRITER_H_ */
