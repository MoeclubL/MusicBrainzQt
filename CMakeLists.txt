cmake_minimum_required(VERSION 3.16)

project(MusicBrainzQt VERSION 1.0.0 LANGUAGES CXX)

# 使用C++17标准以确保更好的兼容性
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译优化和警告设置
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 优先查找Qt6，要求最低版本6.2
find_package(Qt6 6.2 COMPONENTS Core Widgets Network QUIET)
if (Qt6_FOUND)
    message(STATUS "Found Qt6: ${Qt6_VERSION}")
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 5.15 COMPONENTS Core Widgets Network REQUIRED)
    message(STATUS "Found Qt5: ${Qt5_VERSION}")
    set(QT_VERSION_MAJOR 5)
endif()

# Set up Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/api/libmusicbrainzapi.cpp
    src/api/musicbrainzparser.cpp
    src/api/musicbrainz_utils.cpp
    src/api/network_manager.cpp
    src/models/resultitem.cpp
    src/models/resulttablemodel.cpp
    src/ui/advancedsearchwidget.cpp
    src/ui/searchresulttab.cpp
    src/ui/itemdetailtab.cpp
    src/ui/entitylistwidget.cpp
    src/ui/ui_utils.cpp
    src/services/searchservice.cpp
    src/services/entitydetailmanager.cpp
    src/utils/logger.cpp
    src/utils/config_manager.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/api/libmusicbrainzapi.h
    src/api/musicbrainzparser.h
    src/api/musicbrainz_utils.h
    src/api/network_manager.h
    src/models/resultitem.h
    src/models/resulttablemodel.h
    src/ui/advancedsearchwidget.h
    src/ui/searchresulttab.h
    src/ui/itemdetailtab.h
    src/ui/entitylistwidget.h
    src/ui/ui_utils.h
    src/services/searchservice.h
    src/services/entitydetailmanager.h
    src/core/types.h
    src/core/error_types.h
    src/utils/logger.h
    src/utils/config_manager.h
)

# UI files
set(UIS
    ui/mainwindow.ui
    ui/itemdetailtab.ui
    ui/entitylistwidget.ui
    ui/advancedsearchwidget.ui
    ui/searchresulttab.ui
)

# Resource files
set(RESOURCES
    resources/images.qrc
)

# Create executable
add_executable(MusicBrainzQt ${SOURCES} ${HEADERS} ${UIS} ${RESOURCES})

# Link Qt libraries
if (QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(MusicBrainzQt Qt6::Core Qt6::Widgets Qt6::Network)
    # 启用Qt6的现代特性
    target_compile_definitions(MusicBrainzQt PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x060000)
else()
    target_link_libraries(MusicBrainzQt Qt5::Core Qt5::Widgets Qt5::Network)
endif()

# Set output directory
set_target_properties(MusicBrainzQt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation
install(TARGETS MusicBrainzQt
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)