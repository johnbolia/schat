/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2014 IMPOMEZIA <schat@impomezia.com>
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

#ifndef SHAREPLUGIN_P_H_
#define SHAREPLUGIN_P_H_

#include <QMap>

#include "id/ChatId.h"
#include "net/packets/MessageNotice.h"
#include "plugins/ChatPlugin.h"

class ChatView;
class QNetworkAccessManager;
class QNetworkReply;
class SharePluginTr;
class UploadData;

class Share : public ChatPlugin
{
  Q_OBJECT

public:
  Share(QObject *parent);
  ~Share();
  bool upload(const ChatId &roomId, const QList<QUrl> &urls, bool local = true);
  Q_INVOKABLE bool cancel(const QString &oid);
  Q_INVOKABLE void upload(const QString &oid, const QString &desc);
  void read(const MessagePacket &packet);

signals:
  void uploadAdded(const QVariantMap &data);
  void uploadProgress(const QString &roomId, const QString &oid, qint64 bytesSent, qint64 bytesTotal);
  void uploadStarted(const QString &roomId, const QString &oid);
  void uploadStatus(const QString &roomId, const QString &oid, int status);

private slots:
  void onFinished();
  void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);
  void start();

private:
  void add(UploadData *data);

  ChatId m_id;                            ///< Идентификатор загрузки.
  ChatId m_roomId;                        ///< Идентификатор комнаты.
  QMap<ChatId, UploadData*> m_data;       ///< Данные для загрузки файлов.
  QNetworkAccessManager *m_net;           ///< Доступ к сети.
  SharePluginTr *m_tr;                    ///< Перевод строк в js коде.
};

#endif /* SHAREPLUGIN_P_H_ */
