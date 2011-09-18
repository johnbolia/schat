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

#include <QRegExp>

#include "text/HtmlFilter.h"

HtmlFilter::HtmlFilter()
  : AbstractFilter(QLatin1String("Html"))
{
}


QString HtmlFilter::filter(const QString &text, QVariantHash options) const
{
  Q_UNUSED(options)

  QString out = text;

  /// Очищает html документ от неотображаемой информации.
  QRegExp badStuff(QLatin1String("<![^<>]*>|<head[^<>]*>.*</head[^<>]*>|</?html[^<>]*>|</?body[^<>]*>|</?p[^<>]*>"));
  badStuff.setCaseSensitivity(Qt::CaseInsensitive);
  out.remove(badStuff);
  out = out.trimmed();

  if (out.isEmpty())
    return "";

  out.remove('\r');

  /// Обрезает до максимальной длинны.
//  if (left)
//    out = out.left(left);
//
//  if (!strict)
//    return out;

  /// Удаляет запрещённые теги.
  QList<QLatin1String> otherBadTags;
  otherBadTags << QLatin1String("address")
               << QLatin1String("big")
               << QLatin1String("blockquote")
               << QLatin1String("center")
               << QLatin1String("dd")
               << QLatin1String("div")
               << QLatin1String("dl")
               << QLatin1String("dt")
               << QLatin1String("font")
               << QLatin1String("h1")
               << QLatin1String("h2")
               << QLatin1String("h3")
               << QLatin1String("h4")
               << QLatin1String("h5")
               << QLatin1String("h6")
               << QLatin1String("hr")
               << QLatin1String("kbd")
               << QLatin1String("li")
               << QLatin1String("ol")
               << QLatin1String("qt")
               << QLatin1String("small")
               << QLatin1String("sub")
               << QLatin1String("sup")
               << QLatin1String("table")
               << QLatin1String("tbody")
               << QLatin1String("td")
               << QLatin1String("tfoot")
               << QLatin1String("th")
               << QLatin1String("thead")
               << QLatin1String("tr")
               << QLatin1String("img")
               << QLatin1String("ul");

  foreach (QString tag, otherBadTags) {
    badStuff.setPattern(QString("</?%1[^<>]*>").arg(tag));
    out.remove(badStuff);
  }

  /// Удаляет пустые ссылки.
  badStuff.setPattern(QLatin1String("<a[^<]*>[\\s]*</a>"));
  out.remove(badStuff);

  out.replace("  ", "&nbsp;&nbsp;");

  /// Заменяет перенос строк на соответствующий html код.
  out.replace(QLatin1String("\n"), QLatin1String("<br />"));

  /// Заменяет двойные переносы строк на одинарные.
  while (out.contains(QLatin1String("<br /><br /><br />")))
    out.replace(QLatin1String("<br /><br /><br />"), QLatin1String("<br /><br />"));

  /// Удаляет код переноса строки если тот находится в конце сообщения.
  out.replace(QLatin1String("<br /><br /></span>"), QLatin1String("<br /></span>"));
  out.replace(QLatin1String("<br /></span>"), QLatin1String("</span>"));

  if (out.endsWith(QLatin1String("<br /><br />")))
    out = out.left(out.size() - 12);
  if (out.endsWith(QLatin1String("<br />")))
    out = out.left(out.size() - 6);

  /// Удаляет запрещённые css стили.
  /// \todo Эта функция также удалит заданные селекторы из текста, что не допустимо.
  badStuff.setPattern("\\s?font-size:[^;]*;|\\s?background-color:[^;]*;|\\s?font-family:[^;]*;");
  out.remove(badStuff);

//  if (toPlainText(out).isEmpty()) /// FIXME ! toPlainText
//    return "";

  return out;
}
