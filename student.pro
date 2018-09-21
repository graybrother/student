#-------------------------------------------------
#
# Project created by QtCreator 2017-12-15T15:26:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = student
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    studentapp.cpp

HEADERS  += mainwindow.h \
    studentapp.h \
    ysqface.h

FORMS    += \
    mainwindow.ui

INCLUDEPATH += c:/opencv-contrib/include/opencv \
                 c:/opencv-contrib/include/opencv2 \
                 c:/opencv-contrib/include


LIBS +=  c:/opencv-contrib/lib/libopencv_*.dll.a
#LIBS +=  c:/opencv-contrib/lib/libopencv_core320.dll.a
#LIBS +=  c:/opencv-contrib/lib/libopencv_imgproc320.dll.a
#LIBS +=  c:/opencv-contrib/lib/libopencv_imgcodecs320.dll.a
#LIBS +=  c:/opencv-contrib/lib/libopencv_highgui320.dll.a
#LIBS +=  c:/opencv-contrib/lib/libopencv_videoio320.dll.a
#LIBS +=  c:/opencv-contrib/lib/libopencv_video320.dll.a


win32: LIBS += -L$$PWD/ -lysqface

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/
