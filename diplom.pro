# QT       += core gui
QT += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    controller.cpp \
    generator.cpp \
    jsondatareader.cpp \
    jsonplacementwriter.cpp \
    main.cpp \
    mainwindow.cpp \
    matrix_compute.cpp \
    quadraticplacement.cpp \
    visualizer.cpp

HEADERS += \
    controller.h \
    generator.h \
    jsondatareader.h \
    jsonplacementwriter.h \
    mainwindow.h \
    matrix_compute.h \
    quadraticplacement.h \
    structurs.h \
    visualizer.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += /usr/include
LIBS += -L/usr/lib -lsfml-graphics -lsfml-window -lsfml-system
INCLUDEPATH += /usr/include/eigen3

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
