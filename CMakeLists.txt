cmake_minimum_required(VERSION 3.16)

project(MusicBrainzQt 
    VERSION 1.0.0
    DESCRIPTION "A Qt-based MusicBrainz client application"
    LANGUAGES CXX
)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 支持和国际化
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network LinguistTools)

# 启用Qt MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录
include_directories(src)

# 国际化设置
set(TS_FILES
    translations/musicbrainzqt_zh_CN.ts
    translations/musicbrainzqt_en.ts
)

# 创建翻译文件
qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

# 源文件
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    
    # API Layer
    src/api/musicbrainzapi.cpp
    src/api/musicbrainzparser.cpp
    src/api/api_utils.cpp
    src/api/network_manager.cpp
    
    # Models
    src/models/resultitem.cpp
    src/models/resulttablemodel.cpp
    
    # UI Components
    src/ui/advancedsearchwidget.cpp
    src/ui/searchresulttab.cpp
    src/ui/itemdetailtab.cpp
    src/ui/entitylistwidget.cpp
    src/ui/widget_helpers.cpp
    
    # Services
    src/services/searchservice.cpp
    src/services/entitydetailmanager.cpp
    
    # Utils
    src/utils/logger.cpp
    src/utils/config_manager.cpp
)

# 头文件
set(HEADERS
    src/mainwindow.h
    
    # API Layer
    src/api/musicbrainzapi.h
    src/api/musicbrainzparser.h
    src/api/api_utils.h
    src/api/network_manager.h
    
    # Models
    src/models/resultitem.h
    src/models/resulttablemodel.h
    
    # UI Components
    src/ui/advancedsearchwidget.h
    src/ui/searchresulttab.h
    src/ui/itemdetailtab.h
    src/ui/entitylistwidget.h
    src/ui/widget_helpers.h
    
    # Services
    src/services/searchservice.h
    src/services/entitydetailmanager.h
    
    # Core Types
    src/core/types.h
    src/core/error_types.h
    
    # Utils
    src/utils/logger.h
    src/utils/config_manager.h
)

# UI 文件
set(FORMS
    ui/mainwindow.ui
    ui/itemdetailtab.ui
    ui/entitylistwidget.ui
    ui/advancedsearchwidget.ui
    ui/searchresulttab.ui
)

# 资源文件
set(RESOURCES
    resources/images.qrc
    resources/styles.qrc
    ${QM_FILES}
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 编译器特定设置
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 安装翻译文件
install(FILES ${QM_FILES} DESTINATION bin/translations)

# 测试支持
enable_testing()
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
else()
    message(WARNING "Tests directory exists but CMakeLists.txt not found. Tests will be skipped.")
endif()

# 调试信息
message(STATUS "MusicBrainzQt Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Qt6 Components: Core, Widgets, Network, LinguistTools")
message(STATUS "  Translation Files: ${TS_FILES}")
