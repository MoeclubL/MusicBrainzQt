# MusicBrainz Qt Application

这是一个使用 Qt 开发的跨平台应用程序，提供与 MusicBrainz 网页版相同的功能。用户可以搜索、查询和编辑音乐数据。

## 功能特点

- **搜索功能**: 用户可以使用 MusicBrainz API 搜索专辑、艺术家和发行版
- **查询音乐数据**: 检索音乐条目的详细信息
- **编辑音乐条目**: 用户可以通过友好的界面编辑音乐条目信息
- **缓存系统**: 本地缓存搜索结果以提高性能
- **设置管理**: 保存用户偏好设置

## 系统要求

- Qt 5.15 或 Qt 6.0 及以上版本
- C++17 兼容的编译器
- CMake 3.16 或更高版本（如果使用 CMake 构建）
- 网络连接（用于访问 MusicBrainz API）

## 项目结构

```
MusicBrainzQt/
├── src/                    # 源代码目录
│   ├── main.cpp           # 应用程序入口点
│   ├── mainwindow.cpp/.h  # 主窗口实现
│   ├── api/               # MusicBrainz API 交互
│   │   ├── musicbrainzapi.cpp/.h
│   │   └── networkrequest.cpp/.h
│   ├── models/            # 数据模型类
│   │   ├── album.cpp/.h
│   │   ├── artist.cpp/.h
│   │   ├── release.cpp/.h
│   │   └── track.cpp/.h
│   ├── ui/                # UI 组件
│   │   ├── searchwidget.cpp/.h
│   │   ├── editdialog.cpp/.h
│   │   └── resultview.cpp/.h
│   └── utils/             # 实用工具类
│       ├── cache.cpp/.h
│       └── settings.cpp/.h
├── ui/                    # Qt Designer UI 文件
│   ├── mainwindow.ui
│   ├── searchwidget.ui
│   ├── editdialog.ui
│   └── resultview.ui
├── resources/             # 资源文件
│   ├── images.qrc
│   └── icons/
├── tests/                 # 测试文件
├── build/                 # 构建输出目录
├── CMakeLists.txt         # CMake 构建配置
├── musicbrainz-qt.pro     # qmake 项目文件
└── README.md
```

## 构建说明

### 方法一：使用 Qt Creator
1. 打开 Qt Creator
2. 选择 "File" -> "Open File or Project"
3. 选择 `musicbrainz-qt.pro` 或 `CMakeLists.txt`
4. 配置项目（选择合适的 Qt 版本）
5. 点击构建按钮

### 方法二：使用 qmake（命令行）
```bash
# 设置 Qt 环境（根据你的安装路径调整）
# Windows: call "C:\Qt\6.x.x\msvc2022_64\bin\qtenv2.bat"

mkdir build
cd build
qmake ../musicbrainz-qt.pro

# Windows (MSVC):
nmake

# Windows (MinGW):
mingw32-make

# Linux/macOS:
make
```

### 方法三：使用 CMake
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### 便捷构建脚本
项目提供了几个构建脚本：

- **Windows + MSVC**: 运行 `build.bat`
- **Windows + MinGW**: 运行 `build-mingw.bat`  
- **CMake**: 运行 `build-cmake.bat`

## 使用说明

1. **启动应用程序**: 运行构建生成的可执行文件
2. **搜索音乐**:
   - 在搜索栏中输入艺术家、专辑或歌曲名称
   - 选择搜索类型（艺术家、专辑、曲目等）
   - 点击搜索按钮
3. **查看结果**: 搜索结果将显示在结果表格中
4. **编辑条目**: 选择一个结果项，点击"编辑"按钮进行修改
5. **查看详情**: 在结果列表中选择项目查看详细信息

## 开发说明

### 添加新功能
1. 在相应的目录中创建新的源文件
2. 更新 `CMakeLists.txt` 或 `musicbrainz-qt.pro`
3. 如果需要新的 UI，使用 Qt Designer 创建 `.ui` 文件

### API 集成
应用程序使用 MusicBrainz Web Service API v2。有关 API 详细信息：
- 文档: https://musicbrainz.org/doc/MusicBrainz_API
- 速率限制: 1 req/sec，请遵守使用政策

### 编码规范
- 使用 Qt 风格的 C++ 编码
- 类名使用 PascalCase
- 成员变量使用 m_ 前缀
- 私有方法使用 camelCase

## 故障排除

### 编译错误
- 确保已安装正确版本的 Qt
- 检查编译器是否支持 C++17
- 确保所有依赖项都已正确安装

### 运行时问题
- 检查网络连接
- 确保 Qt 库在系统 PATH 中
- 查看控制台输出获取错误信息

## 许可证

本项目采用 MIT 许可证 - 详情请参阅 LICENSE 文件。

## 贡献

欢迎提交 Pull Request 和 Issue。请确保：
1. 代码符合项目的编码规范
2. 包含适当的测试
3. 更新相关文档

## 联系信息

如有问题或建议，请通过 GitHub Issues 联系我们。
  
- `ui/`: Contains UI design files created with Qt Designer.
  
- `resources/`: Contains resource files such as images and icons.
  
- `tests/`: Contains unit tests for API and model functionalities.

## Build Instructions

1. Ensure you have Qt installed on your system.
2. Clone the repository.
3. Navigate to the project directory.
4. Run `qmake` to generate the Makefile.
5. Build the project using `make` (or your preferred build system).

## Usage

1. Launch the application.
2. Use the search widget to find music entries.
3. View detailed information and edit entries as needed.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.