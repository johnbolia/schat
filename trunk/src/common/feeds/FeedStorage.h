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

#ifndef FEEDSTORAGE_H_
#define FEEDSTORAGE_H_

#include <QObject>

#include "feeds/Feed.h"
#include "schat.h"

class SCHAT_EXPORT FeedStorage : public QObject
{
  Q_OBJECT

public:
  FeedStorage(QObject *parent = 0);
  inline static int save(FeedPtr feed)         { return m_self->saveImpl(feed); }
  inline static void add(FeedStorage *hook)    { if (!m_self->m_hooks.contains(hook)) m_self->m_hooks.append(hook); }
  inline static void remove(FeedStorage *hook) { m_self->m_hooks.removeAll(hook); }

protected:
  virtual int saveImpl(FeedPtr feed);

private:
  QList<FeedStorage*> m_hooks; ///< Хуки.
  static FeedStorage *m_self;  ///< Указатель на себя.
};

#endif /* FEEDSTORAGE_H_ */
