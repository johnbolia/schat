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

#include <QDebug>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QtPlugin>
#include <QWebFrame>
#include <QHostAddress>
#include <QFileDialog>

#include "ChatCore.h"
#include "ChatSettings.h"
#include "client/ChatClient.h"
#include "client/SimpleClient.h"
#include "DateTime.h"
#include "hooks/ChatViewHooks.h"
#include "messages/Message.h"
#include "net/packets/MessageNotice.h"
#include "net/SimpleID.h"
#include "SendFileMessages.h"
#include "SendFilePlugin.h"
#include "SendFilePlugin_p.h"
#include "SendFileThread.h"
#include "sglobal.h"
#include "Tr.h"
#include "Translation.h"
#include "ui/tabs/ChatView.h"
#include "ui/TabWidget.h"
#include "WebBridge.h"

class SendFileTr : public Tr
{
  Q_DECLARE_TR_FUNCTIONS(WebBridgeTr)

public:
  SendFileTr() : Tr() { m_prefix = LS("file-"); }

protected:
  QString valueImpl(const QString &key) const
  {
    if (key == LS("waiting"))         return tr("Waiting");
    else if (key == LS("cancel"))     return tr("Cancel");
    else if (key == LS("cancelled"))  return tr("Cancelled");
    else if (key == LS("saveas"))     return tr("Save as");
    else if (key == LS("connecting")) return tr("Connecting...");
    else if (key == LS("sent"))       return tr("File sent");
    return QString();
  }
};


SendFilePluginImpl::SendFilePluginImpl(QObject *parent)
  : ChatPlugin(parent)
  , m_port(0)
{
  ChatCore::settings()->setLocalDefault(LS("SendFile/Port"), 0);
  ChatCore::settings()->setLocalDefault(LS("SendFile/Dir"), QDir::fromNativeSeparators(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));

  m_port = getPort();

  m_tr = new SendFileTr();
  new SendFileMessages(this);

  ChatCore::translation()->addOther(LS("sendfile"));
  QDesktopServices::setUrlHandler(LS("chat-sendfile"), this, "openUrl");

  m_thread = new SendFile::Thread(m_port);
  connect(m_thread, SIGNAL(finished(QByteArray, qint64)), SLOT(finished(QByteArray)));
  connect(m_thread, SIGNAL(progress(QByteArray, qint64, qint64, int)), SLOT(progress(QByteArray, qint64, qint64, int)));
  connect(m_thread, SIGNAL(started(QByteArray, qint64)), SLOT(started(QByteArray)));

  connect(ChatViewHooks::i(), SIGNAL(initHook(ChatView*)), SLOT(init(ChatView*)));
  connect(ChatViewHooks::i(), SIGNAL(loadFinishedHook(ChatView*)), SLOT(loadFinished(ChatView*)));
}


SendFilePluginImpl::~SendFilePluginImpl()
{
  delete m_tr;
}


/*!
 * Отправка одиночного файла.
 *
 * \param dest Идентификатор получателя.
 * \param file Файл, который будет отправлен.
 */
bool SendFilePluginImpl::sendFile(const QByteArray &dest, const QString &file)
{
  if (SimpleID::typeOf(dest) != SimpleID::UserId)
    return false;

  SendFileTransaction transaction(new SendFile::Transaction(dest, ChatCore::randomId(), file));
  if (!transaction->isValid())
    return false;

  transaction->setLocal(localHosts());

  MessagePacket packet(new MessageNotice(ChatClient::id(), dest, LS("file"), DateTime::utc(), transaction->id()));
  packet->setCommand(packet->text());
  packet->setData(transaction->toReceiver());

  if (ChatClient::io()->send(packet, true)) {
    m_thread->add(transaction);

    Message message(transaction->id(), dest, LS("file"), LS("addFileMessage"));
    message.setAuthor(ChatClient::id());
    message.setDate();
    message.data()[LS("File")]      = transaction->fileName();
    message.data()[LS("Direction")] = LS("outgoing");
    TabWidget::add(message);

    m_transactions[transaction->id()] = transaction;
    return true;
  }

  return false;
}


/*!
 * Чтение входящего пакета.
 */
void SendFilePluginImpl::read(const MessagePacket &packet)
{
  qDebug() << "----------------";
  qDebug() << "command:" << packet->command();
  qDebug() << "text:   " << packet->text();
  qDebug() << "json:   " << packet->raw();
  qDebug() << "----------------";
  if (packet->text() == LS("file"))
    incomingFile(packet);
  else if (packet->text() == LS("cancel"))
    cancel(packet);
  else if (packet->text() == LS("accept"))
    accept(packet);
}


void SendFilePluginImpl::init(ChatView *view)
{
  if (SimpleID::typeOf(view->id()) != SimpleID::UserId)
    return;

  view->addJS(LS("qrc:/js/SendFile/SendFile.js"));
  view->page()->mainFrame()->addToJavaScriptWindowObject(LS("SendFile"), this);
}


void SendFilePluginImpl::loadFinished(ChatView *view)
{
  if (SimpleID::typeOf(view->id()) == SimpleID::UserId)
    view->addCSS(LS("qrc:/css/SendFile/SendFile.css"));
}


void SendFilePluginImpl::openUrl(const QUrl &url)
{
  QStringList path = url.path().split(LC('/'));
  if (path.size() < 2)
    return;

  QString action = path.at(0);
  QByteArray id = SimpleID::decode(path.at(1));
  if (SimpleID::typeOf(id) != SimpleID::MessageId)
    return;

  if (action == LS("cancel"))
    cancel(id);
  else if (action == LS("saveas"))
    saveAs(id);
}


void SendFilePluginImpl::finished(const QByteArray &id)
{
  SendFileTransaction transaction = m_transactions.value(id);
  if (!transaction)
    return;

  if (transaction->role() == SendFile::SenderRole)
    emit sent(SimpleID::encode(id));
}


void SendFilePluginImpl::progress(const QByteArray &id, qint64 current, qint64 total, int percent)
{
  SendFileTransaction transaction = m_transactions.value(id);
  if (!transaction)
    return;

  emit progress(SimpleID::encode(id), tr("%1 of %2").arg(WebBridge::i()->bytesToHuman(current), WebBridge::i()->bytesToHuman(total)), percent);
}


void SendFilePluginImpl::started(const QByteArray &id)
{
  SendFileTransaction transaction = m_transactions.value(id);
  if (!transaction)
    return;

  progress(id, 0, transaction->file().size, 0);
}


MessagePacket SendFilePluginImpl::reply(const SendFileTransaction &transaction, const QString &text)
{
  MessagePacket packet(new MessageNotice(ChatClient::id(), transaction->user(), text, DateTime::utc(), transaction->id()));
  packet->setCommand(LS("file"));
  packet->setDirection(Notice::Internal);
  return packet;
}


/*!
 * Определение порта для передачи файлов, если порт не определён в настройках, используется случайный порт в диапазоне от 49152 до 65536.
 */
quint16 SendFilePluginImpl::getPort() const
{
  QString key = LS("SendFile/Port");
  quint16 port = SCHAT_OPTION(key).toInt();

  if (!port) {
    qrand();
    port = qrand() % 16383 + 49152;
    ChatCore::settings()->setValue(key, port);
  }

  return port;
}


SendFile::Hosts SendFilePluginImpl::localHosts() const
{
  return SendFile::Hosts(ChatClient::io()->json().value(LS("host")).toString(), m_port, ChatClient::io()->localAddress().toString(), m_port);
}


/*!
 * Принятие удалённой стороной входящего файла.
 * В поле \b hosts передаются адреса и порты удалённой стороны.
 */
void SendFilePluginImpl::accept(const MessagePacket &packet)
{
  SendFileTransaction transaction = m_transactions.value(packet->id());
  if (!transaction || transaction->role() != SendFile::SenderRole)
    return;

  SendFile::Hosts hosts(packet->json().value(LS("hosts")).toList());
  if (!hosts.isValid())
    return;

  transaction->setRemote(hosts);
  m_thread->add(transaction);
  emit accepted(SimpleID::encode(transaction->id()), transaction->fileName());
}


/*!
 * Обработка отмены передачи файла вызванной удалённым клиентом.
 */
void SendFilePluginImpl::cancel(const MessagePacket &packet)
{
  SendFileTransaction transaction = m_transactions.value(packet->id());
  if (!transaction)
    return;

  m_transactions.remove(packet->id());
  emit cancelled(SimpleID::encode(packet->id()));
}


/*!
 * Обработка нового входящего файла.
 */
void SendFilePluginImpl::incomingFile(const MessagePacket &packet)
{
  if (m_transactions.contains(packet->id()))
    return;

  SendFileTransaction transaction(new SendFile::Transaction(packet->sender(), packet->id(), packet->json()));
  if (!transaction->isValid())
    return;

  transaction->setLocal(localHosts());

  Message message(packet->id(), packet->sender(), LS("file"), LS("addFileMessage"));
  message.setAuthor(packet->sender());
  message.setDate();
  message.data()[LS("File")]      = transaction->fileName();
  message.data()[LS("Size")]      = transaction->file().size;
  message.data()[LS("Direction")] = LS("incoming");
  TabWidget::add(message);

  m_transactions[transaction->id()] = transaction;
}


/*!
 * Обработка локально вызванной отмены передачи файла.
 */
void SendFilePluginImpl::cancel(const QByteArray &id)
{
  SendFileTransaction transaction = m_transactions.value(id);
  if (!transaction)
    return;

  m_transactions.remove(id);
  ChatClient::io()->send(reply(transaction, LS("cancel")), true);

  emit cancelled(SimpleID::encode(id));
}


/*!
 * Выбор места сохранения файла.
 */
void SendFilePluginImpl::saveAs(const QByteArray &id)
{
  SendFileTransaction transaction = m_transactions.value(id);
  if (!transaction)
    return;

  QDir dir(SCHAT_OPTION(LS("SendFile/Dir")).toString());
  if (!dir.exists())
    dir.setPath(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));

  QString fileName = dir.path() + LC('/') + transaction->fileName();
  fileName = QFileDialog::getSaveFileName(TabWidget::i(), tr("Save"), fileName, LS("*.") + QFileInfo(fileName).suffix() + LS(";;*.*"));
  if (fileName.isEmpty())
    return;

  transaction->saveAs(fileName);
  m_thread->add(transaction);

  ChatCore::settings()->setValue(LS("SendFile/Dir"), QFileInfo(fileName).absolutePath());
  emit accepted(SimpleID::encode(transaction->id()), QFileInfo(fileName).fileName());

  MessagePacket packet = reply(transaction, LS("accept"));
  QVariantMap data;
  data[LS("hosts")] = transaction->local().toJSON();
  packet->setData(data);
  ChatClient::io()->send(packet, true);
}


ChatPlugin *SendFilePlugin::create()
{
  m_plugin = new SendFilePluginImpl(this);
  return m_plugin;
}

Q_EXPORT_PLUGIN2(SendFile, SendFilePlugin);
