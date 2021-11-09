FORMS += \
    $$PWD/ui/titlebar.ui

INCLUDEPATH += \
    $$PWD/include

HEADERS += \
    $$PWD/include/mclock.h \
    $$PWD/include/mdialog.h \
    $$PWD/include/mfilemsgmanager.h \
    $$PWD/include/mhostaddress.h \
    $$PWD/include/mlogmanager.h \
    $$PWD/include/mprogress.h \
    $$PWD/include/mspeed.h \
    $$PWD/include/mudpbroadcast.h \
    $$PWD/include/titlebar.h

SOURCES += \
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
