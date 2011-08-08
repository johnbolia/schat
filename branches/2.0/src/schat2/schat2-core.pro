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

SCHAT_RESOURCES = 1
SCHAT_DEBUG     = 1
SCHAT_USE_SSL   = 1

TEMPLATE = lib
QT = core gui network webkit
TARGET = schat
DEFINES += SCHAT_CORE_LIBRARY
win32:RC_FILE = schat2-core.rc

HEADERS = \
    actions/ChatViewAction.h \
    actions/MenuBuilder.h \
    actions/UserMenu.h \
    ChatCore.h \
    ChatCore_p.h \
    ChatPlugins.h \
    ChatSettings.h \
    messages/AbstractMessage.h \
    messages/AlertMessage.h \
    messages/MessageAdapter.h \
    messages/MessageBox.h \
    messages/UserMessage.h \
    NetworkManager.h \
    plugins/AbstractHistory.h \
    plugins/ChatPlugin.h \
    schat.h \
    ui/tabs/AbstractTab.h \
    ui/tabs/ChatView.h \
    ui/tabs/ChatViewTab.h \
    ui/tabs/PrivateTab.h \
    ui/UserUtils.h \

SOURCES = \
    actions/ChatViewAction.cpp \
    actions/MenuBuilder.cpp \
    actions/UserMenu.cpp \
    ChatCore.cpp \
    ChatPlugins.cpp \
    ChatSettings.cpp \
    messages/AbstractMessage.cpp \
    messages/AlertMessage.cpp \
    messages/MessageAdapter.cpp \
    messages/MessageBox.cpp \
    messages/UserMessage.cpp \
    NetworkManager.cpp \
    plugins/AbstractHistory.cpp \
    plugins/ChatPlugin.cpp \
    ui/tabs/AbstractTab.cpp \
    ui/tabs/ChatView.cpp \
    ui/tabs/ChatViewTab.cpp \
    ui/tabs/PrivateTab.cpp \
    ui/UserUtils.cpp \
    
SCHAT_CLIENT_LIB = 1

TRANSLATIONS += ../../data/translations/schat2-core_en.ts
TRANSLATIONS += ../../data/translations/schat2-core_ru.ts
CODECFORTR = UTF-8

include(../common/common.pri)
