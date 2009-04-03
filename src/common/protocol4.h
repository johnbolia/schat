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

#ifndef PROTOCOL4_H_
#define PROTOCOL4_H_

#include <QByteArray>
#include <QDataStream>

/*!
 * \brief Содержит общие определения \b Simple \b Chat протокола.
 */
namespace protocol {

static const int StreamVersion = QDataStream::Qt_4_4;

/// Версия протокола, размер блока 2 байта (quint8 + quint8).
enum Version {
  MajorVersion = 4, ///< Старшая версия протокола.
  MinorVersion = 0  ///< Младшая версия (модификация) протокола.
};


/// Коды пакетов, размер кода 2 байта (quint16).
enum PacketOpcode {
  Greeting = 7666 ///< Пакет инициирующий рукопожатие. \sa protocol::packet::Greeting.
};


/// Пол или иная информация, определяющая тип пользователя.
enum Gender {
  Male,   ///< Пользователь мужского пола.
  Female, ///< Пользователь женского пола.
  Bot     ///< Пользователь является ботом.
};


/*!
 * \brief Определения пакетов \b Simple \b Chat протокола.
 */
namespace packet {


/*!
 * Размеры структур данных.
 */
enum Sizes {
  HeaderSize = 4,   ///< Размер заголовка пакета (размер + опкод).
  MaxSize    = 8192 ///< Максимальный размер пакета.
};


/*!
 * \brief Абстрактный базовый пакет, размер 4 байта.
 */
class AbstractPacket {
public:
  /// Базовый конструктор.
  AbstractPacket()
  : opcode(0) {}

  /// Конструктор инициализирующий опкод пакета.
  AbstractPacket(quint16 opcode)
  : opcode(opcode) {}

  /// Деструктор
  virtual ~AbstractPacket() {}

  /// Отправка в поток тела пакета.
  virtual void toStream(QDataStream &stream) const { Q_UNUSED(stream) }

  quint16 opcode; ///< Опкод пакета.
};


/*!
 * \brief Пакет инициирующий рукопожатие.
 *
 * Этот пакет должен быть отправлен клиентом сразу после
 * установки соединения.
 */
class Greeting : public AbstractPacket {
public:

  /// Конструктор для подготовки пакета к отправке.
  Greeting(quint32 features,
      quint8 gender,
      const QByteArray &uniqueId,
      const QByteArray &userName,
      const QByteArray &password,
      const QString &nick,
      const QString &fullName)
  : AbstractPacket(protocol::Greeting),
  majorVersion(protocol::MajorVersion),
  minorVersion(protocol::MinorVersion),
  features(features),
  gender(gender),
  uniqueId(uniqueId),
  userName(userName),
  password(password),
  nick(nick),
  fullName(fullName)
  {}

  void toStream(QDataStream &stream) const
  {
    stream << majorVersion << minorVersion << features << gender
           << uniqueId << userName << password << nick << fullName;
  }

  quint8 majorVersion; ///< Размер 1 байт. \sa protocol::Version.
  quint8 minorVersion; ///< Размер 1 байт. \sa protocol::Version.
  quint32 features;    ///< Размер 4 байта.
  quint8 gender;       ///< Размер 1 байт. \sa protocol::Gender.
  QByteArray uniqueId; ///< Размер от 4 до 24 байт. \sa SimpleClient::Private::uniqueId.
  QByteArray userName; ///< Минимальный размер 4 байта максимальный пока не определён \sa SimpleClient::Private::userName.
  QByteArray password; ///< Размер от 4 до 24 байт. \sa SimpleClient::Private::password.
  QString nick;        ///< Размер от 4 до 52 байт. \sa SimpleClient::Private::nick.
  QString fullName;    ///< Размер от 4 до 68 байт. \sa SimpleClient::Private::fullName.
};
}

class PacketTool {
public:
  /*!
   * Универсальный конструктор для формирования пакета для отправки.
   *
   * \param packet Абстрактный базовый пакет или его наследник.
   */
  static QByteArray create(const protocol::packet::AbstractPacket &packet)
  {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(protocol::StreamVersion);
    out << quint16(0) << packet.opcode;
    packet.toStream(out);
    out.device()->seek(0);
    out << quint16(block.size() - 4);
    return block;
  }
};
}

#ifdef SCHAT_DEBUG
  #define DEBUG_OUT(x) qDebug() << x;
#else
  #define DEBUG_OUT(x)
#endif

#endif /* PROTOCOL4_H_ */
