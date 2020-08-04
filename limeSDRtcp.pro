QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += -l$$PWD\LimeSuite

SOURCES += \
    limestreamer.cpp \
    main.cpp \
    mainwindow.cpp \
    spectrummonitor.cpp

HEADERS += \
    LMS7002M_parameters.h \
    LimeSuite.h \
    include/packedMonitor.h \
    limestreamer.h \
    mainwindow.h \
    spectrummonitor.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/include/ -lpackedMonitor

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include



## .h文件搜索路径
#INCLUDEPATH += D:/R2019a/extern/include
#INCLUDEPATH += D:/R2019a/extern/include/Win64

## 用到的MATLAB 的.lib库文件 及其搜索路径
#INCLUDEPATH += D:/R2019a/extern/lib/win64/microsoft
#DEPENDPATH += D:/R2019a/extern/lib/win64/microsoft

#win32: LIBS += -LD:/R2019a/extern/lib/win64/microsoft -llibmex
#win32: LIBS += -LD:/R2019a/extern/lib/win64/microsoft -llibmx
#win32: LIBS += -LD:/R2019a/extern/lib/win64/microsoft -llibmat
#win32: LIBS += -LD:/R2019a/extern/lib/win64/microsoft -llibeng
#win32: LIBS += -LD:/R2019a/extern/lib/win64/microsoft -lmclmcr
#win32: LIBS += -LD:/R2019a/extern/lib/win64/microsoft -lmclmcrrt
##LIBS += -LD:/R2019a/extern/lib/win64/microsoft/*.lib

# .h文件搜索路径
INCLUDEPATH += $$quote(C:/Program Files/MATLAB/R2017a/extern/include)
INCLUDEPATH += $$quote(C:/Program Files/MATLAB/R2017a/extern/include/Win64)

# 用到的MATLAB 的.lib库文件 及其搜索路径
INCLUDEPATH += $$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft)
DEPENDPATH += $$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft)

win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -llibmex
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -llibmx
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -llibmat
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -llibeng
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -lmclmcr
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -lmclmcrrt
#LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft)/*.lib

#$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft)
