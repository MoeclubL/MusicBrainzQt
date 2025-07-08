# MusicBrainzQt

A cross-platform desktop application for browsing and searching the MusicBrainz music database. Built with Qt 6, MusicBrainzQt provides a native desktop experience for exploring music metadata.

## 🎵 Features

- **Search** for artists, albums, recordings, and other music entities
- **Browse** detailed information about music with comprehensive metadata
- **Explore** relationships between different music entities  
- **View** tags, aliases, and community-contributed information
- **Navigate** through an intuitive, modern interface

## 💻 Quick Start

### Prerequisites

- **Qt 6.x** (Core, Widgets, Network modules)
- **C++17** compatible compiler
- **CMake 3.16+**

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/MoeclubL/MusicBrainzQt.git
cd MusicBrainzQt

# Build with CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

### Recent Improvements 🚀

- **🏗️ Unified API Architecture**: Modular design with 20% code reduction
- **🎨 QSS Styling System**: Centralized theme management for consistent UI
- **🌐 Multi-language Support**: English and Chinese interface
- **📦 Modern CMake**: Updated build system with testing integration

## 📁 Project Structure

```
MusicBrainzQt/
├── src/                    # Source code
│   ├── api/               # MusicBrainz API integration
│   ├── models/            # Data models
│   ├── ui/                # UI components (QSS-styled)
│   ├── services/          # Business logic services
│   ├── core/              # Core types and utilities
│   └── utils/             # Helper utilities
├── ui/                    # Qt Designer UI files
├── resources/             # Application resources
├── tests/                 # Unit tests
└── docs/                  # Documentation
```

## ✅ Roadmap

- [ ] User authentication for MusicBrainz API
- [ ] Enhanced search with advanced filtering
- [ ] Local caching for offline experience
- [ ] Integration with Last.fm/Spotify
- [ ] Comprehensive settings page

## 🤝 Contributing

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

-   **[MusicBrainz](https://musicbrainz.org/)** - For providing the comprehensive music database and API
-   **Qt Project** - For the excellent cross-platform framework
-   **Contributors** - Everyone who has contributed to making this project better

---

**Note**: This application is not officially affiliated with MusicBrainz. It's a community-driven project that uses the MusicBrainz API in accordance with their guidelines.