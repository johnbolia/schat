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

#include "feeds/FeedStorage.h"
#include "GenericCh.h"

GenericCh::GenericCh(QObject *parent)
  : Ch(parent)
{
}


void GenericCh::newChannelImpl(ChatChannel channel, ChatChannel user)
{
  if (channel->type() == SimpleID::ChannelId) {
    FeedPtr acl = channel->feed("acl", true, false);
    if (user)
      acl->head().acl().add(user->id());

    FeedStorage::save(acl);

    channel->feed("topic");
  }
  else if (channel->type() == SimpleID::ServerId) {
    channel->feed("acl");
  }
}


void GenericCh::newUserChannelImpl(ChatChannel channel, const AuthRequest & /*data*/, const QString & /*host*/, bool /*created*/)
{
  if (!channel->account())
    channel->createAccount();

  channel->feed("account");

  FeedPtr acl = channel->feed("acl", true, false);
  acl->head().acl().add(channel->id());
  FeedStorage::save(acl);
}