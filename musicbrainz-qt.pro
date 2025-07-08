TEMPLATE = app
TARGET = musicbrainz-qt
QT += core gui widgets network

# 使用C++17标准以确保更好的兼容性
CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/api/musicbrainzapi.cpp \
    src/api/musicbrainz_response_handler.cpp \
    src/api/musicbrainzparser.cpp \
    src/api/api_utils.cpp \
    src/api/network_manager.cpp \
    src/models/resultitem.cpp \
    src/models/resulttablemodel.cpp \
    src/ui/advancedsearchwidget.cpp \
    src/ui/searchresulttab.cpp \
    src/ui/itemdetailtab.cpp \
    src/ui/entitylistwidget.cpp \
    src/ui/widget_helpers.cpp \
    src/services/searchservice.cpp \
    src/services/entitydetailmanager.cpp \
    src/utils/config_manager.cpp     src/ui/settingsdialog.cpp

HEADERS += \
    src/mainwindow.h \
    src/api/musicbrainzapi.h \
    src/api/musicbrainz_response_handler.h \
    src/api/musicbrainzparser.h \
    src/api/api_utils.h \
    src/api/network_manager.h \
    src/models/resultitem.h \
    src/models/resulttablemodel.h \
    src/ui/advancedsearchwidget.h \
    src/ui/searchresulttab.h \
    src/ui/itemdetailtab.h \
    src/ui/entitylistwidget.h \
    src/ui/widget_helpers.h \
    src/services/searchservice.h \
    src/services/entitydetailmanager.h \
    src/core/types.h \
    src/core/error_types.h \
    src/utils/config_manager.h \
    src/ui/settingsdialog.h

FORMS +=     ui/mainwindow.ui     ui/itemdetailtab.ui     ui/entitylistwidget.ui     ui/advancedsearchwidget.ui     ui/searchresulttab.ui     ui/settingsdialog.ui

RESOURCES += 
    resources/images.qrc

TRANSLATIONS += \
    translations/musicbrainzqt_en.ts \
    translations/musicbrainzqt_zh_CN.ts
