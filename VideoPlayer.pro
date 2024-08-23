
QT       += core gui
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets



UI_DIR  = obj/Gui
MOC_DIR = obj/Moc
OBJECTS_DIR = obj/Obj




CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17

TARGET = VideoPlayer
TEMPLATE = app


include(module/VideoPlayer/VideoPlayer.pri)

include(module/DragAbleWidget/DragAbleWidget.pri)

SOURCES += src/main.cpp \
    src/Widget/VideoPlayerWidget.cpp \
    src/Widget/ShowVideoWidget.cpp \
    src/Widget/VideoSlider.cpp


HEADERS  += \
    src/Widget/VideoPlayerWidget.h \
    src/Widget/ShowVideoWidget.h \
    src/Widget/VideoSlider.h

FORMS    += \
    src/Widget/VideoPlayerWidget.ui \
    src/Widget/ShowVideoWidget.ui

RESOURCES += \
    resources/resources.qrc

INCLUDEPATH += $$PWD/src

win32:RC_FILE=$$PWD/resources/main.rc
