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

#ifndef NETWORKMANAGER_H_
#define NETWORKMANAGER_H_

#include <QHash>
#include <QObject>
#include <QStringList>
#include <QVariant>

#include "schat.h"

class ChatSettings;
class FileLocations;
class MessageAdapter;
class NetworkManager;
class SettingsLegacy;
class SimpleClient;

/*!
 * Используется для хранения информации о подключении.
 * Поддерживает запись и чтение информации из конфигурационного файла.
 */
class SCHAT_CORE_EXPORT NetworkItem
{
public:
  NetworkItem();
  NetworkItem(const QByteArray &id);
  bool isValid() const;
  inline bool isAuthorized() const { return m_authorized; }
  inline QByteArray cookie() const { return m_cookie; }
  inline QByteArray id() const { return m_id; }
  inline QString account() const { return m_account; }
  inline QString name() const { return m_name; }
  inline QString password() const { return m_password; }
  inline QString url() const { return m_url; }
  inline void setAccount(const QString &account) { m_account = account; }
  inline void setPassword(const QString &password) { m_password = password; }
  inline void setUrl(const QString &url) { m_url = url; }
  static NetworkItem item();
  void read();
  void write();

  friend class NetworkManager;

private:
  QString auth();
  void setAuth(const QString &auth);

  bool m_authorized;     ///< \p true если пользователь авторизирован.
  QByteArray m_cookie;   ///< Cookie.
  QByteArray m_id;       ///< Идентификатор сервера.
  QByteArray m_userId;   ///< Идентификатор пользователя.
  QString m_account;     ///< Имя пользователя.
  QString m_name;        ///< Имя сервера.
  QString m_password;    ///< Пароль.
  QString m_url;         ///< Адрес сервера.
};


/*!
 * Менеджер подключений.
 */
class SCHAT_CORE_EXPORT NetworkManager : public QObject
{
  Q_OBJECT

public:
  NetworkManager(QObject *parent = 0);
  bool isAutoConnect() const;
  bool open();
  bool open(const QByteArray &id);
  bool open(const QString &url);
  inline bool isItem(const QByteArray &id) const { if (id.isEmpty()) return false; return m_items.contains(id); }
  inline const QByteArray& tmpId() const { return m_tmpId; }
  inline int count() const { return m_items.count(); }
  inline NetworkItem item() const { return item(serverId()); }
  inline NetworkItem item(const QByteArray &id) const { return m_items.value(id); }
  inline NetworkItem& edit(const QByteArray &id) { return m_items[id]; }
  inline QByteArray selected() const { return m_selected; }
  inline QString root() const { return root(serverId()); }
  int isSelectedActive() const;
  QByteArray serverId() const;
  QList<NetworkItem> items() const;
  static QString currentServerName();
  void removeItem(const QByteArray &id);
  void setSelected(const QByteArray &id);

private slots:
  void clientStateChanged(int state);
  void loggedIn(const QString &name);

private:
  QString root(const QByteArray &id) const;
  QStringList networkList() const;
  void load();
  void write();

  ChatSettings *m_settings; ///< Основные настройки.
  FileLocations *m_locations;
  int m_invalids;
  MessageAdapter *m_adapter;
  QByteArray m_tmpId;       ///< Временный идентификатор для текущего редактируемой сети.
  QHash<QByteArray, NetworkItem> m_items;
  QByteArray m_selected;     ///< Текущая выбранная сеть в настройках.
  SimpleClient *m_client;   ///< Указатель на клиент.
};

#endif /* NETWORKMANAGER_H_ */
