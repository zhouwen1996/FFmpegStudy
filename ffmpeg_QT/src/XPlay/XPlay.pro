#-------------------------------------------------
#
# Project created by QtCreator 2021-03-13T10:16:35
#
#-------------------------------------------------

QT       += core gui
#add QT module
QT       += opengl
QT       += openglextensions
QT       += multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XPlay
TEMPLATE = app

#add include and lib path
INCLUDEPATH += $$PWD/../../include
LIBS += -L$$PWD/../../lib/win32

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        widget.cpp \
    XDemux.cpp \
    XDcode.cpp \
    XVideoWidget.cpp \
    XResample.cpp \
    XAudioPlay.cpp \
    XAudioThread.cpp \
    XVideoThread.cpp \
    XDemuxThread.cpp \
    XDecodeThread.cpp \
    XSlider.cpp

HEADERS += \
        widget.h \
    XDemux.h \
    XDcode.h \
    XVideoWidget.h \
    XResample.h \
    XAudioPlay.h \
    XAudioThread.h \
    XVideoThread.h \
    IVideoCall.h \
    XDemuxThread.h \
    XDecodeThread.h \
    XSlider.h

FORMS += \
        widget.ui
