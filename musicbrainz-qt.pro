TEMPLATE = app
TARGET = musicbrainz-qt
QT += core gui widgets network

# 使用C++17标准以确保更好的兼容性
CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/api/libmusicbrainzapi.cpp \
    src/api/musicbrainzparser.cpp \
    src/api/musicbrainz_utils.cpp \
    src/api/network_manager.cpp \
    src/models/resultitem.cpp \
    src/models/resulttablemodel.cpp \
    src/ui/advancedsearchwidget.cpp \
    src/ui/searchresulttab.cpp \
    src/ui/itemdetailtab.cpp \
    src/ui/entitylistwidget.cpp \
    src/ui/ui_utils.cpp \
    src/services/searchservice.cpp \
    src/services/entitydetailmanager.cpp \
    src/utils/logger.cpp \
    src/utils/config_manager.cpp

HEADERS += \
    src/mainwindow.h \
    src/api/libmusicbrainzapi.h \
    src/api/musicbrainzparser.h \
    src/api/musicbrainz_utils.h \
    src/api/network_manager.h \
    src/models/resultitem.h \
    src/models/resulttablemodel.h \
    src/ui/advancedsearchwidget.h \
    src/ui/searchresulttab.h \
    src/ui/itemdetailtab.h \
    src/ui/entitylistwidget.h \
    src/ui/ui_utils.h \
    src/services/searchservice.h \
    src/services/entitydetailmanager.h \
    src/core/types.h \
    src/core/error_types.h \
    src/utils/logger.h \
    src/utils/config_manager.h

FORMS += \
    ui/mainwindow.ui \
    ui/itemdetailtab.ui \
    ui/entitylistwidget.ui \
    ui/advancedsearchwidget.ui \
    ui/searchresulttab.ui

RESOURCES += \
    resources/images.qrc

# Include paths
INCLUDEPATH += src \
               src/api \
               src/models \
               src/ui \
               src/services \
               src/core \
               src/utils

# Additional configurations
DESTDIR = bin