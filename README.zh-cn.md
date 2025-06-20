# MusicBrainzQt

[English Version](README.md)

一个现代化的跨平台桌面应用程序，用于浏览和搜索 MusicBrainz 音乐数据库。基于 Qt 6 构建，MusicBrainzQt 为探索音乐元数据提供了原生桌面体验。

## 🎵 什么是 MusicBrainzQt？

MusicBrainzQt 是一个连接到 [MusicBrainz](https://musicbrainz.org/) 数据库的桌面客户端，允许您：

- **搜索**艺术家、专辑、录音和其他音乐实体
- **浏览**包含详细元数据的音乐详细信息
- **探索**不同音乐实体之间的关系
- **查看**标签、别名和社区贡献的信息
- **导航**通过直观、现代的界面

## 🚀 致最终用户

### 快速入门

1.  从 [发布页面](https://github.com/MoeclubL/MusicBrainzQt/releases) **下载** 最新版本
2.  **安装** 并运行 MusicBrainzQt
3.  使用搜索栏 **搜索** 您最喜爱的艺术家或专辑
4.  双击任何结果以 **探索** 详细信息

### 功能特性

* **通用搜索**：查找艺术家、专辑、录音、作品等
* **丰富的详情视图**：为每个实体提供全面的信息选项卡
* **关系浏览器**：发现音乐实体之间的联系
* **标签浏览器**：查看社区标签和元数据
* **现代化的用户界面**：遵循平台惯例的简洁、响应式界面
* **高性能**：高效的缓存和优化的 API 使用

### 系统要求

* **Windows**：Windows 10 或更高版本
* **macOS**：macOS 10.15 或更高版本
* **Linux**：大多数现代发行版
* **网络**：访问 MusicBrainz API 需要互联网连接

## 💻 致开发者

### 从源码构建

#### 前提条件

* Qt 6.0+ (Core, Widgets, Network 模块)
* C++17 兼容的编译器
* CMake 3.16+ 或 qmake
* Git

#### 克隆与构建

```
# 克隆仓库
git clone [https://github.com/MoeclubL/MusicBrainzQt.git](https://github.com/MoeclubL/MusicBrainzQt.git)
cd MusicBrainzQt

# 使用 CMake (推荐)
mkdir build && cd build
cmake ..
cmake --build .

# 使用 qmake (备选方案)
qmake musicbrainz-qt.pro
make
```

#### 项目结构

```
MusicBrainzQt/
├── src/                    # 源代码
│   ├── api/               # MusicBrainz API 集成
│   ├── models/            # 数据模型
│   ├── ui/                # UI 组件
│   ├── services/          # 业务逻辑服务
│   ├── core/              # 核心类型和实用工具
│   └── utils/             # 辅助工具
├── ui/                    # Qt Designer UI 文件
├── resources/             # 应用程序资源
├── tests/                 # 单元测试
└── docs/                  # 文档
```

### 架构

MusicBrainzQt 遵循现代 Qt 最佳实践：

* **分层架构**：API、业务逻辑和 UI 之间清晰分离
* **基于组件的 UI**：可复用的、与 .ui 文件分离的控件
* **信号-槽通信**：使用 Qt 的事件系统实现松耦合
* **智能指针**：现代 C++ 内存管理
* **异步操作**：带有适当错误处理的非阻塞 API 请求

## 📚 文档

* [**开发指南**](docs/DEVELOPMENT.md) - 详细的开发设置和指南
* [**API 文档**](docs/API.md) - MusicBrainz API 集成细节
* [**贡献指南**](CONTRIBUTING.md) - 如何为项目做贡献
* [**架构概览**](docs/ARCHITECTURE.md) - 技术架构文档

## 🤝 贡献

我们欢迎社区的贡献！无论是修复错误、添加功能还是改进文档，我们都非常感谢您的帮助。

1.  **Fork** 本仓库
2.  **创建** 一个功能分支 (`git checkout -b feature/amazing-feature`)
3.  **提交** 您的更改 (`git commit -m 'Add amazing feature'`)
4.  **推送** 到该分支 (`git push origin feature/amazing-feature`)
5.  **开启** 一个 Pull Request

请阅读我们的[贡献指南](CONTRIBUTING.md)以获取详细指南。

## 📄 许可证

本项目采用 MIT 许可证 - 详情请参阅 [LICENSE](LICENSE) 文件。

## 🙏 致谢

* [**MusicBrainz**](https://musicbrainz.org/) - 提供了全面的音乐数据库和 API
* **Qt Project** - 提供了优秀的跨平台框架
* **贡献者们** - 每一位为使这个项目变得更好而做出贡献的人

## 📞 支持

* **问题反馈**: [GitHub Issues](https://github.com/MoeclubL/MusicBrainzQt/issues)
* **讨论区**: [GitHub Discussions](https://github.com/MoeclubL/MusicBrainzQt/discussions)
* **电子邮件**: [moecaa@telecom.moe](mailto:moecaa@telecom.moe)

[English Version](README.md)

**请注意**：本应用程序与 MusicBrainz 官方并无关联。这是一个社区驱动的项目，根据 MusicBrainz 的指导方针使用其 API。

**AI 代码声明**：本项目的部份代码是在 AI 工具的协助下开发的，以加速开发进程和提高代码质量。所有由 AI 生成的代码都经过了审查、测试，并按照项目的编码标准进行了整合。
