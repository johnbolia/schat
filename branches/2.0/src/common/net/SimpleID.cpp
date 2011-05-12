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

#include <QByteArray>
#include <QCryptographicHash>
#include <QNetworkInterface>
#include <QUuid>

#define SCHAT_RANDOM_CLIENT_ID

#include "net/Protocol.h"
#include "net/SimpleID.h"


/*!
 * Проверяет идентификатор на вхождение в диапазон пользовательских идентификаторов.
 *
 * \param id Проверяемый идентификатор.
 * \return true если идентификатор входит в диапазон.
 */
bool SimpleID::isUserRoleId(const QByteArray &id)
{
  int type = typeOf(id);
  if (type < MinUserRoleId || type > MaxUserRoleId)
    return false;

  return true;
}


/*!
 * Проверяет идентификатор на вхождение в диапазон пользовательских идентификаторов,
 * а также на принадлежность к пользователю \p userId.
 *
 * \param userId Идентификатор пользователя.
 * \param id     Проверяемый идентификатор.
 * \return true если идентификатор входит в диапазон.
 */
bool SimpleID::isUserRoleId(const QByteArray &userId, const QByteArray &id)
{
  if (!isUserRoleId(id))
    return false;

  if (userId.left(DefaultSize - 1) == id.left(DefaultSize - 1))
    return true;

  return false;
}


/*!
 * Возвращает тип идентификатора.
 */
int SimpleID::typeOf(const QByteArray &id)
{
  if (id.size() != DefaultSize)
    return InvalidId;

  return quint8(id.at(DefaultSize - 1));
}


QByteArray SimpleID::session(const QByteArray &id)
{
  return QCryptographicHash::hash(QString(id + QUuid::createUuid()).toLatin1(), QCryptographicHash::Sha1) += SessionId;
}


QByteArray SimpleID::setType(int type, const QByteArray &id)
{
  if (id.size() != DefaultSize)
    return id;

  return id.left(DefaultSize - 1) += type;
}


/*!
 * Получение уникального идентификатора клиента на основе
 * mac адреса первого активного сетевого интерфейса.
 */
QByteArray SimpleID::uniqueId()
{
  #if !defined(SCHAT_DAEMON) && defined(SCHAT_RANDOM_CLIENT_ID)
  return QCryptographicHash::hash(QUuid::createUuid().toString().toLatin1(), QCryptographicHash::Sha1) += UniqueUserId;
  #endif

  QList<QNetworkInterface> all = QNetworkInterface::allInterfaces();
  foreach (QNetworkInterface iface, all) {
    QString hw = iface.hardwareAddress();
    if (!hw.isEmpty() && iface.flags().testFlag(QNetworkInterface::IsUp) && iface.flags().testFlag(QNetworkInterface::IsRunning)) {
      return QCryptographicHash::hash(hw.toLatin1(), QCryptographicHash::Sha1) += UniqueUserId;
    }
  }

  return QCryptographicHash::hash("", QCryptographicHash::Sha1);
}
