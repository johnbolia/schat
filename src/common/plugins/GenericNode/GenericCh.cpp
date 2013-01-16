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

#include "Ch.h"
#include "feeds/FeedNames.h"
#include "feeds/FeedStorage.h"
#include "GenericCh.h"
#include "sglobal.h"

GenericCh::GenericCh(QObject *parent)
  : ChHook(parent)
{
}


void GenericCh::newChannel(ChatChannel channel, ChatChannel user)
{
  Ch::addNewFeedIfNotExist(channel, FEED_ACL, user);
}


void GenericCh::rename(ChatChannel channel)
{
  if (channel->type() == SimpleID::ServerId)
    FeedStorage::save(channel->feed(LS("server")), channel->date());
}


void GenericCh::server(ChatChannel channel, bool created)
{
  Q_UNUSED(created)

  channel->feed(FEED_ACL);
  channel->feed(LS("server"));
}


void GenericCh::updateStatistics()
{
  Ch::server()->feed(LS("server"))->head().setDate();
}


void GenericCh::userChannel(ChatChannel channel)
{
  if (!channel->account())
    channel->createAccount();

  channel->feed(LS("account"));

  Ch::addNewUserFeedIfNotExist(channel, FEED_ACL);
  Ch::addNewUserFeedIfNotExist(channel, LS("profile"));
}
