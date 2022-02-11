QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    authwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    sessionhandler.cpp

HEADERS += \
    authwindow.h \
    mainwindow.h \
    sessionhandler.h

FORMS += \
    authwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/release/ -lssh
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/debug/ -lssh
else:unix: LIBS += -L$$PWD/lib/ -lssh

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/release/libssh.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/debug/libssh.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/release/ssh.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/debug/ssh.lib
else:unix: PRE_TARGETDEPS += $$PWD/lib/libssh.a
