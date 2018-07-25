#-------------------------------------------------
#
# Project created by QtCreator 2017-06-12T08:56:10
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = iptvutils
TEMPLATE = app

win32:VERSION = 0.6.0.1 # major.minor.patch.build
else:VERSION = 0.6.0    # major.minor.patch

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    interface.cpp \
    igmp.cpp \
    packetparser.cpp \
    pcapfilter.cpp \
    Recorder/recorder.cpp \
    Player/player.cpp \
    Analyzer/analyzer.cpp \
    PacketProducer/packetproducer.cpp \
    PacketProducer/product.cpp \
    PacketProducer/pcapproduct.cpp \
    Configuration/workerconfiguration.cpp \
    Configuration/networkconfiguration.cpp \
    Configuration/fileconfiguration.cpp \
    PacketProducer/pcapbufferedproducer.cpp \
    PacketConsumer/packetconsumer.cpp \
    PacketConsumer/pcapnetworkconsumer.cpp \
    Player/pcapfilenetworkplayer.cpp \
    Recorder/networkpcapfilerecorder.cpp \
    PacketConsumer/pcapfileconsumer.cpp \
    Recorder/tsnetworkfilerecorder.cpp \
    PacketConsumer/tsfileconsumer.cpp \
    Recorder/pcaptsconverter.cpp \
    Middleware/pcapmiddleware.cpp \
    Middleware/looppcapmiddleware.cpp \
    Middleware/tsparser.cpp \
    Status/analyzerstatus.cpp \
    Middleware/analyzerpcapmiddleware.cpp \
    validator.cpp \
    Analyzer/tsanalyzer.cpp \
    playbackwidget.cpp \
    recordwidget.cpp \
    convertwidget.cpp \
    PacketConsumer/tsnetworkconsumer.cpp \
    PacketProducer/tsproduct.cpp \
    Player/tsfilenetworkplayer.cpp \
    PacketProducer/tsbufferedproducer.cpp \
    Middleware/analyzertsmiddleware.cpp \
    Middleware/tsmiddleware.cpp

HEADERS += \
        mainwindow.h \
    interface.h \
    igmp.h \
    packetparser.h \
    pcapfilter.h \
    Recorder/recorder.h \
    Player/player.h \
    concurrentqueue.h \
    Analyzer/analyzer.h \
    PacketProducer/packetproducer.h \
    PacketProducer/product.h \
    PacketProducer/pcapproduct.h \
    Configuration/workerconfiguration.h \
    Configuration/networkconfiguration.h \
    Configuration/networkoutputconfiguration.h \
    Configuration/networkinputconfiguration.h \
    Configuration/fileconfiguration.h \
    Configuration/fileinputconfiguration.h \
    Configuration/fileoutputconfiguration.h \
    PacketProducer/pcapbufferedproducer.h \
    PacketConsumer/packetconsumer.h \
    PacketConsumer/pcapnetworkconsumer.h \
    Player/pcapfilenetworkplayer.h \
    Recorder/networkpcapfilerecorder.h \
    PacketConsumer/pcapfileconsumer.h \
    Recorder/tsnetworkfilerecorder.h \
    PacketConsumer/tsfileconsumer.h \
    Status/status.h \
    Recorder/pcaptsconverter.h \
    Middleware/pcapmiddleware.h \
    Middleware/looppcapmiddleware.h \
    Middleware/tsparser.h \
    Status/analyzerstatus.h \
    Middleware/analyzerpcapmiddleware.h \
    Middleware/pidinfo.h \
    Status/finalstatus.h \
    validator.h \
    Analyzer/tsanalyzer.h \
    Analyzer/tserrors.h \
    Middleware/programinfo.h \
    Status/streaminfo.h \
    playbackwidget.h \
    recordwidget.h \
    convertwidget.h \
    Status/streamid.h \
    Status/workerstatus.h \
    PacketConsumer/tsnetworkconsumer.h \
    PacketProducer/tsproduct.h \
    PacketProducer/productprovider.h \
    Player/tsfilenetworkplayer.h \
    PacketProducer/tsbufferedproducer.h \
    Middleware/analyzertsmiddleware.h \
    Middleware/tsmiddleware.h

FORMS += \
        mainwindow.ui \
    playbackwidget.ui \
    recordwidget.ui \
    convertwidget.ui



unix {
    LIBS += -lpcap
}

unix:!mac {
    #VARIABLES
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    BINDIR = $$PREFIX/bin
    DATADIR =$$PREFIX/share

    INSTALLS += target desktop icon32

    target.path =$$BINDIR

    desktop.path = $$DATADIR/applications/$${TARGET}
    desktop.files += $${TARGET}.desktop

    icon32.path = $$DATADIR/icons/hicolor/32x32/apps
    icon32.files += icons/iptvutils.png
}

win32 {
    INCLUDEPATH += $$PWD/WpdPack_4_1_2\WpdPack\Include
    LIBS += -L"$$PWD/WpdPack_4_1_2\WpdPack\Lib\x64" -lwpcap -lws2_32
    RC_ICONS += icons/iptvutils.ico
}

mac {
    ICON = icons/iptvutils.icns
}

RESOURCES += \
    img.qrc \
    icons.qrc

# Define the preprocessor macro to get the application version in our application.
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
