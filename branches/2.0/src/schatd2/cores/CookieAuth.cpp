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

#include "cores/CookieAuth.h"
#include "cores/Core.h"
#include "events.h"
#include "net/packets/auth.h"
#include "NodeLog.h"
#include "Storage.h"

CookieAuth::CookieAuth(Core *core)
  : NodeAuth(core)
{
}


AuthResult CookieAuth::auth(const AuthRequest &data)
{
  if (data.cookie.isEmpty() || SimpleID::typeOf(data.cookie) != SimpleID::CookieId)
    return AuthResult();

  Storage *storage = Storage::i();
  ChatUser exist = storage->user(data.cookie, true);
  if (!exist)
    return AuthResult();

  if (storage->user(exist->id()))
    return AuthResult(Notice::UserAlreadyExists, data.id); ///< \deprecated Необходима поддержка множественного входа.

  QString normalNick = storage->normalize(data.nick);
  if (storage->user(normalNick, false))
    return AuthResult(Notice::NickAlreadyUse, data.id, 0);

  ChatUser user = ChatUser(new ServerUser(normalNick, exist->id(), data, m_core->packetsEvent()->socket()));
  if (!user->isValid())
    return AuthResult(Notice::BadRequest, data.id);

  user->setUserAgent(data.userAgent);
  user->setHost(m_core->packetsEvent()->address.toString());
  user->setCookie(exist->cookie());
  user->setGroups(exist->groups());
  user->setAccount(exist->account());
  m_core->add(user, data.authType, data.id);

  SCHAT_LOG_DEBUG() << "COOKIE AUTH" << user->nick() << user->host() << SimpleID::encode(user->id()) << user->userAgent() << data.host;
  return AuthResult(user->id(), data.id);
}


int CookieAuth::type() const
{
  return AuthRequest::Cookie;
}


BypassCookieAuth::BypassCookieAuth(Core *core)
  : AnonymousAuth(core)
{
}


int BypassCookieAuth::type() const
{
  return AuthRequest::Cookie;
}
