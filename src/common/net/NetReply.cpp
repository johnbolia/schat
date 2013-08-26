/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2013 IMPOMEZIA <schat@impomezia.com>
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

#include "JSON.h"
#include "net/NetReply.h"
#include "sglobal.h"

NetReply::NetReply(const QString &id, int status)
  : id(id)
  , status(status)
{
}


NetReply::NetReply(const QVariantList &list)
{
  if (list.size() < 5)
    return;

  id      = list.at(1).toString();
  status  = list.at(2).toInt();
  headers = list.at(3).toMap();
  data    = list.mid(4);
}


bool NetReply::isValid() const
{
  if (id.isEmpty())
    return false;

  return true;
}


QByteArray NetReply::toJSON() const
{
  QVariantList list;
  list.append(LS("REP"));
  list.append(id);
  list.append(status);
  list.append(headers);

  if (data.canConvert(QVariant::List)) {
    const QVariantList d = data.toList();
    if (!d.isEmpty())
      list.append(data.toList());
    else
      list.append(QVariant());
  }
  else
    list.append(data);

  return JSON::generate(list);
}
