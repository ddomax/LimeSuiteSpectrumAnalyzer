QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    filesink.cpp \
    filesource.cpp \
    limestreamer.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    spectrummonitor.cpp \
    spectrumplotter.cpp

HEADERS += \
    MatlabCode/build/for_redistribution_files_only/packedMonitor.h \
    filesink.h \
    filesource.h \
    include/LMS7002M_parameters.h \
    include/LimeSuite.h \
    limestreamer.h \
    mainwindow.h \
    qcustomplot.h \
    spectrummonitor.h \
    spectrumplotter.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Environment for LimeSuite
win32: LIBS += -L$$PWD/include/ -lLimeSuite

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

# Environment for Matlab
win32: LIBS += -L$$PWD/MatlabCode/build/for_redistribution_files_only/ -lpackedMonitor

INCLUDEPATH += $$PWD/MatlabCode/build/for_redistribution_files_only
DEPENDPATH += $$PWD/MatlabCode/build/for_redistribution_files_only

INCLUDEPATH += $$quote(C:/Program Files/MATLAB/R2017a/extern/include)
INCLUDEPATH += $$quote(C:/Program Files/MATLAB/R2017a/extern/include/Win64)

INCLUDEPATH += $$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft)
DEPENDPATH += $$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft)

win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -llibmex
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -llibmx
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -llibmat
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -llibeng
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -lmclmcr
win32: LIBS += -L$$quote(C:/Program Files/MATLAB/R2017a/extern/lib/win64/microsoft) -lmclmcrrt

#QMAKE_CXXFLAGS += /F 128000000
QMAKE_LFLAGS   += /STACK:512000000
