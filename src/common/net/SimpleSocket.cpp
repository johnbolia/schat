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

#define SCHAT_DISABLE_DEBUG

#include <QBasicTimer>
#include <QSslConfiguration>
#include <QSslKey>
#include <QTimerEvent>

#include "debugstream.h"
#include "net/PacketReader.h"
#include "net/PacketWriter.h"
#include "net/SimpleSocket.h"
#include "net/SimpleSocket_p.h"
#include "net/TransportReader.h"
#include "net/TransportWriter.h"

SimpleSocketPrivate::SimpleSocketPrivate()
  : authorized(false)
  , release(false)
  , serverSide(false)
  , sslAvailable(false)
  , rxStream(0)
  , date(0)
  , nextBlockSize(0)
  , id(0)
  , rx(0)
  , rxSeq(0)
  , tx(0)
  , txSeq(0)
{
  timer = new QBasicTimer();
  txStream = new QDataStream(&txBuffer, QIODevice::ReadWrite);
  sendStream = new QDataStream(&sendBuffer, QIODevice::ReadWrite);
  readStream = new QDataStream(&readBuffer, QIODevice::ReadWrite);
}


SimpleSocketPrivate::~SimpleSocketPrivate()
{
  qDebug() << "~SimpleSocketPrivate()";

  if (timer->isActive())
    timer->stop();

  delete timer;
  delete txStream;
  delete sendStream;
  delete readStream;

  if (rxStream)
    delete rxStream;
}


/*!
 * Чтение транспортного пакета.
 */
bool SimpleSocketPrivate::readTransport()
{
  SCHAT_DEBUG_STREAM(this << "readTransport()")

  Q_Q(SimpleSocket);

  rx += nextBlockSize + 4;
  TransportReader reader(nextBlockSize, rxStream);
  int type = reader.readHeader();
  nextBlockSize = reader.available();

  if (type == Protocol::GenericTransport) {
    rxSeq = reader.sequence();

    int options = reader.options();
    if (options & Protocol::TimeStamp)
      date = reader.timestamp();

    // Чтение служебного транспортного пакета.
    if (options & Protocol::ContainsInternalPacket) {
      if (serverSide)
        setTimerState(Idling);

      if (nextBlockSize == 0) {
        if (serverSide)
          transmit(QByteArray(), Protocol::ContainsInternalPacket);
        else
          setTimerState(Idling);
      }

      readBuffer = reader.readOne();
      PacketReader packet(readStream);

     // Подтверждение доставки.
      if (!serverSide && packet.type() == Protocol::DeliveryConfirmationPacket) {
        setTimerState(Idling);
        packet.get<quint16>();
        QList<quint64> list = packet.get<QList<quint64> >();

        if (!list.isEmpty()) {
          foreach (quint64 key, list) {
            deliveryConfirm.removeAll(key);
          }

          if (deliveryConfirm.isEmpty())
            emit(q->allDelivered(id));
        }

        SCHAT_DEBUG_STREAM(this << "DeliveryConfirmation" << list << "r. size:" << deliveryConfirm.size());
      }
      else if (packet.type() == Protocol::ProbeSecureConnectionPacket) {
        sslHandshake(packet.get<quint16>());
      }

      reader.skipAll();
      return true;
    }

    /// Из стандартного транспортного пакета, виртуальные пакеты извлекаются в очередь \p m_readQueue,
    /// и если это серверный сокет sequence транспортного пакета будет добавлен в \p m_deliveryConfirm.
    readQueue += reader.read();

    if (serverSide)
      deliveryConfirm += rxSeq;

    return true;
  }

  return false;
}


bool SimpleSocketPrivate::transmit(const QByteArray &packet, quint8 options, quint8 type, quint8 subversion)
{
  return transmit(QList<QByteArray>() += packet, options, type, subversion);
}


/*!
 * Низкоуровневая функция отправки данных, виртуальные пакеты автоматически упаковываются в транспортный пакет.
 *
 * \param packets    Список виртуальных пакетов для отправки.
 * \param options    packet options.
 * \param type       packet type.
 * \param subversion packet subversion.
 *
 * \return true если данные были записаны в сокет.
 */
bool SimpleSocketPrivate::transmit(const QList<QByteArray> &packets, quint8 options, quint8 type, quint8 subversion)
{
  Q_Q(SimpleSocket);
  SCHAT_DEBUG_STREAM(this << "transmit(...)")
  SCHAT_DEBUG_STREAM(this << "  >> seq:" << txSeq << "opt:" << options << "type:" << type << "sv:" << subversion << "r. size:" << deliveryConfirm.size());

  if (!q->isReady())
    return false;

  if (packets.isEmpty())
    return false;

  qint64 ts = 0;
  if (serverSide && options != Protocol::ContainsInternalPacket)
    ts = date;

  TransportWriter tp(txStream, packets, txSeq, ts, options, type, subversion);
  QByteArray packet = tp.data();

  if (!serverSide && options != Protocol::ContainsInternalPacket) {
    deliveryConfirm.append(txSeq);
    txSeq++;
    setTimerState(WaitingReply);
  }

  int size = q->write(packet);
  if (size == -1)
    return false;

  tx += size;
  SCHAT_DEBUG_STREAM(this << "  >> Transmit:" << size << "Total:" << tx << "Receive:" << rx)
  return true;
}


void SimpleSocketPrivate::releaseSocket()
{
  if (release)
    return;

  Q_Q(SimpleSocket);
  release = true;

  if (timerState == Leaving) {
    q->setErrorString(SimpleSocket::tr("Time out"));
  }

  if (timer->isActive())
    timer->stop();

  date = 0;
  sendBuffer.clear();
  readBuffer.clear();
  txBuffer.clear();
  nextBlockSize = 0;
  deliveryConfirm.clear();

  emit(q->released(id));
}


/*!
 * Установка состояния таймера.
 */
void SimpleSocketPrivate::setTimerState(TimerState state)
{
  SCHAT_DEBUG_STREAM(this << "setTimerState()" << state);

  Q_Q(SimpleSocket);

  if ((state == Idling || state == WaitingReply) && !authorized)
    return;

  timerState = state;
  if (timer->isActive())
    timer->stop();

  switch (state) {
    case WaitingConnect:
      timer->start(Protocol::ConnectTime, q);
      break;

    case WaitingHandshake:
      timer->start(Protocol::HandshakeTime, q);
      break;

    case Idling:
      if (authorized) {
        if (serverSide)
          timer->start(Protocol::MaxServerIdleTime, q);
        else
          timer->start(Protocol::IdleTime, q);
      }
      break;

    case WaitingReply:
      timer->start(Protocol::ReplyTime, q);
      break;

    case Leaving:
      break;
  }
}


/*!
 * Обработка запроса клиента на шифрованное подключение,
 * в случае поддержкой сервером шифрования, он будет установлено.
 */
void SimpleSocketPrivate::sslHandshake(int option)
{
  Q_Q(SimpleSocket);
  SCHAT_DEBUG_STREAM("SimpleSocketPrivate::sslHandshake")

  if (option == Protocol::SecureConnectionRequest && serverSide) {
    quint16 answer = Protocol::SecureConnectionUnavailable;
    if (sslAvailable)
      answer = Protocol::SecureConnectionAvailable;

    PacketWriter writer(sendStream, Protocol::ProbeSecureConnectionPacket);
    writer.put(answer);
    transmit(writer.data(), Protocol::ContainsInternalPacket);

    #if !defined(SCHAT_NO_SSL)
    if (sslAvailable)
      q->startServerEncryption();
    #endif
  }
  else if (option == Protocol::SecureConnectionAvailable && !serverSide) {
    #if !defined(SCHAT_NO_SSL)
    q->startClientEncryption();
    #endif
  }
  else if (option == Protocol::SecureConnectionUnavailable && !serverSide) {
    emit(q->requestAuth(id));
  }
}


void SimpleSocketPrivate::timerEvent()
{
  SCHAT_DEBUG_STREAM(this << "timerEvent()" << timerState)

  Q_Q(SimpleSocket);

  if (timerState == WaitingConnect || (timerState == WaitingHandshake && !authorized) || (timerState == Idling && serverSide) || timerState == WaitingReply) {
    setTimerState(Leaving);
    q->leave();
    return;
  }

  // Отправка пустого транспортного пакета, сервер должен ответить на него таким же пустым пакетом.
  transmit(QByteArray(), Protocol::ContainsInternalPacket);
  setTimerState(WaitingReply);
  return;
}




SimpleSocket::SimpleSocket(QObject *parent)
  : QSslSocket(parent)
  , d_ptr(new SimpleSocketPrivate())
{
  SCHAT_DEBUG_STREAM(this)

  Q_D(SimpleSocket);
  d->q_ptr = this;
  d->rxStream = new QDataStream(this);

  connect(this, SIGNAL(connected()), SLOT(connected()));
  connect(this, SIGNAL(disconnected()), SLOT(disconnected()));
  connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
  connect(this, SIGNAL(readyRead()), SLOT(readyRead()));

  #if !defined(SCHAT_NO_SSL)
  setProtocol(QSsl::TlsV1);
  d->sslAvailable = QSslSocket::supportsSsl();
  connect(this, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
  connect(this, SIGNAL(encrypted()), SLOT(encrypted()));
  #endif
}


SimpleSocket::SimpleSocket(SimpleSocketPrivate &dd, QObject *parent)
  : QSslSocket(parent)
  , d_ptr(&dd)
{
  SCHAT_DEBUG_STREAM("SimpleSocketPrivate" << this)

  Q_D(SimpleSocket);
  d->q_ptr = this;
  d->rxStream = new QDataStream(this);

  connect(this, SIGNAL(connected()), SLOT(connected()));
  connect(this, SIGNAL(disconnected()), SLOT(disconnected()));
  connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
  connect(this, SIGNAL(readyRead()), SLOT(readyRead()));

  #if !defined(SCHAT_NO_SSL)
  d->sslAvailable = QSslSocket::supportsSsl();
  setProtocol(QSsl::TlsV1);
  connect(this, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
  connect(this, SIGNAL(encrypted()), SLOT(encrypted()));
  #endif
}


SimpleSocket::~SimpleSocket()
{
  SCHAT_DEBUG_STREAM("~SimpleSocket" << this)
  qDebug() << "                           ~SimpleSocket";
  delete d_ptr;
}


bool SimpleSocket::isAuthorized() const
{
  Q_D(const SimpleSocket);
  return d->authorized;
}


bool SimpleSocket::send(const QByteArray &packet)
{
  SCHAT_DEBUG_STREAM(this << "send(...)")

  Q_D(SimpleSocket);

  if (packet.size() > 65535)
    return d->transmit(packet, Protocol::HugePackets);

  return d->transmit(packet);
}


/*!
 * Отправка виртуальных пакетов.
 */
bool SimpleSocket::send(const QList<QByteArray> &packets)
{
  SCHAT_DEBUG_STREAM(this << "send(...)" << packets.size());

  bool huge = false;
  for (int i = 0; i < packets.size(); ++i) {
    if (packets.at(i).size() > 65535) {
      huge = true;
      break;
    }
  }

  Q_D(SimpleSocket);

  if (huge)
    return d->transmit(packets, Protocol::HugePackets);

  return d->transmit(packets);
}


/*!
 * Установка дескриптора сокета, данная функция вызывается только для серверного сокета.
 */
bool SimpleSocket::setSocketDescriptor(int socketDescriptor)
{
  Q_D(SimpleSocket);

  if (QSslSocket::setSocketDescriptor(socketDescriptor)) {
    d->serverSide = true;
    #if QT_VERSION >= 0x040600
    setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    #endif
    d->setTimerState(SimpleSocketPrivate::WaitingHandshake);

    #if !defined(SCHAT_NO_SSL)
    if (d->sslAvailable) {
      setSslConfiguration(QSslConfiguration::defaultConfiguration());
      if (sslConfiguration().localCertificate().isNull() || sslConfiguration().privateKey().isNull()) {
        d->sslAvailable = false;
      }
    }
    #endif

    return true;
  }

  return false;
}


const QByteArray& SimpleSocket::channelId() const
{
  Q_D(const SimpleSocket);
  return d->channelId;
}


QByteArray SimpleSocket::readBuffer() const
{
  return d_func()->readBuffer;
}


QDataStream *SimpleSocket::sendStream()
{
  return d_func()->sendStream;
}


qint64 SimpleSocket::date() const
{
  return d_func()->date;
}


quint64 SimpleSocket::id() const
{
  return d_func()->id;
}


quint64 SimpleSocket::rx() const
{
  return d_func()->rx;
}


quint64 SimpleSocket::tx() const
{
  return d_func()->tx;
}


void SimpleSocket::leave()
{
  SCHAT_DEBUG_STREAM(this << "leave()")

  Q_D(SimpleSocket);

  if (state() == SimpleSocket::ConnectedState) {
    flush();
    disconnectFromHost();
    if (!d->serverSide && state() != SimpleSocket::UnconnectedState) {
      if (!waitForDisconnected(1000))
        abort();
    }
  }
  else {
    abort();
    d->releaseSocket();
  }
}


void SimpleSocket::setAuthorized(const QByteArray &userId)
{
  Q_D(SimpleSocket);

  if (userId.isEmpty()) {
    d->authorized = false;
    return;
  }

  d->channelId = userId;
  d->authorized = true;
  d->setTimerState(SimpleSocketPrivate::Idling);
}


void SimpleSocket::setId(quint64 id)
{
  d_func()->id = id;
}


#if QT_VERSION >= 0x050000
void SimpleSocket::connectToHost(const QString & hostName, quint16 port, OpenMode openMode, NetworkLayerProtocol protocol)
{
  connectToHostImplementation(hostName, port, openMode);
  QSslSocket::connectToHost(hostName, port, openMode, protocol);
}
#endif


void SimpleSocket::setDate(qint64 date)
{
  d_func()->date = date;
}


void SimpleSocket::newPacketsImpl()
{
  Q_D(SimpleSocket);
  emit newPackets(d->id, d->readQueue);
}


void SimpleSocket::timerEvent(QTimerEvent *event)
{
  Q_D(SimpleSocket);

  if (event->timerId() == d->timer->timerId()) {
    d->timerEvent();
  }

  QSslSocket::timerEvent(event);
}


void SimpleSocket::connectToHostImplementation(const QString &hostName, quint16 port, OpenMode openMode)
{
  SCHAT_DEBUG_STREAM(this << "connectToHostImplementation()" << hostName << port)

  Q_D(SimpleSocket);

  d->serverSide = false;
  d->release = false;
  d->setTimerState(SimpleSocketPrivate::WaitingConnect);

# if QT_VERSION < 0x050000
  QSslSocket::connectToHostImplementation(hostName, port, openMode);
# else
  Q_UNUSED(openMode)
# endif
}


void SimpleSocket::connected()
{
  SCHAT_DEBUG_STREAM(this << "connected()")

  Q_D(SimpleSocket);

  #if QT_VERSION >= 0x040600
  setSocketOption(QAbstractSocket::KeepAliveOption, 1);
  #endif
  d->setTimerState(SimpleSocketPrivate::WaitingHandshake);

  if (d->sslAvailable) {
    PacketWriter writer(d->sendStream, Protocol::ProbeSecureConnectionPacket);
    writer.put<quint16>(Protocol::SecureConnectionRequest);
    d->transmit(writer.data(), Protocol::ContainsInternalPacket);
  }
  else {
    emit requestAuth(d->id);
  }
}


void SimpleSocket::disconnected()
{
  SCHAT_DEBUG_STREAM(this << "disconnected()" << state())
  d_func()->releaseSocket();
}


void SimpleSocket::error(QAbstractSocket::SocketError socketError)
{
  SCHAT_DEBUG_STREAM(this << "error()" << socketError << state())

  Q_UNUSED(socketError)
  Q_D(SimpleSocket);

  if (state() != QSslSocket::ConnectedState)
    d->releaseSocket();
}


/*!
 * Слот вызывается при получении новых данных.
 * Если данных достаточно для чтения транспортного пакета происходит чтение пакета.
 *
 * \sa readTransport()
 */
void SimpleSocket::readyRead()
{
  SCHAT_DEBUG_STREAM(this << "readyRead()" << bytesAvailable())

  Q_D(SimpleSocket);

  forever {
    if (!d->nextBlockSize) {
      if (bytesAvailable() < 4)
        break;

      *d->rxStream >> d->nextBlockSize;
      if (!d->authorized && d->nextBlockSize == 1195725856) { // Обработка HTTP GET.
        disconnectFromHost();
      }
    }

    if (bytesAvailable() < d->nextBlockSize)
      break;

    if (!d->readTransport())
      read(d->nextBlockSize);

    d->nextBlockSize = 0;
  }

  if (d->readQueue.isEmpty())
    return;

  if (d->serverSide)
    d->setTimerState(SimpleSocketPrivate::Idling);

  // Отправка подтверждения о доставке.
  if (d->serverSide && !d->deliveryConfirm.isEmpty()) {
    PacketWriter writer(d->sendStream, Protocol::DeliveryConfirmationPacket);
    writer.put<quint16>(0);
    writer.put(d->deliveryConfirm);
    d->transmit(writer.data(), Protocol::ContainsInternalPacket);
    d->deliveryConfirm.clear();
  }

  newPacketsImpl();
  d->readQueue.clear();
}


#if !defined(SCHAT_NO_SSL)
void SimpleSocket::encrypted()
{
  SCHAT_DEBUG_STREAM(this << "encrypted()")
  Q_D(SimpleSocket);
  emit requestAuth(d->id);
}


void SimpleSocket::sslErrors(const QList<QSslError> &errors)
{
  SCHAT_DEBUG_STREAM(this << "sslErrors()" << errors)

  QList<QSslError::SslError> noCriticalErrors;
  noCriticalErrors.append(QSslError::NoError);
  noCriticalErrors.append(QSslError::SelfSignedCertificate);        // The passed certificate is self signed and the same certificate cannot be found in the list of trusted certificates.
  noCriticalErrors.append(QSslError::SelfSignedCertificateInChain); // The certificate chain could be built up using the untrusted certificates but the root could not be found locally.
  noCriticalErrors.append(QSslError::CertificateExpired);           // The certificate has expired: that is the notAfter date is before the current time.
  noCriticalErrors.append(QSslError::CertificateNotYetValid);       // The certificate is not yet valid: the notBefore date is after the current time.
  noCriticalErrors.append(QSslError::HostNameMismatch);

  for (int i = 0; i < errors.size(); ++i) {
    if (!noCriticalErrors.contains(errors.at(i).error()))
      return;
  }

  ignoreSslErrors();
}
#endif
