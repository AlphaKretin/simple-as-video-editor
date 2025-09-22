QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SimpleVideoEditor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        src/main.cpp \
        src/SimpleVideoEditor.cpp \
        src/TrimDialog.cpp \
        src/CropDialog.cpp \
        src/ResizeDialog.cpp \
        src/ConvertDialog.cpp \
        src/convertVideo.cpp

HEADERS += \
        src/SimpleVideoEditor.h \
        src/TrimDialog.h \
        src/CropDialog.h \
        src/ResizeDialog.h \
        src/ConvertDialog.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Application icon (Windows) - Commented out until icon file is available
# win32:RC_ICONS += resources/app_icon.ico

# macOS icon - Commented out until icon file is available
# macx:ICON = resources/app_icon.icns

# Create a resources directory
!exists(resources) {
    system(mkdir resources)
    message("Created resources directory")
}

# Check for FFmpeg
unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libavcodec libavformat libavutil
}

macx {
    # macOS typically installs FFmpeg via Homebrew
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib -lavcodec -lavformat -lavutil
}

win32 {
    # For Windows, specify FFmpeg paths
    # Assuming FFmpeg is installed in the default location
    INCLUDEPATH += C:/ffmpeg/include
    LIBS += -LC:/ffmpeg/lib -lavcodec -lavformat -lavutil -lavdevice -lavfilter -lswscale -lswresample
    
    # If using a custom FFmpeg build, change the paths accordingly
    # Example for custom installation
    # INCLUDEPATH += $$(FFMPEG_DIR)/include
    # LIBS += -L$$(FFMPEG_DIR)/lib -lavcodec -lavformat -lavutil -lavdevice -lavfilter -lswscale -lswresample
}
