QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


CONFIG +=  c++17

# QMAKE_CXXFLAGS += -stdlib=libc++
# QMAKE_LFLAGS += -stdlib=libc++

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ansiparser.cpp \
    chighlighter.cpp \
    codeeditor.cpp \
    cpphighlighter.cpp \
    fileexplorer.cpp \
    main.cpp \
    mainwindow.cpp \
    pythonhighlighter.cpp \
    terminal_widget.cpp \
    utils/compilationmanager.cpp \
    utils/lspclient.cpp \
    utils/lspclientpython.cpp \
    utils/powershellterminalwidget.cpp \
    utils/runterminalwidget.cpp

HEADERS += \
    ansiparser.h \
    chighlighter.h \
    codeeditor.h \
    cpphighlighter.h \
    fileexplorer.h \
    mainwindow.h \
    pythonhighlighter.h \
    terminal_widget.h \
    utils/compilationmanager.h \
    utils/lspclient.h \
    utils/lspclientpython.h \
    utils/powershellterminalwidget.h \
    utils/runterminalwidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources.qrc
