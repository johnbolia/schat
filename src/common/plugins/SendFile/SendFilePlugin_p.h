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

#ifndef SENDFILEPLUGIN_P_H_
#define SENDFILEPLUGIN_P_H_

#include <QVariant>

#include "net/packets/MessageNotice.h"
#include "plugins/ChatPlugin.h"
#include "SendFileTransaction.h"

class ChatView;
class SendFileTr;

class SendFilePluginImpl : public ChatPlugin
{
  Q_OBJECT

public:
  SendFilePluginImpl(QObject *parent);
  ~SendFilePluginImpl();

  bool sendFile(const QByteArray &dest, const QString &file);
  void read(const MessagePacket &packet);

private slots:
  void init(ChatView *view);
  void loadFinished(ChatView *view);

private:
  void incomingFile(const MessagePacket &packet);

  QHash<QByteArray, SendFileTransaction> m_incoming; ///< Входящие файлы.
  QHash<QByteArray, SendFileTransaction> m_outgoing; ///< Отправленные файлы.
  SendFileTr *m_tr;                                  ///< Класс перевода строк.
};

#endif /* SENDFILEPLUGIN_P_H_ */
