/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2012 IMPOMEZIA <schat@impomezia.com>
 * Copyright © 2012 Alexey Ivanov <alexey.ivanes@gmail.com>
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

#ifndef HUNSPELLCHECKER_H
#define HUNSPELLCHECKER_H

#include <QMap>

#include "SpellBackend.h"

class Hunspell;

class HunspellChecker: public SpellBackend
{
public:
  HunspellChecker(QObject *parent = 0);
  ~HunspellChecker();

  bool add(const QString &word);
  bool available() const;
  bool isCorrect(const QString &word) const;
  QStringList dictionaries() const;
  QStringList suggestions(const QString &word) const;
  void setLangs(const QStringList &dicts);

private:
  void loadHunspell(const QStringList &dicts);

  QMap<QString, Hunspell*> FHunSpellMap;
  QMap<QString, QByteArray> FDictionaryMap;
  QString dictPath;
};

#endif
