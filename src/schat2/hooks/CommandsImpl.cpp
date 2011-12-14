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

#include <QDebug>

#include "ChatCore.h"
#include "ChatNotify.h"
#include "ChatSettings.h"
#include "ChatUrls.h"
#include "client/ChatClient.h"
#include "client/ClientCmd.h"
#include "client/ClientMessages.h"
#include "client/SimpleClient.h"
#include "hooks/CommandsImpl.h"
#include "ui/StatusMenu.h"

namespace Hooks
{

CommandsImpl::CommandsImpl(QObject *parent)
  : Messages(parent)
{
  ChatClient::messages()->hooks()->add(this);
}


/*!
 * Обработка команд.
 */
bool CommandsImpl::command(const QByteArray &dest, const ClientCmd &cmd)
{
  Q_UNUSED(dest);

  QString command = cmd.command().toLower();

  /// - /about Открытие вкладки O Simple Chat.
  if (command == "about")
    ChatNotify::start(Notify::OpenAbout);

  /// - /away Установка статуса «Отсутствую» (Away).
  else if (command == "away")
    StatusMenu::apply(Status::Away);

  /// - /dnd Установка статуса «Не беспокоить» (Do not disturb).
  else if (command == "dnd")
    StatusMenu::apply(Status::DnD);

  /// - /exit Выход из чата.
  else if (command == "exit" || command == "quit")
    ChatNotify::start(Notify::Quit);

  /// - /ffc Установка статуса «Готов болтать» (Free for chat).
  else if (command == "ffc")
    StatusMenu::apply(Status::FreeForChat);

  /// - /hide Скрытие окна чата.
  else if (command == "hide")
    ChatNotify::start(Notify::ToggleVisibility);

  /// - /nick Установка нового ника.
  else if (command == "nick") {
    if (!Channel::isValidName(cmd.body()))
      return true;

    if (ChatClient::state() != ChatClient::Online) {
      ChatClient::io()->setNick(cmd.body());
      ChatCore::settings()->setValue("Profile/Nick", ChatClient::channel()->name());
      return true;
    }

    return false;
  }

  /// - /offline Установка статуса «Не в сети» (Offline) и отключение от сервера.
  else if (command == "offline")
    StatusMenu::apply(Status::Offline);

  /// - /online Установка статуса «В сети» (Online).
  else if (command == "online")
    StatusMenu::apply(Status::Online);

  /// - /open Открытие URL адреса.
  else if (command == "open")
    ChatUrls::open(cmd.body());

  /// - /set Установка опции чата, имя опции чувствительно к регистру символов.
  else if (command == QLatin1String("set")) {
    ClientCmd body(cmd.body());
    if (body.isValid() && body.isBody())
      ChatCore::settings()->setValue(body.command(), body.body());
  }
  else
    return false;

  return true;
}

} // namespace Hooks
