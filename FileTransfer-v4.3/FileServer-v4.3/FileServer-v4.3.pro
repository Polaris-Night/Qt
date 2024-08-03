QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_MESSAGELOGCONTEXT #release下输出日志信息

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = FileServer-v4.3
VERSION = 2021.10.25
RC_ICONS = transfer.ico
# RC_FILE = uac.rc

MOC_DIR = temp/moc
RCC_DIR = temp/rcc
UI_DIR = temp/ui
OBJECTS_DIR = temp/obj

include($$PWD/../tool/tool.pri)

SOURCES += \
    src/main.cpp \
    src/lockmanager.cpp \
    src/mainwindow.cpp \
    src/mserver.cpp \
    src/receiver.cpp \
    src/worker.cpp

INCLUDEPATH += \
    $$PWD/src

HEADERS += \
    src/lockmanager.h \
    src/mainwindow.h \
    src/mserver.h \
    src/receiver.h \
    src/worker.h

FORMS += \
    ui/mainwindow.ui \

win32 {
DISTFILES += \
    menifest.xml \
    uac.rc
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
