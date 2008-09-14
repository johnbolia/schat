# $Id$
# IMPOMEZIA Simple Chat
# Copyright (c) 2008 IMPOMEZIA (http://impomezia.com)
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

SCHAT_CONSOLE    = 1
SCHAT_DEBUG      = 0
SCHAT_SINGLE_APP = 1
SCHAT_RESOURCES  = 1
SCHAT_RC_FILE    = 1

QT += network
TEMPLATE = app

HEADERS += \
    daemonui.h \
    ipc/localclientservice.h \

SOURCES += \
    daemonui.cpp \
    ipc/localclientservice.cpp \
    main.cpp \

include(../common/common.pri)
