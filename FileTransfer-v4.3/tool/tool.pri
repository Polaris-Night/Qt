FORMS += \
    $$PWD/ui/titlebar.ui

INCLUDEPATH += \
    $$PWD/src

HEADERS += \
    $$PWD/src/filespliter.h \
    $$PWD/src/mclock.h \
    $$PWD/src/mdialog.h \
    $$PWD/src/message.h \
    $$PWD/src/mfilemsgmanager.h \
    $$PWD/src/mhostaddress.h \
    $$PWD/src/mlogmanager.h \
    $$PWD/src/mprogress.h \
    $$PWD/src/mspeed.h \
    $$PWD/src/mudpbroadcast.h \
    $$PWD/src/titlebar.h

SOURCES += \
    $$PWD/src/filespliter.cpp \
    $$PWD/src/mclock.cpp \
    $$PWD/src/mdialog.cpp \
    $$PWD/src/mfilemsgmanager.cpp \
    $$PWD/src/mhostaddress.cpp \
    $$PWD/src/mlogmanager.cpp \
    $$PWD/src/mprogress.cpp \
    $$PWD/src/mspeed.cpp \
    $$PWD/src/mudpbroadcast.cpp \
    $$PWD/src/titlebar.cpp

RESOURCES += \
    $$PWD/qss.qrc \
