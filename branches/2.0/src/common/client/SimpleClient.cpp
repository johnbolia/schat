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

#include <QBasicTimer>
#include <QTimerEvent>

#include "Channel.h"
#include "client/SimpleClient.h"
#include "client/SimpleClient_p.h"
#include "debugstream.h"
#include "net/PacketReader.h"
#include "net/packets/auth.h"
#include "net/packets/channels.h"
#include "net/packets/message.h"
#include "net/packets/notices.h"
#include "net/packets/users.h"
#include "net/PacketWriter.h"
#include "net/Protocol.h"
#include "User.h"

SimpleClient::SimpleClient(QObject *parent)
  : SimpleSocket(*new SimpleSocketPrivate(), parent)
  , m_sendLock(false)
  , m_user(new User())
  , m_clientState(ClientOffline)
  , m_reconnects(0)
  , m_syncChannelCache(0)
{
  m_user->setUserAgent(SimpleID::userAgent());

  m_reconnectTimer = new QBasicTimer;
  m_uniqueId = SimpleID::uniqueId();
  m_serverData = new ServerData();

  connect(this, SIGNAL(requestAuth(quint64)), SLOT(requestAuth()));
  connect(this, SIGNAL(released(quint64)), SLOT(released()));
}


SimpleClient::SimpleClient(SimpleClientPrivate &dd, QObject *parent)
  : SimpleSocket(dd, parent)
{
  m_user->setUserAgent(SimpleID::userAgent());

  m_reconnectTimer = new QBasicTimer;
  m_uniqueId = SimpleID::uniqueId();
  m_serverData = new ServerData();

  connect(this, SIGNAL(requestAuth(quint64)), SLOT(requestAuth()));
  connect(this, SIGNAL(released(quint64)), SLOT(released()));
}


SimpleClient::~SimpleClient()
{
  if (m_reconnectTimer->isActive())
    m_reconnectTimer->stop();

  delete m_reconnectTimer;
  delete m_serverData;
}


/*!
 * Установка подключения к серверу.
 */
bool SimpleClient::openUrl(const QUrl &url)
{
  SCHAT_DEBUG_STREAM(this << "openUrl()" << url.toString())

  m_url = url;

  if (!m_url.isValid())
    return false;

  if (m_url.scheme() != "schat")
    return false;

  if (m_reconnectTimer->isActive())
    m_reconnectTimer->stop();

  if (state() != QAbstractSocket::UnconnectedState) {
    leave();
  }

  if (!m_nick.isEmpty())
    m_user->setNick(m_nick);

  setClientState(ClientConnecting);
  connectToHost(url.host(), url.port(Protocol::DefaultPort));
  return true;
}


/*!
 * Отправка сообщения.
 */
bool SimpleClient::send(const MessageData &data)
{
  Q_D(SimpleClient);
  QList<QByteArray> packets;
  QByteArray dest = data.destId;

  packets.append(MessageWriter(d->sendStream, data).data());
  return send(packets);
}


bool SimpleClient::send(const QByteArray &packet)
{
  if (m_sendLock) {
    m_sendQueue.append(packet);
    return true;
  }

  return SimpleSocket::send(packet);
}



bool SimpleClient::send(const QList<QByteArray> &packets)
{
  if (m_sendLock) {
    m_sendQueue.append(packets);
    return true;
  }

  return SimpleSocket::send(packets);
}


QByteArray SimpleClient::serverId() const
{
  return m_serverData->id();
}


/*!
 * Получение оригинального ника, не искажённого функцией автоматического
 * разрешения коллизий.
 */
QString SimpleClient::nick() const
{
  if (m_nick.isEmpty())
    return m_user->nick();

  return m_nick;
}


void SimpleClient::setNick(const QString &nick)
{
  m_user->setNick(nick);
  m_nick = "";
}


void SimpleClient::leave()
{
  SCHAT_DEBUG_STREAM(this << "leave()")

  if (m_reconnectTimer->isActive())
    m_reconnectTimer->stop();

  setClientState(ClientOffline);
  SimpleSocket::leave();
  setAuthorized(QByteArray());
}


/*!
 * Отключение от канала.
 */
void SimpleClient::part(const QByteArray &channelId)
{
  SCHAT_DEBUG_STREAM(this << "part()" << channelId.toHex())

  if (!m_channels.contains(channelId))
    return;

  m_channels.remove(channelId);
  user()->removeId(SimpleID::ChannelListId, channelId);

  MessageData message(userId(), channelId, "part", "");
  send(message);
}


/*!
 * Обработка пакетов.
 */
void SimpleClient::newPacketsImpl()
{
  Q_D(SimpleClient);
  SCHAT_DEBUG_STREAM(this << "newPacketsImpl()" << d->readQueue.size())

  while (!d->readQueue.isEmpty()) {
    d->readBuffer = d->readQueue.takeFirst();
    PacketReader reader(d->readStream);
    m_reader = &reader;

    switch (reader.type()) {
      case Protocol::AuthReplyPacket:
        readAuthReply();
        break;

      case Protocol::ChannelPacket:
        readChannel();
        break;

      case Protocol::MessagePacket:
        readMessage();
        break;

      case Protocol::UserDataPacket:
        readUserData();
        break;

      case Protocol::NoticePacket:
        readNotice();
        break;

      default:
        break;
    }
  }

  if (m_syncChannelCache && !m_syncChannelCache->users.isEmpty()) {
    emit join(m_syncChannelCache->id, m_syncChannelCache->users);
    m_syncChannelCache->users.clear();
  }
}


void SimpleClient::timerEvent(QTimerEvent *event)
{
  if (event->timerId() == m_reconnectTimer->timerId()) {
    openUrl(m_url);
    return;
  }

  SimpleSocket::timerEvent(event);
}


/*!
 * Обработка разрыва соединения.
 */
void SimpleClient::released()
{
  SCHAT_DEBUG_STREAM(this << "released()" << errorString() << isAuthorized())

  if (m_reconnectTimer->isActive())
    m_reconnectTimer->stop();

  if (m_clientState == ClientOffline)
    return;

  if (m_clientState == ClientOnline) {
    setClientState(ClientOffline);
    setClientState(ClientConnecting);
  }

  if (m_reconnects < Protocol::MaxFastReconnects) {
    m_reconnectTimer->start(Protocol::FastReconnectTime, this);
  }
  else if (m_reconnects < Protocol::MaxFastReconnects + Protocol::MaxNormalReconnects) {
    m_reconnectTimer->start(Protocol::NormalReconnectTime, this);
  }
  else {
    m_reconnectTimer->start(Protocol::SlowReconnectTime, this);
  }

  ++m_reconnects;
}


void SimpleClient::requestAuth()
{
  SCHAT_DEBUG_STREAM(this << "requestAuth()")

  Q_D(SimpleClient);
  AuthRequestData data(AuthRequestData::Anonymous, m_url.host(), m_user.data());
  data.uniqueId = m_uniqueId;
  send(AuthRequestWriter(d->sendStream, data).data());
}


/*!
 * Добавление нового канала.
 * В случае если идентификатор канала уже используется, возможны 2 сценария:
 * - Если существующий канал содержит пользователей, добавления не произойдёт,
 * объект \p channel будет удалён и функция возвратит false.
 * - Существующий канал не содержит пользователей, что означает его не валидность,
 * объект канала будет удалён и его место в таблице каналов займёт \p channel.
 *
 * \return true в случае успешного добавления канала.
 * \todo ! Высылать сигнал о новых пользователях канала если информация о них уже имеется у клиента.
 */
bool SimpleClient::addChannel(Channel *channel)
{
  QByteArray id = channel->id();
  Channel *ch = m_channels.value(id);

  if (ch) {
    if (ch->userCount()) {
      delete channel;
      return false;
    }
    else {
      m_channels.remove(id);
      delete ch;
    }
  }

  qDebug() << "###" << channel->userCount();
  m_channels.insert(id, channel);
  channel->addUser(userId());
  user()->addId(SimpleID::ChannelListId, id);

  emit join(id, QByteArray());
  return true;
}


QString SimpleClient::mangleNick()
{
  int rand = qrand() % 89 + 10;
  if (m_nick.isEmpty())
    m_nick = m_user->nick();

  return m_nick.left(User::MaxNickLength - 2) + QString::number(rand);
}


/*!
 * Очистка данных состояния клиента.
 */
void SimpleClient::clearClient()
{
  m_user->remove(SimpleID::ChannelListId);
  m_user->remove(SimpleID::TalksListId);

  m_users.clear();
  m_users.insert(userId(), m_user);

  qDeleteAll(m_channels);
  m_channels.clear();
}


/*!
 * Восстановление состояния клиента после повторного подключения к предыдущему серверу.
 */
void SimpleClient::restore()
{
  Q_D(SimpleClient);
  QList<QByteArray> packets;

  /// Происходит восстановление приватных разговоров.
  if (user()->count(SimpleID::TalksListId)) {
    MessageData data(userId(), SimpleID::setType(SimpleID::TalksListId, userId()), "add", "");
    MessageWriter writer(d->sendStream, data);
    writer.putId(user()->ids(SimpleID::TalksListId));
    packets.append(writer.data());
  }

  /// Клиент заново входит в ранее открытие каналы.
  if (!m_channels.isEmpty()) {
    MessageData data(userId(), QByteArray(), "join", "");
    data.options |= MessageData::TextOption;

    QHashIterator<QByteArray, Channel*> i(m_channels);
    while (i.hasNext()) {
      i.next();
      data.destId = i.key();
      data.text = i.value()->name();
      packets.append(MessageWriter(d->sendStream, data).data());
    }
  }

  clearClient();

  if (!packets.isEmpty())
    send(packets);
}


/*!
 * Установка состояния клиента.
 */
void SimpleClient::setClientState(ClientState state)
{
  if (m_clientState == state)
    return;

  m_clientState = state;

  if (state == ClientOnline || state == ClientOffline)
    m_reconnects = 0;

  if (state == ClientOnline) {
    m_users.insert(userId(), m_user);
  }
  else {
    foreach (Channel *chan, m_channels) {
      chan->clear();
    }
  }

  emit clientStateChanged(m_clientState);
}


/*!
 * Установка идентификатора сервера, при успешной авторизации.
 *
 * В случае повторного подключения к предыдущему серверу происходит,
 * вход в раннее открытые каналы.
 * В случае если подключение происходит к новому серверу, таблица каналов очищается.
 *
 * \param data Данные сервера.
 */
void SimpleClient::setServerData(const ServerData &data)
{
  Q_D(SimpleClient);
  bool sameServer = false;

  if (!m_serverData->id().isEmpty() && m_serverData->id() == data.id())
    sameServer = true;

  m_serverData->setId(data.id());
  m_serverData->setName(data.name());
  m_serverData->setChannelId(data.channelId());
  m_serverData->setFeatures(data.features());

  setClientState(ClientOnline);

  if (!sameServer) {
    clearClient();

    if (m_serverData->features() & ServerData::AutoJoinSupport && !m_serverData->channelId().isEmpty()) {
      MessageData data(userId(), m_serverData->channelId(), "join", "");
      send(MessageWriter(d->sendStream, data).data());
    }
  }
  else
    restore();
}


void SimpleClient::unlock()
{
  if (!m_sendQueue.isEmpty()) {
    SimpleSocket::send(m_sendQueue);
    m_sendQueue.clear();
  }

  m_sendLock = false;
}


/*!
 * Обработка команд.
 *
 * \return true в случае если команда была обработана, иначе false.
 */
bool SimpleClient::command()
{
  QString command = m_messageData->command;
  SCHAT_DEBUG_STREAM(this << "command()" << command)

  if (command == "part") {
    removeUserFromChannel(m_reader->dest(), m_reader->sender());
    return true;
  }

  if (command == "BSCh") {
    lock();
    m_syncChannelCache = new SyncChannelCache();
    m_syncChannelCache->id = m_reader->dest();
    return true;
  }

  if (command == "ESCh") {
    unlock();
    m_syncChannelCache->users.append(QByteArray());

    if (!m_syncChannelCache->users.isEmpty()) {
      emit join(m_syncChannelCache->id, m_syncChannelCache->users);
      m_syncChannelCache->users.clear();
    }

    delete m_syncChannelCache;
    m_syncChannelCache = 0;
    return true;
  }

  if (command == "leave") {
    removeUser(m_reader->sender());
    return true;
  }

  if (command == "status") {
    updateUserStatus(m_messageData->text);
    return true;
  }

  return false;
}


/*!
 * Чтение пакета AuthReplyPacket.
 */
bool SimpleClient::readAuthReply()
{
  AuthReplyData data = AuthReplyReader(m_reader).data;

  SCHAT_DEBUG_STREAM(this << "AuthReply" << data.status << data.error << data.userId.toHex())

  if (data.status == AuthReplyData::AccessGranted) {
    setAuthorized(data.userId);
    m_user->setId(data.userId);
    m_user->setHost(data.host);

    setServerData(data.serverData);

    if (m_user->status() == User::OfflineStatus)
      m_user->setStatus(User::OnlineStatus);

    emit userDataChanged(userId());
    return true;
  }
  else if (data.status == AuthReplyData::AccessDenied) {
    if (data.error == AuthReplyData::NickAlreadyUse) {
      m_user->setNick(mangleNick());
      requestAuth();
    }
  }

  return false;
}


/*!
 * Чтение пакета Packet::JoinReply.
 *
 * Обработка подключения к каналу, в случае успеха канал добавляется в таблицу каналов.
 */
bool SimpleClient::readChannel()
{
  ChannelReader reader(m_reader);

  SCHAT_DEBUG_STREAM(this << "readChannel()" << reader.channel->id().toHex())

  if (!reader.channel->isValid())
    return false;

  addChannel(reader.channel);
  return true;
}


/*!
 * Чтение сообщения и обработка поддерживаемых команд.
 * В случае если сообщении не содержало команд или команда
 * не была обработана, высылается сигнал о новом сообщении.
 */
bool SimpleClient::readMessage()
{
  SCHAT_DEBUG_STREAM(this << "readMessage()")

  MessageReader reader(m_reader);
  m_messageData = &reader.data;
  m_messageData->timestamp = timestamp();

  if (m_messageData->options & MessageData::ControlOption && command()) {
    return true;
  }

  SCHAT_DEBUG_STREAM("      " << reader.data.text)

  emit message(reader.data);
  return true;
}


bool SimpleClient::readNotice()
{
  SCHAT_DEBUG_STREAM(this << "readNotice()")

  NoticeReader reader(m_reader);
  reader.data.timestamp = timestamp();
  emit notice(reader.data);

  return true;
}


/*!
 * Чтение пакета Packet::UserData.
 *
 * В случае если адрес назначения канал, то требуется чтобы текущий пользователь был подключен к нему заранее,
 * также будет произведена попытка добавления пользователя в него и в случае успеха будет выслан сигнал.
 * Идентификатор нового пользователя не может быть пустым и в случае если новый пользователь отсутствует
 * в таблице пользователей, то будет произведена попытка его добавить.
 */
bool SimpleClient::readUserData()
{
  SCHAT_DEBUG_STREAM(this << "readUserData()")

  QByteArray dest = m_reader->dest();
  int type = SimpleID::typeOf(dest);

  /// Если идентификатор назначения канал, то это канал должен быть известен клиенту.
  if (type == SimpleID::ChannelId && !m_channels.contains(dest))
    return false;

  /// Идентификатор отправителя не может быть пустым.
  QByteArray id = m_reader->sender();
  if (id.isEmpty())
    return false;

  UserReader reader(m_reader);
  if (!reader.user.isValid())
    return false;

  /// Если пользователь не существует, то он будет создан, иначе произойдёт обновление данных,
  /// В обоих случаях будет выслан сигнал userDataChanged().
  ClientUser user = m_users.value(id);
  if (!user) {
    user = ClientUser(new User(&reader.user));
    m_users.insert(id, user);

    emit userDataChanged(id);
  }
  else {
    updateUserData(user, &reader.user);
  }

  /// Если идентификатор назначения канал, то пользователь будет добавлен в него.
  if (type == SimpleID::ChannelId) {
    Channel *chan = m_channels.value(dest);
    if (!chan)
      return false;

    user->addId(SimpleID::ChannelListId, dest);
    chan->addUser(id);

    if (m_syncChannelCache)
      m_syncChannelCache->users.append(id);
    else
      emit join(dest, id);
  }
  else if (type == SimpleID::UserId) {
    this->user()->addId(SimpleID::TalksListId, id);
  }

  return true;
}


/*!
 * Удаление пользователя.
 */
bool SimpleClient::removeUser(const QByteArray &userId)
{
  ClientUser user = m_users.value(userId);
  if (!user)
    return false;

  QList<QByteArray> channels = user->ids(SimpleID::ChannelListId);
  for (int i = 0; i < channels.size(); ++i) {
    removeUserFromChannel(channels.at(i), userId, false);
  }

  m_users.remove(userId);
  emit userLeave(userId);
  return true;
}


/*!
 * Удаление пользователя из канала.
 */
bool SimpleClient::removeUserFromChannel(const QByteArray &channelId, const QByteArray &userId, bool clear)
{
  ClientUser user = m_users.value(userId);
  Channel *channel = m_channels.value(channelId);

  if (!user.isNull() && channel) {
    channel->removeUser(user->id());
    user->removeId(SimpleID::ChannelListId, channel->id());
    emit part(channel->id(), user->id());

    if (clear && !this->user()->containsId(SimpleID::TalksListId, userId) && user->count(SimpleID::ChannelListId) == 0)
      removeUser(userId);

    return true;
  }

  return false;
}


/*!
 * Обновление информации о пользователе.
 */
void SimpleClient::updateUserData(ClientUser existUser, User *user)
{
  SCHAT_DEBUG_STREAM(this << "updateUserData()");

  if (existUser == m_user && m_user->nick() != user->nick()) {
    setNick(user->nick());
  }

  existUser->setNick(user->nick());
  existUser->setRawGender(user->rawGender());
  existUser->setStatus(user->status());

  emit userDataChanged(existUser->id());
}


/*!
 * Обновление статуса пользователя.
 */
void SimpleClient::updateUserStatus(const QString &text)
{
  ClientUser user = this->user(m_reader->sender());
  if (!user)
    return;

  if (!user->setStatus(text))
    return;

  if (user->status() == User::OfflineStatus) {
    if (m_reader->headerOption() & Protocol::Broadcast) {
      user->setStatus(User::OnlineStatus);
    }
  }

  emit userDataChanged(user->id());
}
