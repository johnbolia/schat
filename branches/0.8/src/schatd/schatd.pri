# $Id$
# IMPOMEZIA Simple Chat
# Copyright (c) 2008-2010 IMPOMEZIA <schat@impomezia.com>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

QT = core network
TEMPLATE = app

DEFINES += SCHAT_NO_WRITE_SETTINGS

HEADERS = \
    abstractprofile.h \
    abstractsettings.h \
    channellog.h \
    clientservice.h \
    daemon.h \
    daemonapp.h \
    daemonlog.h \
    daemonservice.h \
    daemonsettings.h \
    linkunit.h \
    network.h \
    networkreader.h \
    normalizereader.h \
    protocol.h \
    translation.h \
    userunit.h \
    version.h \

SOURCES = \
    abstractprofile.cpp \
    abstractsettings.cpp \
    channellog.cpp \
    clientservice.cpp \
    daemon.cpp \
    daemonapp.cpp \
    daemonlog.cpp \
    daemonservice.cpp \
    daemonsettings.cpp \
    linkunit.cpp \
    main.cpp \
    network.cpp \
    networkreader.cpp \
    normalizereader.cpp \
    translation.cpp \
    userunit.cpp \

contains( SCHAT_LOCAL_IPC, 1 ) {
  HEADERS += ipc/localservice.h
  SOURCES += ipc/localservice.cpp
}
else {
  DEFINES += SCHAT_NO_LOCAL_SERVER
}

TRANSLATIONS += ../../data/translations/schatd_ru.ts
CODECFORTR = UTF-8

etc.files = ../../data/normalize.xml
etc.files += ../../data/motd.html

unix {
  contains( SCHAT_EASY_DAEMON, 1 ) {
    target.path += ~/schatd
    etc.path = ~/schatd
  }
  else {
    target.path += $$SCHAT_PREFIX/usr/bin
    etc.path = $$SCHAT_PREFIX/etc/schat
  }

  INSTALLS += target etc
}

include(../3rdparty/qtservice/src/qtservice.pri)
include(../common/common.pri)
