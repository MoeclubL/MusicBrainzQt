# MusicBrainzQt

A modern, cross-platform desktop application for browsing and searching the MusicBrainz music database. Built with Qt 6, MusicBrainzQt provides a native desktop experience for exploring music metadata.

## 🎵 What is MusicBrainzQt?

MusicBrainzQt is a desktop client that connects to the [MusicBrainz](https://musicbrainz.org/) database, allowing you to:

- **Search** for artists, albums, recordings, and other music entities
- **Browse** detailed information about music with comprehensive metadata
- **Explore** relationships between different music entities
- **View** tags, aliases, and community-contributed information
- **Navigate** through an intuitive, modern interface

## 🚀 For End Users

### Quick Start

1. **Download** the latest release from the [Releases](https://github.com/your-username/MusicBrainzQt/releases) page
2. **Install** and run MusicBrainzQt
3. **Search** for your favorite artists or albums using the search bar
4. **Explore** detailed information by double-clicking on any result

### Features

- **Universal Search**: Find artists, albums, recordings, works, and more
- **Rich Detail Views**: Comprehensive information tabs for each entity
- **Relationship Explorer**: Discover connections between music entities  
- **Tag Browser**: View community tags and metadata
- **Modern UI**: Clean, responsive interface following platform conventions
- **Fast Performance**: Efficient caching and optimized API usage

### System Requirements

- **Windows**: Windows 10 or later
- **macOS**: macOS 10.15 or later  
- **Linux**: Most modern distributions
- **Internet**: Required for MusicBrainz API access

## 💻 For Developers

### Building from Source

#### Prerequisites

- Qt 6.0+ (Core, Widgets, Network modules)
- C++17 compatible compiler
- CMake 3.16+ or qmake
- Git

#### Clone and Build

```bash
# Clone the repository
git clone https://github.com/your-username/MusicBrainzQt.git
cd MusicBrainzQt

# Using CMake (recommended)
mkdir build && cd build
cmake ..
cmake --build .

# Using qmake (alternative)
qmake musicbrainz-qt.pro
make
```

#### Project Structure

```
MusicBrainzQt/
├── src/                    # Source code
│   ├── api/               # MusicBrainz API integration  
│   ├── models/            # Data models
│   ├── ui/                # UI components
│   ├── services/          # Business logic services
│   ├── core/              # Core types and utilities
│   └── utils/             # Helper utilities
├── ui/                    # Qt Designer UI files
├── resources/             # Application resources
├── tests/                 # Unit tests
└── docs/                  # Documentation
```

### Architecture

MusicBrainzQt follows modern Qt best practices:

- **Layered Architecture**: Clear separation between API, business logic, and UI
- **Component-Based UI**: Reusable widgets with .ui file separation
- **Signal-Slot Communication**: Qt's event system for loose coupling
- **Smart Pointers**: Modern C++ memory management
- **Async Operations**: Non-blocking API requests with proper error handling

## 📚 Documentation

- **[Development Guide](docs/DEVELOPMENT.md)** - Detailed development setup and guidelines
- **[API Documentation](docs/API.md)** - MusicBrainz API integration details
- **[Contributing Guide](CONTRIBUTING.md)** - How to contribute to the project
- **[Architecture Overview](docs/ARCHITECTURE.md)** - Technical architecture documentation

## 🤝 Contributing

We welcome contributions from the community! Whether you're fixing bugs, adding features, or improving documentation, your help is appreciated.

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

Please read our [Contributing Guide](CONTRIBUTING.md) for detailed guidelines.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **[MusicBrainz](https://musicbrainz.org/)** - For providing the comprehensive music database and API
- **Qt Project** - For the excellent cross-platform framework
- **Contributors** - Everyone who has contributed to making this project better

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/your-username/MusicBrainzQt/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-username/MusicBrainzQt/discussions)
- **Email**: [project-email@example.com](mailto:project-email@example.com)

---

**Note**: This application is not officially affiliated with MusicBrainz. It's a community-driven project that uses the MusicBrainz API in accordance with their guidelines.
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