TEMPLATE = app
TARGET = video_from_buffer
QT += gui gui-private multimedia multimediawidgets
DEFINES += "QT_AVPLAYER_MULTIMEDIA"
DEFINES += "QT_NO_CAST_FROM_ASCII"
INCLUDEPATH += . ../../src

LIBS += -L$$PWD/../../../ffmpeg-7.0.1-full_build-shared/lib
INCLUDEPATH += $$PWD/../../../ffmpeg-7.0.1-full_build-shared/include

include(../../src/QtAVPlayer/QtAVPlayer.pri)

CONFIG += c++1z

SOURCES += main.cpp \
    file1.cpp \
    rawdataparser.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/$$TARGET
INSTALLS += target

HEADERS += \
    file1.h \
    rawdataparser.h
