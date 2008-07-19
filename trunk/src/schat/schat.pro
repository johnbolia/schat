# $Id$
# Simple Chat
# Copyright � 2008 IMPOMEZIA (http://impomezia.net.ru)

TEMPLATE = app
TARGET = 
DEPENDPATH  += . \
               ../common \
               ../schatd               
INCLUDEPATH += . \
               ../common \
               ../schatd
QT += network
CONFIG += console
#DEFINES += SCHAT_DEBUG SCHAT_CLIENT
DEFINES += SCHAT_CLIENT

RESOURCES += ../../data/schat.qrc

HEADERS += \
    aboutdialog.h \
    abstracttab.h \
    channellog.h \
    chatbrowser.h \
    clientsocket.h \
    daemonsettings.h \
    directchannel.h \
    directchannelserver.h \
    log.h \
    mainchannel.h \
    network.h \
    networkreader.h \
    networkwidget.h \
    profile.h \
    profilewidget.h \
    protocol.h \
    schatwindow.h \
    server.h \
    serversocket.h \
    settings.h \
    settingsdialog.h \
    tab.h \
    version.h \
    welcomedialog.h \
    clientservice.h \
    clientthread.h

SOURCES += \
    aboutdialog.cpp \
    abstracttab.cpp \
    channellog.cpp \
    chatbrowser.cpp \
    clientsocket.cpp \
    daemonsettings.cpp \
    directchannel.cpp \
    directchannelserver.cpp \
    log.cpp \
    main.cpp \
    mainchannel.cpp \
    network.cpp \
    networkreader.cpp \
    networkwidget.cpp \
    profile.cpp \
    profilewidget.cpp \
    schatwindow.cpp \
    server.cpp \
    serversocket.cpp \
    settings.cpp \
    settingsdialog.cpp \
    tab.cpp \
    welcomedialog.cpp \
    clientservice.cpp \
    clientthread.cpp

CONFIG(debug, debug|release) { 
    RCC_DIR = ../../tmp/schat/debug/rcc
    MOC_DIR = ../../tmp/schat/debug/moc
    OBJECTS_DIR = ../../tmp/schat/debug/obj
    DESTDIR = ../../out/debug
}

CONFIG(release, debug|release) { 
    RCC_DIR = ../../tmp/schat/release/rcc
    MOC_DIR = ../../tmp/schat/release/moc
    OBJECTS_DIR = ../../tmp/schat/release/obj
    DESTDIR = ../../out/release
}

win32 {
    RC_FILE = schat.rc
    HEADERS += win32/updatenotify.h
    SOURCES += win32/updatenotify.cpp
    DEFINES += SCHAT_UPDATE
}

INCLUDEPATH += ../3rdparty/singleapplication/include
SINGLEAPPLICATION_LIBCONFIG = staticlib
win32-msvc*:LIBS += ../3rdparty/singleapplication/bin/singleapplication.lib
else:LIBS += -L ../3rdparty/singleapplication/bin -lsingleapplication
