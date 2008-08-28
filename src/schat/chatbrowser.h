/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008 IMPOMEZIA (http://impomezia.net.ru)
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CHATBROWSER_H_
#define CHATBROWSER_H_

#include <QTextBrowser>
#include <QTime>

#include "animatedsmile.h"
#include "channellog.h"

class Settings;

class ChatBrowser : public QTextBrowser {
  Q_OBJECT

public:
  ChatBrowser(Settings *settings, QWidget *parent = 0);
  inline QString channel() const                 { return m_channelLog->channel(); }
  inline void setChannel(const QString &channel) { m_channelLog->setChannel(channel); }
  static inline QString currentTime()            { return QTime::currentTime().toString("hh:mm:ss"); }
  void msg(const QString &text);
  void msgBadNickName(const QString &nick);
  void msgChangedNick(quint16 sex, const QString &oldNick, const QString &newNick);
  void msgDisconnect();
  void msgNewParticipant(quint8 sex, const QString &nick);
  void msgOldClientProtocol();
  void msgOldServerProtocol();
  void msgParticipantLeft(quint8 sex, const QString &nick, const QString &bye);
  void msgReadyForUse(const QString &addr);
  void msgReadyForUse(const QString &network, const QString &addr);
  void scroll();

protected:
  bool viewportEvent(QEvent *event);
  void contextMenuEvent(QContextMenuEvent *event);
  void hideEvent(QHideEvent* /*event*/) { playPauseAnimations(false); }
  void showEvent(QShowEvent* /*event*/) { playPauseAnimations(true); }

public slots:
  void msgNewMessage(const QString &nick, const QString &message);

private slots:
  void playPauseAnimations(bool play);
  void setAnimations();

private:
  ChannelLog *m_channelLog;
  int m_keepAnimations;
  QList<AnimatedSmile*> m_animatedSmiles;
  QString m_style;
  Settings *m_settings;
};

#endif /*CHATBROWSER_H_*/
