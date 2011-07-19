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

#ifndef USER_H_
#define USER_H_

#include <QHash>
#include <QSharedPointer>
#include "schat.h"

/*!
 * Пользователь.
 */
class SCHAT_EXPORT User
{
public:
  /// Ограничения.
  enum Limits {
    MinNickLengh = 3,     ///< Минимальная длина ника.
    MaxNickLength = 20,   ///< Максимальная длина ника.
    MaxByeMsgLength = 128 ///< Максимальная длина сообщения при выходе.
  };

  /// Пол пользователя.
  enum Gender {
    Male = 0,      ///< Мужской пол.
    Female = 100,  ///< Женский пол.
    Unknown = 150, ///< Неизвестный пол.
    Ghost = 151    ///< Привидение.
  };

  /// Цветовой вариант иконки пола.
  enum Color {
    Default,
    Black,
    Gray,
    Green,
    Red,
    White,
    Yellow,
    Medical,
    Nude,
    Thief
  };

  /// Статус.
  enum Status {
    OfflineStatus,
    OnlineStatus,
    AwayStatus,
    AutoAwayStatus,
    DnDStatus,
    FreeForChatStatus
  };

  User();
  User(const QString &nick);
  User(const User *other);
  virtual ~User();
  inline bool isValid() const { return m_valid; }

  // m_id.
  bool setId(const QByteArray &id);
  inline QByteArray id() const { return m_id; }

  // m_nick.
  bool setNick(const QString &nick);
  inline QString nick() const { return m_nick; }
  static bool isValidNick(const QString &nick);
  static QString defaultNick();

  // m_gender.
  inline int rawGender() const         { return m_gender; }
  inline void setRawGender(int gender) { m_gender = gender; }
  int color() const;
  int gender() const;
  void setColor(Color color);
  void setColor(int color);
  void setGender(Gender gender);

  // m_ids.
  bool addId(int type, const QByteArray &id);
  bool containsId(int type, const QByteArray &id);
  bool remove(int type);
  bool removeId(int type, const QByteArray &id);
  int count(int type);
  QList<QByteArray> ids(int type);

  // network.
  inline QString host() const { return m_host; }
  inline QString userAgent() const { return m_userAgent; }
  inline void setHost(const QString &adderss) { m_host = adderss; }
  inline void setUserAgent(const QString &agent) { m_userAgent = agent; }

  // m_status
  bool setStatus(const QString &text);
  inline int status() const { return m_status; }
  inline QString statusToString() { return statusToString(m_status, m_statuses.value(m_status)); }
  inline void setStatus(int status) { m_status = status; }
  QString statusText(int status = -1) const;
  static QString statusToString(int status, const QString &text);
  void setStatus(int status, const QString &text);

protected:
  inline bool validate(bool valid) { if (valid) return true; else m_valid = false; return false; }

  bool m_valid;                         ///< true все данные корректны.
  int m_gender;                         ///< Пол и цвет иконки.
  int m_status;                         ///< Код статуса.
  QByteArray m_id;                      ///< Идентификатор пользователя.
  QHash<int, QList<QByteArray> > m_ids; ///< Списки идентификаторов.
  QHash<int, QString> m_statuses;       ///< Статусы.
  QString m_host;                       ///< Адрес пользователя.
  QString m_nick;                       ///< Ник пользователя.
  QString m_userAgent;                  ///< User Agent пользователя.
};

typedef QSharedPointer<User> ClientUser;

#endif /* USER_H_ */
