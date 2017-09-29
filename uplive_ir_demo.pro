TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp \
    ir-view.cpp \
    pox_system.c \
    image-item.cpp \
    h264-enc.c \
    image-encode.cpp \
    librtmp_send264.cpp \
    image-send.cpp

RESOURCES += qml.qrc
TARGET = main

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

DEFINES += FLS_SYS

contains( DEFINES, FLS_SYS ) {
    INCLUDEPATH += "$$_PRO_FILE_PWD_/uplive-core-libs/include/librtmp"
    INCLUDEPATH += "$$_PRO_FILE_PWD_/uplive-core-libs/include/openssl"
#    LIBS += "$$_PRO_FILE_PWD_/uplive-core-libs/freescale/librtmp/librtmp.so"  \
#            "$$_PRO_FILE_PWD_/uplive-core-libs/freescale/librtmp/librtmp.so.1" \
#            "$$_PRO_FILE_PWD_/uplive-core-libs/freescale/librtmp/libssl.so" \
#            "$$_PRO_FILE_PWD_/uplive-core-libs/freescale/librtmp/libssl.so.1.0.0"
    LIBS += "$$_PRO_FILE_PWD_/uplive-core-libs/freescale/librtmp/librtmp.a"  \
            "$$_PRO_FILE_PWD_/uplive-core-libs/freescale/librtmp/libssl.a" \
            "$$_PRO_FILE_PWD_/uplive-core-libs/freescale/librtmp/libcrypto.so" \
            "$$_PRO_FILE_PWD_/uplive-core-libs/freescale/librtmp/libcrypto.so.1.0.0"


    INCLUDEPATH += "$$_PRO_FILE_PWD_/uplive-core-libs/include/vpu/freescale"
    LIBS += "$$_PRO_FILE_PWD_/uplive-core-libs/freescale/libvpu.a"
    INCLUDEPATH += "$$_PRO_FILE_PWD_/uplive-core-libs/include/ffmpeg/freescale"
    LIBS += -L"$$_PRO_FILE_PWD_/uplive-core-libs/freescale/ffmpeg/" -lavformat-57 -lavcodec-57 -lavdevice-57  -lavfilter-6 -lavutil-55 -lswresample-2 -lswscale-4 -lpostproc-54
}

HEADERS += \
    config.h \
    ir-view.h \
    pci-irlib-machine.h \
    pox_system.h \
    image-item.h \
    h264-enc.h \
    image-encode.h \
    librtmp_send264.h \
    sps_decode.h \
    image-send.h

DISTFILES +=
