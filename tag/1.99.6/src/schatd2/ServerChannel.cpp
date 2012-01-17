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

#include "Account.h"
#include "DataBase.h"
#include "DateTime.h"
#include "feeds/FeedStorage.h"
#include "net/SimpleID.h"
#include "Normalize.h"
#include "ServerChannel.h"
#include "sglobal.h"

ServerChannel::ServerChannel(ClientChannel channel)
  : Channel(channel->id(), channel->name())
{
  normalize();

  channels().set(channel->channels().all());
}


ServerChannel::ServerChannel(const QByteArray &id, const QString &name)
  : Channel(id, name)
{
  normalize();
}

ServerChannel::~ServerChannel()
{
}


bool ServerChannel::setName(const QString &name)
{
  if (Channel::setName(name)) {
    normalize();
    return true;
  }

  return false;
}


void ServerChannel::createAccount()
{
  m_account = new Account();
  m_account->setDate(DateTime::utc());
  m_account->groups() += LS("anonymous");
  m_account->setChannel(key());

  if (DataBase::noMaster) {
    m_account->groups() += LS("master");
    DataBase::noMaster = false;
  }

  DataBase::add(account());
}


bool ServerChannel::canEdit(ChatChannel channel)
{
  return feed("acl")->head().acl().can(channel.data(), Acl::Edit);
}


bool ServerChannel::canRead(ChatChannel channel)
{
  return feed("acl")->head().acl().can(channel.data(), Acl::Read);
}


bool ServerChannel::canWrite(ChatChannel channel)
{
  return feed("acl")->head().acl().can(channel.data(), Acl::Write);
}


/*!
 * Получение фида по имени, эта функция позволяет автоматически создать фид, если он не существует.
 *
 * \param name   Имя фида.
 * \param create \b true если необходимо создать фид, если он не существует.
 * \param save   \b true если необходимо сохранить фид после создания.
 */
FeedPtr ServerChannel::feed(const QString &name, bool create, bool save)
{
  FeedPtr feed = feeds().all().value(name);
  if (feed || !create)
    return feed;

  feeds().add(FeedStorage::create(name), save);
  return this->feed(name, false);
}


void ServerChannel::normalize()
{
  m_normalized = Normalize::toId(this);
}
