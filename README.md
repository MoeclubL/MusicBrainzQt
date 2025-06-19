# MusicBrainzQt

[中文版本](README.zh-cn.md)

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
git clone https://github.com/MoeclubL/MusicBrainzQt.git
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

- **Issues**: [GitHub Issues](https://github.com/MoeclubL/MusicBrainzQt/issues)
- **Discussions**: [GitHub Discussions](https://github.com/MoeclubL/MusicBrainzQt/discussions)
- **Email**: [moecaa@telecom.moe](mailto:moecaa@telecom.moe)

---

[中文版本](README.zh-cn.md)

**Note**: This application is not officially affiliated with MusicBrainz. It's a community-driven project that uses the MusicBrainz API in accordance with their guidelines.

**AI Code Notice**: Portions of this project were developed with assistance from AI tools to accelerate development and improve code quality. All AI-generated code has been reviewed, tested, and integrated following the project's coding standards.