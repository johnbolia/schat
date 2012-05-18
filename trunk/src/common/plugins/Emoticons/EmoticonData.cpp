/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2012 IMPOMEZIA <schat@impomezia.com>
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

#include <QFile>
#include <QVariant>

#include "EmoticonData.h"
#include "sglobal.h"

EmoticonData::EmoticonData(const QString &file, const QString &id, const QVariantMap &data)
  : m_height(0)
  , m_width(0)
  , m_file(file)
  , m_id(id)
{
  if (data.isEmpty())
    return;

  QVariantList size = data.value(LS("size")).toList();
  if (size.size() < 2)
    return;

  m_width = size.at(0).toInt();
  m_height = size.at(1).toInt();

  QVariantList text = data.value(LS("text")).toList();
  if (text.isEmpty())
    return;

  for (int i = 0; i < text.size(); ++i) {
    QString t = text.at(i).toString();
    if (t.isEmpty())
      continue;

    if (!m_texts.contains(t))
      m_texts.append(t);
  }
}


bool EmoticonData::isValid() const
{
  if (!QFile::exists(m_file))
    return false;

  if (m_width <= 0)
    return false;

  if (m_height <= 0)
    return false;

  if (m_texts.isEmpty())
    return false;

  return true;
}
