#-------------------------------------------------
#
# Project created by QtCreator 2020-10-20T00:00:49
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = currencyconv
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        calenderwidget.cpp \
        error.cpp \
        main.cpp \
        mainwindow.cpp \
        request.cpp \
        waitingspinnerwidget.cpp

HEADERS += \
        calenderwidget.h \
        error.h \
        mainwindow.h \
        request.h \
        waitingspinnerwidget.h

FORMS += \
        calenderwidget.ui \
        error.ui \
        mainwindow.ui

# Default rules for deployment.
isEmpty(PREFIX){
 PREFIX = /usr
}

BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

icon.files = icons/currencyconv.png
icon.path = $$DATADIR/icons/hicolor/512x512/apps/

desktop.files = currencyconv.desktop
desktop.path = $$DATADIR/applications/

INSTALLS += target icon desktop

RESOURCES += \
    breeze.qrc \
    resources.qrc
