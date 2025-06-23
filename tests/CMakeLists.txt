# 测试配置文件
# MusicBrainzQt 单元测试

# 查找Qt测试模块
find_package(Qt6 REQUIRED COMPONENTS Test)

# 包含主项目的头文件
include_directories(../src)

# 测试源文件
set(TEST_SOURCES
    tst_api.cpp
    tst_models.cpp
)

# 创建测试可执行文件
foreach(test_source ${TEST_SOURCES})
    # 获取测试名称（去掉.cpp扩展名）
    get_filename_component(test_name ${test_source} NAME_WE)
    
    # 创建测试可执行文件
    add_executable(${test_name}
        ${test_source}
        # 包含需要测试的源文件
        ../src/models/resultitem.cpp
        ../src/models/resulttablemodel.cpp
        ../src/api/musicbrainzapi.cpp
        ../src/api/musicbrainzparser.cpp
        ../src/api/api_utils.cpp
        ../src/api/network_manager.cpp
        ../src/core/types.h
        ../src/core/error_types.h
    )
    
    # 链接Qt库
    target_link_libraries(${test_name}
        Qt6::Core
        Qt6::Test
        Qt6::Network
        Qt6::Widgets
    )
    
    # 添加到测试套件
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# 设置测试输出目录
set_target_properties(${TEST_SOURCES} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

message(STATUS "Tests configured: ${TEST_SOURCES}")
