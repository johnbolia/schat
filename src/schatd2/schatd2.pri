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

SCHAT_RESOURCES   = 0
SCHAT_RC_FILE     = 1
SCHAT_SINGLEAPP   = 0

QT = core gui network
TEMPLATE = app

HEADERS = \
    Channel.h \
    Core.h \
    debugstream.h \
    events.h \
    net/PacketReader.h \
    net/packets/auth.h \
    net/packets/channels.h \
    net/packets/message.h \
    net/packets/users.h \
    net/PacketWriter.h \
    net/ServerData.h \
    net/SimpleID.h \
    net/SimpleSocket.h \
    net/TransportReader.h \
    net/TransportWriter.h \
    protov4.h \
    Server.h \
    ServerChannel.h \
    ServerUser.h \
    Storage.h \
    User.h \
    Worker.h \
    WorkerThread.h \

SOURCES = \
    Channel.cpp \
    Core.cpp \
    events.cpp \
    main.cpp \
    net/PacketReader.cpp \
    net/packets/auth.cpp \
    net/packets/channels.cpp \
    net/packets/message.cpp \
    net/packets/users.cpp \
    net/ServerData.cpp \
    net/SimpleID.cpp \
    net/SimpleSocket.cpp \
    net/TransportReader.cpp \
    net/TransportWriter.cpp \
    protov4.cpp \
    Server.cpp \
    ServerChannel.cpp \
    ServerUser.cpp \
    Storage.cpp \
    User.cpp \
    Worker.cpp \
    WorkerThread.cpp \
    
FORMS += protov4.ui

DEFINES += SCHAT_DAEMON

TRANSLATIONS += ../../data/translations/schatd2_en.ts
TRANSLATIONS += ../../data/translations/schatd2_ru.ts
CODECFORTR = UTF-8

include(../common/common.pri)

