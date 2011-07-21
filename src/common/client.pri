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

HEADERS += \
    Channel.h \
    client/ClientCmd.h \
    client/ClientHelper.h \
    client/SimpleClient.h \
    client/SimpleClient_p.h \
    FileLocations.h \
    net/PacketReader.h \
    net/packets/auth.h \
    net/packets/channels.h \
    net/packets/message.h \
    net/packets/notices.h \
    net/packets/users.h \
    net/PacketWriter.h \
    net/ServerData.h \
    net/SimpleID.h \
    net/SimpleSocket.h \
    net/SimpleSocket_p.h \
    net/TransportReader.h \
    net/TransportWriter.h \
    schat.h \
    User.h \

SOURCES += \
    Channel.cpp \
    client/ClientCmd.cpp \
    client/ClientHelper.cpp \
    client/SimpleClient.cpp \
    FileLocations.cpp \
    net/PacketReader.cpp \
    net/packets/auth.cpp \
    net/packets/channels.cpp \
    net/packets/message.cpp \
    net/packets/notices.cpp \
    net/packets/users.cpp \
    net/ServerData.cpp \
    net/SimpleID.cpp \
    net/SimpleSocket.cpp \
    net/TransportReader.cpp \
    net/TransportWriter.cpp \
    User.cpp \
