QT += gui
CONFIG += c++11
QT += network
QT += websockets
QT += sql
CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_NO_DEBUG_OUTPUT
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ChatClient.cpp \
        Functions.cpp \
        GetInputEvent.cpp \
        SnmpStack.cpp \
        TCPCommunicator.cpp \
        TransportAdapter.cpp \
        UDPCommunicator.cpp \
        Utility.cpp \
        WavWriter.cpp \
        at88sc.cpp \
        audiometer.cpp \
        database.cpp \
        gpioclass.cpp \
        main.cpp \
        networkmng.cpp \
        roip_ed137.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ChatClient.h \
    Codecs.h \
    GetInputEvent.h \
    SnmpStack.h \
    StaticVariables.h \
    TCPCommunicator.h \
    TransportAdapter.h \
    UDPCommunicator.h \
    Utility.h \
    WavWriter.h \
    at88sc.h \
    audiometer.h \
    database.h \
    ed137_rtp.h \
    gpioclass.h \
    graphics.h \
    menuindex.h \
    networkmng.h \
    roip_ed137.h

linux-jetson-nano2g-g++:{
    INCLUDEPATH += /home/ubuntu/JETSONData/nano2GB/sysroot/usr/local/include
    LIBS += -L/usr/local/lib -lpjsua2-aarch64-unknown-linux-gnu -lstdc++ -lpjsua-aarch64-unknown-linux-gnu -lpjsip-ua-aarch64-unknown-linux-gnu -lpjsip-simple-aarch64-unknown-linux-gnu -lpjsip-aarch64-unknown-linux-gnu -lpjmedia-codec-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjmedia-videodev-aarch64-unknown-linux-gnu -lpjmedia-audiodev-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjnath-aarch64-unknown-linux-gnu -lpjlib-util-aarch64-unknown-linux-gnu -lsrtp-aarch64-unknown-linux-gnu -lresample-aarch64-unknown-linux-gnu -lg729codec-aarch64-unknown-linux-gnu -lgsmcodec-aarch64-unknown-linux-gnu -lspeex-aarch64-unknown-linux-gnu -lilbccodec-aarch64-unknown-linux-gnu -lg7221codec-aarch64-unknown-linux-gnu -lyuv-aarch64-unknown-linux-gnu -lpj-aarch64-unknown-linux-gnu -lssl -lcrypto -luuid -lm -lrt -lpthread -lasound
}

linux-jetson-nano-g++:{
    INCLUDEPATH += /home/ubuntu/JETSONData/nano/sysroot/usr/local/include
    LIBS += -L/usr/local/lib -lpjsua2-aarch64-unknown-linux-gnu -lstdc++ -lpjsua-aarch64-unknown-linux-gnu -lpjsip-ua-aarch64-unknown-linux-gnu -lpjsip-simple-aarch64-unknown-linux-gnu -lpjsip-aarch64-unknown-linux-gnu -lpjmedia-codec-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjmedia-videodev-aarch64-unknown-linux-gnu -lpjmedia-audiodev-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjnath-aarch64-unknown-linux-gnu -lpjlib-util-aarch64-unknown-linux-gnu -lsrtp-aarch64-unknown-linux-gnu -lresample-aarch64-unknown-linux-gnu -lg729codec-aarch64-unknown-linux-gnu -lgsmcodec-aarch64-unknown-linux-gnu -lspeex-aarch64-unknown-linux-gnu -lilbccodec-aarch64-unknown-linux-gnu -lg7221codec-aarch64-unknown-linux-gnu -lyuv-aarch64-unknown-linux-gnu -lpj-aarch64-unknown-linux-gnu -lssl -lcrypto -luuid -lm -lrt -lpthread -lasound
}
linux-oe-generic-g++:{
    INCLUDEPATH += /opt/b2qt/2.7.2/sysroots/aarch64-poky-linux/usr/local/include
    LIBS += -lpjsua2-aarch64-unknown-linux-gnu -lstdc++ -lpjsua-aarch64-unknown-linux-gnu -lpjsip-ua-aarch64-unknown-linux-gnu -lpjsip-simple-aarch64-unknown-linux-gnu -lpjsip-aarch64-unknown-linux-gnu -lpjmedia-codec-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjmedia-videodev-aarch64-unknown-linux-gnu -lpjmedia-audiodev-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjnath-aarch64-unknown-linux-gnu -lpjlib-util-aarch64-unknown-linux-gnu  -lsrtp-aarch64-unknown-linux-gnu -lresample-aarch64-unknown-linux-gnu -lg729codec-aarch64-unknown-linux-gnu -lgsmcodec-aarch64-unknown-linux-gnu -lspeex-aarch64-unknown-linux-gnu -lilbccodec-aarch64-unknown-linux-gnu -lg7221codec-aarch64-unknown-linux-gnu -lyuv-aarch64-unknown-linux-gnu  -lpj-aarch64-unknown-linux-gnu -lssl -lcrypto -luuid -lm -lrt -lpthread  -lasound
}
linux-jetson-orin-g++:{
    INCLUDEPATH += /home/ubuntu/BackupData/BackupData/OrinNx/Jetson_Linux_R35.3.1_aarch64/QtSource/sysroot/usr/local/include
    LIBS += -L/usr/local/lib -lpjsua2-aarch64-unknown-linux-gnu -lstdc++ -lpjsua-aarch64-unknown-linux-gnu -lpjsip-ua-aarch64-unknown-linux-gnu -lpjsip-simple-aarch64-unknown-linux-gnu -lpjsip-aarch64-unknown-linux-gnu -lpjmedia-codec-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjmedia-videodev-aarch64-unknown-linux-gnu -lpjmedia-audiodev-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjnath-aarch64-unknown-linux-gnu -lpjlib-util-aarch64-unknown-linux-gnu -lsrtp-aarch64-unknown-linux-gnu -lresample-aarch64-unknown-linux-gnu -lg729codec-aarch64-unknown-linux-gnu -lgsmcodec-aarch64-unknown-linux-gnu -lspeex-aarch64-unknown-linux-gnu -lilbccodec-aarch64-unknown-linux-gnu -lg7221codec-aarch64-unknown-linux-gnu -lyuv-aarch64-unknown-linux-gnu -lpj-aarch64-unknown-linux-gnu -lssl -lcrypto -luuid -lm -lrt -lpthread -lasound
}
linux-g++:{
    LIBS += `pkg-config --libs libpjproject`
}

#aarch64-xilinx-linux-g++:{
#    LIBS += -lpjsua2-aarch64-unknown-linux-gnu -lstdc++ -lpjsua-aarch64-unknown-linux-gnu -lpjsip-ua-aarch64-unknown-linux-gnu -lpjsip-simple-aarch64-unknown-linux-gnu -lpjsip-aarch64-unknown-linux-gnu -lpjmedia-codec-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjmedia-videodev-aarch64-unknown-linux-gnu -lpjmedia-audiodev-aarch64-unknown-linux-gnu -lpjmedia-aarch64-unknown-linux-gnu -lpjnath-aarch64-unknown-linux-gnu -lpjlib-util-aarch64-unknown-linux-gnu  -lsrtp-aarch64-unknown-linux-gnu -lresample-aarch64-unknown-linux-gnu -lg729codec-aarch64-unknown-linux-gnu -lgsmcodec-aarch64-unknown-linux-gnu -lspeex-aarch64-unknown-linux-gnu -lilbccodec-aarch64-unknown-linux-gnu -lg7221codec-aarch64-unknown-linux-gnu -lyuv-aarch64-unknown-linux-gnu  -lpj-aarch64-unknown-linux-gnu -lopus -lssl -lcrypto -luuid -lm -lrt -lpthread  -lasound -L/usr/lib -lSDL2  -lavdevice -lavformat -lavcodec -lswscale -lavutil -lv4l2
#}
LIBS += -DPJ_AUTOCONF=1 -O2 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1


LIBS += -lboost_system -lboost_chrono -lboost_thread -ludev
LIBS += -lnetsnmp -lasound
