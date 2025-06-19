cmake_minimum_required(VERSION 3.16)

project(MusicBrainzQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(Qt6 COMPONENTS Core Widgets Network QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Widgets Network REQUIRED)
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
    src/models/resultitem.cpp
    src/models/resultitemmodel.cpp
    src/models/resulttablemodel.cpp
    src/ui/advancedsearchwidget.cpp
    src/ui/searchresulttab.cpp
    src/ui/itemdetailtab.cpp
    src/ui/entitylistwidget.cpp
    src/services/searchservice.cpp
    src/services/entitydetailmanager.cpp
    src/utils/logger.cpp
    src/utils/config_manager.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/api/libmusicbrainzapi.h
    src/api/musicbrainztypes.h
    src/models/resultitem.h
    src/models/resultitemmodel.h
    src/models/resulttablemodel.h
    src/ui/advancedsearchwidget.h
    src/ui/searchresulttab.h
    src/ui/itemdetailtab.h
    src/ui/entitylistwidget.h
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
if (Qt6_FOUND)
    target_link_libraries(MusicBrainzQt Qt6::Core Qt6::Widgets Qt6::Network)
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