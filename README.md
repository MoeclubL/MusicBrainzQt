# MusicBrainzQt

A cross-platform desktop application for browsing and searching the MusicBrainz music database. Built with Qt 6, MusicBrainzQt provides a native desktop experience for exploring music metadata.

## 🎵 What is MusicBrainzQt?

MusicBrainzQt is a desktop client that connects to the [MusicBrainz](https://musicbrainz.org/) database, allowing you to:

- **Search** for artists, albums, recordings, and other music entities
- **Browse** detailed information about music with comprehensive metadata
- **Explore** relationships between different music entities
- **View** tags, aliases, and community-contributed information
- **Navigate** through an intuitive, modern interface

## 🚀 For End Users

### Quick Start

1. **Download** the latest release from the [Releases](https://github.com/MoeclubL/MusicBrainzQt/releases) page
2. **Install** and run MusicBrainzQt
3. **Search** for your favorite artists or albums using the search bar
4. **Explore** detailed information by double-clicking on any result

### Features

- **Universal Search**: Find artists, albums, recordings, works, and more
- **Rich Detail Views**: Comprehensive information tabs for each entity
- **Relationship Explorer**: Discover connections between music entities  
- **Tag Browser**: View community tags and metadata
- **Modern UI**: Clean, responsive interface following platform conventions
- **Multi-language Support**: English and Chinese (Simplified) interface
- **Cross-platform**: Native experience on Windows, macOS, and Linux
- **Fast Performance**: Efficient caching and optimized API usage

### System Requirements

- **Windows**: Windows 10 or later
- **macOS**: macOS 10.15 or later  
- **Linux**: Most modern distributions
- **Internet**: Required for MusicBrainz API access

## 💻 For Developers

### Building from Source

#### Prerequisites

- **Qt Creator** (recommended IDE)
- **Qt 5.15+ or 6.x** (Core, Widgets, Network modules)
- **C++17** compatible compiler
- **Git** for cloning the repository

#### Quick Build

```bash
# Clone the repository
git clone https://github.com/MoeclubL/MusicBrainzQt.git
cd MusicBrainzQt

# Using CMake (Recommended)
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel

# Using Qt Creator
# File -> Open File or Project -> select CMakeLists.txt
# Build -> Build Project (Ctrl+B)
```

For detailed build instructions, see [BUILD_DEPLOYMENT_GUIDE.md](docs/BUILD_DEPLOYMENT_GUIDE.md).

### Recent Optimizations 🚀

MusicBrainzQt has undergone major improvements in both API architecture and UI styling:

#### API Architecture Refactoring
- **🏗️ Unified Architecture**: Modular tool classes replace scattered code
- **📉 Code Reduction**: 20.6% reduction in core API code with 90% less duplication
- **⚡ Performance**: Optimized URL building, response parsing, and memory usage
- **🔧 Maintainability**: New API types require only 1-2 file changes vs 5-7 previously
- **✨ Extended Features**: Added 7 new API methods (DiscID lookup, collections, browsing)

#### Style Management Modernization
- **🎨 Unified QSS Styling**: Migrated from C++ inline styles to centralized QSS stylesheet
- **📁 Simplified Structure**: Removed empty directories and unused components
- **🔄 Consistent Theming**: All UI components now use consistent style classes
- **🛠️ Improved Maintainability**: Single stylesheet file for all application styling
- **⚡ Better Performance**: Reduced style computation overhead

#### Internationalization Support
- **🌐 Multi-language Interface**: Full support for English and Chinese (Simplified)
- **🔄 Automatic Language Detection**: Detects system language and loads appropriate translations
- **📝 Complete Translation Coverage**: All UI elements and messages are translatable
- **🛠️ Developer-friendly**: Easy to add new languages with Qt Linguist tools

#### Enhanced Build System
- **📦 Modern CMake**: Updated to use CMake best practices for Qt6
- **🧪 Integrated Testing**: Comprehensive unit test framework with CTest integration
- **🔧 Development Tools**: Improved debugging and profiling capabilities
- **📚 Documentation**: Extensive build and deployment guides

For technical details, see:
- [docs/PROJECT_ANALYSIS_REPORT.md](docs/PROJECT_ANALYSIS_REPORT.md)
- [docs/BUILD_DEPLOYMENT_GUIDE.md](docs/BUILD_DEPLOYMENT_GUIDE.md)
- [docs/API_OPTIMIZATION_ANALYSIS.md](docs/API_OPTIMIZATION_ANALYSIS.md)
- [docs/REFACTORING_REPORT.md](docs/REFACTORING_REPORT.md)

#### Project Structure

```
MusicBrainzQt/
├── src/                    # Source code
│   ├── api/               # MusicBrainz API integration (optimized)  
│   │   ├── musicbrainzapi.cpp/h           # Core API client
│   │   ├── musicbrainz_response_handler.cpp/h  # Response processing
│   │   ├── musicbrainzparser.cpp/h        # JSON parsing utilities
│   │   ├── network_manager.cpp/h          # Network layer
│   │   └── api_utils.cpp/h               # API helper utilities
│   ├── models/            # Data models
│   ├── ui/                # UI components (QSS-styled)
│   ├── services/          # Business logic services
│   ├── core/              # Core types and utilities
│   │   ├── types.h                       # Core type definitions
│   │   ├── error_types.h                 # Error handling types
│   │   └── logging.cpp                   # Qt logging categories
│   └── utils/             # Helper utilities (config management)
├── ui/                    # Qt Designer UI files
├── resources/             # Application resources
│   ├── images.qrc        # Image resources
│   └── styles/           # QSS stylesheets
│       ├── main.qss      # Main application stylesheet
│       └── styles.qrc    # Style resources
├── tests/                 # Unit tests
└── docs/                  # Documentation
```

### Architecture

MusicBrainzQt follows modern Qt best practices:

- **Layered Architecture**: Clear separation between API, business logic, and UI
- **Component-Based UI**: Reusable widgets with .ui file separation
- **Centralized Styling**: QSS-based theme system for consistent appearance
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

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **[MusicBrainz](https://musicbrainz.org/)** - For providing the comprehensive music database and API
- **Qt Project** - For the excellent cross-platform framework
- **Contributors** - Everyone who has contributed to making this project better

---

**Note**: This application is not officially affiliated with MusicBrainz. It's a community-driven project that uses the MusicBrainz API in accordance with their guidelines.
