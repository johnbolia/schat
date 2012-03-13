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

#ifndef ABSTRACTMESSAGE_H_
#define ABSTRACTMESSAGE_H_

#include <QDateTime>
#include <QHash>
#include <QVariant>

#include "schat.h"

class MessageData;

class SCHAT_CORE_EXPORT AbstractMessage
{
public:
  /// Тип сообщения.
  enum MessageType {
    UnknownType,      ///< Неизвестный тип.
    UserMessageType,  ///< Сообщение от пользователя.
    AlertMessageType, ///< Уведомления.
    ServiceMessageType
  };

  /// Направление сообщения.
  enum Direction {
    UnknownDirection,
    OutgoingDirection,
    IncomingDirection
  };

  ///< Приоритет сообщения.
  enum Priority {
    IdlePriority = 4,
    LowPriority = 6,
    NormalPriority = 8,
    HighPriority = 10,
    HighestPriority = 13
  };

  AbstractMessage(const QString &type, const MessageData &data, int parseOptions = 0);
  AbstractMessage(const QString &type, const QString &text = QString(), const QByteArray &destId = QByteArray(), int parseOptions = 0);
  inline int direction() const { return m_direction; }
  inline int priority() const { return m_priority; }
  inline QByteArray destId() const { return m_destId; }
  inline QByteArray senderId() const { return m_senderId; }
  inline qint64 timestamp() const { return m_timestamp; }
  inline QString id() const { return m_id; }
  inline QString text() const { return m_text; }
  inline QString type() const { return m_type; }
  inline void setPriority(int priority) { m_priority = priority; }
  static QString quote(const QString &text);
  static QString tpl(const QString &fileName);
  virtual QString js(bool add = true) const;
  void setText(const QString &text, int parseOptions);

protected:
  QDateTime dateTime() const;
  QString appendMessage(QString &html, const QString &func = QLatin1String("appendMessage")) const;
  QString button(const QVariant &data) const;
  QStringList vars(const QString &text) const;
  void extra(QString &html) const;
  void id(QString &html) const;
  void nick(QString &html) const;
  void replace(QString &text, const QVariantMap map) const;
  void text(QString &html) const;
  void time(QString &html) const;
  void type(QString &html) const;

  int m_direction;       ///< Направление сообщения.
  int m_parseOptions;    ///< Опции обработки сообщения.
  int m_priority;        ///< Приоритет сообщения.
  QByteArray m_destId;   ///< Идентификатор назначения.
  QByteArray m_senderId; ///< Идентификатор отправителя.
  qint64 m_timestamp;    ///< Отметка времени.
  QString m_bodyTpl;     ///< Шаблон тела сообщения.
  QString m_extra;       ///< Дополнительные css классы.
  QString m_id;          ///< Уникальный идентификатор сообщения.
  QString m_template;    ///< Базовый шаблон сообщения.
  QString m_text;        ///< Текст сообщения.
  QString m_timeTpl;     ///< Шаблон времени.
  QString m_type;        ///< Имя css класса типа сообщения.

private:
  static QHash<QString, QString> m_templates; ///< Таблица шаблонов для сообщений.
};

#endif /* ABSTRACTMESSAGE_H_ */