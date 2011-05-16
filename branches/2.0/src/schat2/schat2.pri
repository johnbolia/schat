# $Id$
# IMPOMEZIA Simple Chat
# Copyright (c) 2008-2011 IMPOMEZIA <schat@impomezia.com>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see <http://www.gnu.org/licenses/>.

SCHAT_RESOURCES   = 1
SCHAT_RC_FILE     = 1
SCHAT_SINGLEAPP   = 0

QT = core gui network webkit
TEMPLATE = app

HEADERS = \
    arora/lineedit.h \
    arora/lineedit_p.h \
    Channel.h \
    ChatCore.h \
    ChatMessage.h \
    ClientUI.h \
    debugstream.h \
    MessageAdapter.h \
    net/PacketReader.h \
    net/packets/auth.h \
    net/packets/channels.h \
    net/packets/message.h \
    net/packets/users.h \
    net/PacketWriter.h \
    net/SimpleClient.h \
    net/SimpleClient_p.h \
    net/SimpleID.h \
    net/SimpleSocket.h \
    net/TransportReader.h \
    net/TransportWriter.h \
    QProgressIndicator/QProgressIndicator.h \
    qtwin/qtwin.h \
    ui/NickEdit.h \
    ui/SoundButton.h \
    ui/StatusBar.h \
    ui/TabBar.h \
    ui/tabs/AbstractTab.h \
    ui/tabs/ChannelTab.h \
    ui/tabs/ChatView.h \
    ui/tabs/ChatView_p.h \
    ui/tabs/ChatViewTab.h \
    ui/tabs/PrivateTab.h \
    ui/tabs/UserView.h \
    ui/tabs/WelcomeTab.h \
    ui/TabWidget.h \
    ui/UserUtils.h \
    User.h \

SOURCES = \
    arora/lineedit.cpp \
    Channel.cpp \
    ChatCore.cpp \
    ChatMessage.cpp \
    ClientUI.cpp \
    main.cpp \
    MessageAdapter.cpp \
    net/PacketReader.cpp \
    net/packets/auth.cpp \
    net/packets/channels.cpp \
    net/packets/message.cpp \
    net/packets/users.cpp \
    net/SimpleClient.cpp \
    net/SimpleID.cpp \
    net/SimpleSocket.cpp \
    net/TransportReader.cpp \
    net/TransportWriter.cpp \
    QProgressIndicator/QProgressIndicator.cpp \
    qtwin/qtwin.cpp \
    ui/NickEdit.cpp \
    ui/SoundButton.cpp \
    ui/StatusBar.cpp \
    ui/TabBar.cpp \
    ui/tabs/AbstractTab.cpp \
    ui/tabs/ChannelTab.cpp \
    ui/tabs/ChatView.cpp \
    ui/tabs/ChatViewTab.cpp \
    ui/tabs/PrivateTab.cpp \
    ui/tabs/UserView.cpp \
    ui/tabs/WelcomeTab.cpp \
    ui/TabWidget.cpp \
    ui/UserUtils.cpp \
    User.cpp \
    
TRANSLATIONS += ../../data/translations/client_en.ts
TRANSLATIONS += ../../data/translations/client_ru.ts
CODECFORTR = UTF-8

include(../common/common.pri)

