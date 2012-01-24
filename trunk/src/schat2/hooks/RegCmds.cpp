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

#include "client/ChatClient.h"
#include "client/ClientCmd.h"
#include "client/ClientMessages.h"
#include "hooks/RegCmds.h"
#include "net/SimpleID.h"
#include "sglobal.h"
#include "client/ClientFeeds.h"

RegCmds::RegCmds(QObject *parent)
  : Messages(parent)
{
  ChatClient::messages()->hooks()->add(this);
}


bool RegCmds::command(const QByteArray & /*dest*/, const ClientCmd &cmd)
{
  QString command = cmd.command().toLower();
  if (command == LS("reg")) {
    ClientCmd body(cmd.body());
    if (!body.isValid())
      return true;

    ChatClient::feeds()->request(ChatClient::id(), LS("query"), LS("account"), reg(body.command(), body.body()));
    return true;
  }

  return false;
}


QVariantMap RegCmds::reg(const QString &name, const QString &password)
{
  QVariantMap out;
  out[LS("action")] = LS("reg");
  out[LS("name")]   = name.simplified().toLower().remove(LC(' '));
  out[LS("pass")]   = SimpleID::encode(SimpleID::password(password));

  return out;
}
