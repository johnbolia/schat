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

#ifndef NOTICES_H_
#define NOTICES_H_

#include "net/PacketWriter.h"
#include "schat.h"

class PacketReader;

class SCHAT_EXPORT NoticeData
{
public:
  /// Тип уведомления.
  enum Type {
    Invalid,          ///< Некорректный тип.
    MessageDelivered, ///< Сообщение было доставлено.
    MessageRejected,  ///< Сообщение было отклонено.
    SlaveNodeXHost    ///< Оригинальный адрес пользователя.
  };

  enum Param1 {
    UnknownError,    ///< Неизвестная ошибка.
    UserUnavailable, ///< Пользователь недоступен.
    Ignored,         ///< Сообщение было игнорированно.
  };

  NoticeData()
  : timestamp(0)
  , type(0)
  , messageName(0)
  , param1(0)
  , param2(0)
  {}

  NoticeData(quint16 type, const QByteArray &senderId, const QByteArray &destId, const QString &text);
  NoticeData(const QByteArray &senderId, const QByteArray &destId, quint16 type, quint64 messageName, quint8 param1 = 0);

  QByteArray destId;   ///< Идентификатор назначения.
  QByteArray senderId; ///< Идентификатор отправителя.
  qint64 timestamp;    ///< Отметка времени.
  quint16 type;        ///< Тип.
  quint64 messageName; ///< Имя-счётчик сообщения, только для типов MessageDelivered и MessageRejected.
  quint8 param1;       ///< Не обязательный параметр №1
  quint8 param2;       ///< Не обязательный параметр №2
  QString text;        ///< Текстовое содержимое.
};


/*!
 * Формирует пакет Protocol::NoticePacket.
 */
class SCHAT_EXPORT NoticeWriter : public PacketWriter
{
public:
  NoticeWriter(QDataStream *stream, const NoticeData &data);
};


/*!
 * Читает пакет Protocol::NoticePacket.
 */
class SCHAT_EXPORT NoticeReader
{
public:
  NoticeReader(PacketReader *reader);

  NoticeData data;
};

#endif /* NOTICES_H_ */
