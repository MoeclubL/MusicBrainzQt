# MusicBrainzQt Development Guide

This document provides comprehensive guidelines for developers working on MusicBrainzQt.

## 🏗️ Project Architecture

### Overview

MusicBrainzQt follows a layered architecture pattern with clear separation of concerns:

```
┌─────────────────────────────────────────┐
│                UI Layer                 │
│  (MainWindow, ItemDetailTab, etc.)     │
│  Uses: WidgetHelpers (UI-specific utilities) │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│             Service Layer               │
│   (SearchService, EntityDetailManager) │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│               API Layer                 │
│    (MusicBrainzApi, NetworkManager) │
│   Uses: MusicBrainzUtils (API-only)    │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│            Data Models                  │
│   (ResultItem, ResultTableModel)       │
└─────────────────────────────────────────┘
```

### Key Architectural Principles

1. **Layered Architecture**: Clear separation between UI, business logic, and data access
2. **Utility Separation**: 
   - `WidgetHelpers` for UI-specific operations (formatting, clipboard, URLs)
   - `MusicBrainzUtils` for API-specific operations (entity conversion, URL building)
3. **Centralized Styling**: All visual styling managed through QSS files
4. **Single Responsibility**: Each class has a focused, well-defined purpose
5. **Dependency Direction**: Higher layers depend on lower layers, not vice versa

### Core Components

#### API Layer (`src/api/`)
- **MusicBrainzApi**: Main interface to MusicBrainz web service
- **MusicBrainzParser**: Universal JSON response parser for all entity types
- **MusicBrainzUtils**: Entity type conversion and utilities (API-only)
- **NetworkManager**: HTTP request handling with rate limiting
- Handles rate limiting, authentication, and error management

#### Data Models (`src/models/`)
- **ResultItem**: Unified data container for all entity types
- **ResultTableModel**: Qt model for displaying search results

#### UI Components (`src/ui/`)
- **MainWindow**: Application main window with search interface
- **ItemDetailTab**: Comprehensive detail view for entities
- **EntityListWidget**: Reusable list component for all entity types
- **AdvancedSearchWidget**: Complex search form builder
- **SearchResultTab**: Search results display and management
- **WidgetHelpers**: UI-specific utilities (formatting, clipboard, URLs)

#### Service Layer (`src/services/`)
- **SearchService**: Business logic for search operations
- **EntityDetailManager**: Entity detail retrieval and caching

#### Core Types (`src/core/`)
- **types.h**: Common type definitions and enums
- **error_types.h**: Error handling types

#### Utilities (`src/utils/`)
- **ConfigManager**: Application configuration management
## Core Components

### Logging System
- **QLoggingCategory**: Centralized Qt logging system with categories
- **Core Categories**: musicbrainzApi, mbzService, mbzUi, mbzNetwork, mbzParser
- **Configuration**: Runtime log level control through QT_LOGGING_RULES

#### Services (`src/services/`)
- **SearchService**: Coordinates search operations
- **EntityDetailManager**: Manages entity detail loading and caching

#### Core Types (`src/core/`)
- **types.h**: Core enumerations and type definitions
- **error_types.h**: Error handling structures

#### Utilities (`src/utils/`)
### Error Handling
- **Error Types**: Centralized error definitions in `core/error_types.h`
- **Result Pattern**: Type-safe error handling with Result<T, ErrorInfo>
- **Qt Integration**: Seamless integration with Qt logging categories
- **ConfigManager**: Application configuration management

## 🛠️ Development Setup

### Prerequisites

Ensure you have the following installed:

- **Qt 6.0+** with the following modules:
  - Qt6Core
  - Qt6Widgets  
  - Qt6Network
- **CMake 3.16+** or **qmake**
- **C++17** compatible compiler:
  - GCC 7+ (Linux)
  - Clang 5+ (macOS)
  - MSVC 2019+ (Windows)
  - MinGW-w64 8+ (Windows)

### IDE Setup

#### Qt Creator (Recommended)
1. Open `CMakeLists.txt` or `musicbrainz-qt.pro`
2. Configure your Qt kit
3. Build and run

#### Visual Studio Code
1. Install the CMake and C++ extensions
2. Open the project folder
3. Use CMake: Configure and CMake: Build commands

#### CLion
1. Open the project folder
2. CLion will automatically detect CMakeLists.txt
3. Configure Qt path if needed

### Building

#### Using CMake (Recommended)

```bash
# Debug build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Release build
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

#### Using qmake

```bash
# Debug build
qmake CONFIG+=debug musicbrainz-qt.pro
make

# Release build  
qmake CONFIG+=release musicbrainz-qt.pro
make
```

## 📝 Coding Standards

### C++ Guidelines

#### Naming Conventions
- **Classes**: PascalCase (`MainWindow`, `ResultItem`)
- **Functions/Methods**: camelCase (`searchArtists`, `getEntityDetails`)
- **Variables**: camelCase (`entityType`, `searchQuery`)
- **Member Variables**: m_ prefix (`m_searchService`, `m_currentItem`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_SEARCH_RESULTS`)
- **Enums**: PascalCase values (`EntityType::Artist`)

#### Code Style
```cpp
// Class definition
class MyClass : public QObject
{
    Q_OBJECT
    
public:
    explicit MyClass(QObject *parent = nullptr);
    ~MyClass() override;
    
    // Public methods
    void doSomething();
    
signals:
    void somethingHappened();
    
private slots:
    void onSomethingTriggered();
    
private:
    // Private methods
    void helperMethod();
    
    // Member variables
    QString m_name;
    QSharedPointer<SomeService> m_service;
};
```

#### Modern C++ Features
- Use `auto` for type deduction when appropriate
- Prefer smart pointers (`QSharedPointer`, `std::unique_ptr`)
- Use range-based for loops
- Employ RAII principles
- Use `const` and `constexpr` where possible

### Qt Guidelines

#### Signal-Slot Connections
```cpp
// Prefer new syntax
connect(button, &QPushButton::clicked, 
        this, &MyClass::onButtonClicked);

// For overloaded signals, use explicit casting
connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MyClass::onIndexChanged);
```

#### Memory Management
- Use Qt's parent-child ownership model
- Prefer `QSharedPointer` for shared ownership
- Always set parent for widgets
- Use `deleteLater()` for safe object deletion

#### UI Development
- Always use .ui files for layouts
- Use QSS stylesheets for all styling (no inline C++ styles)
- Apply styles using `setProperty("class", "style-name")`
- Separate UI logic from business logic
- Use Qt's model-view architecture
- Implement proper signal-slot communication

#### Styling Guidelines
- **Never use setStyleSheet()** - Use QSS classes instead
- Define styles in `resources/styles/main.qss`
- Use semantic class names (e.g., `primary-button`, `error-text`)
- Group related styles together in the QSS file
- Use CSS-like selectors for widget-specific styling

## 🎨 Style Management

### QSS Stylesheet System

MusicBrainzQt uses a centralized QSS (Qt Style Sheets) system for all visual styling:

#### Style Architecture
```
resources/styles/
├── main.qss           # Main application stylesheet
└── styles.qrc         # Qt resource file for styles
```

#### Using Styles in Code

**✅ Correct - Use QSS classes:**
```cpp
// Apply a style class to a widget
QPushButton *button = new QPushButton("Click me");
button->setProperty("class", "primary-button");

QLabel *errorLabel = new QLabel("Error message");
errorLabel->setProperty("class", "error-text");

// For complex styling, combine classes
QWidget *card = new QWidget();
card->setProperty("class", "card-container hover-item");
```

**❌ Incorrect - Never use setStyleSheet():**
```cpp
// DON'T DO THIS
button->setStyleSheet("background-color: #007bff; color: white;");
label->setStyleSheet("color: red; font-weight: bold;");
```

#### Common Style Classes

- **Text Styles**: `secondary-text`, `muted-text`, `id-text`, `bold-text`, `error-text`
- **Buttons**: `primary-button`, `secondary-button`
- **Containers**: `card-container`, `info-item`
- **Tables**: `table-label`, `table-value`, `transparent-table`
- **Tags**: `tag-popular`, `tag-common`, `tag-uncommon`, `genre-item`
- **Relationships**: `relationship-item`, `relationship-link`, `relationship-name`
- **Reviews**: `review-section`, `review-item`, `rating-section`

#### Adding New Styles

1. Define the style in `resources/styles/main.qss`:
```css
.my-custom-style {
    background-color: #f8f9fa;
    border: 1px solid #dee2e6;
    border-radius: 4px;
    padding: 8px;
}
```

2. Apply it in your code:
```cpp
widget->setProperty("class", "my-custom-style");
```

3. The QSS file is automatically loaded at application startup via `main.cpp`

## 🧪 Testing

### Unit Tests

Tests are located in the `tests/` directory. Use Qt Test framework:

```cpp
#include <QtTest>
#include "../src/models/resultitem.h"

class TestResultItem : public QObject
{
    Q_OBJECT
    
private slots:
    void testBasicFunctionality();
    void testDataPersistence();
};

void TestResultItem::testBasicFunctionality()
{
    ResultItem item("123", "Test Artist", EntityType::Artist);
    QCOMPARE(item.getId(), "123");
    QCOMPARE(item.getName(), "Test Artist");
}
```

### Running Tests

```bash
# Using CMake
cd build
ctest

# Manual execution
./tests/test_suite
```

## 🐛 Debugging

### Logging System

Use the standard Qt logging functions:

```cpp
#include <QDebug>

// Different log levels
qDebug() << "Debug information";
qInfo() << "General information";
qWarning() << "Warning message";
qCritical() << "Critical error";
```

### Error Handling

Use the simplified error types:

```cpp
#include "core/error_types.h"

// Basic error handling
if (someCondition) {
    qWarning() << "Something went wrong";
    return;
}
```

### Debug Categories
- ``: User interface events
- `logAPI`: MusicBrainz API interactions  
- `logCore`: Core application logic
- `logModel`: Data model operations
- `logNetwork`: Network communications
- `logError`: Error conditions

### Common Debugging Scenarios

#### API Issues
1. Enable API logging: `QLoggingCategory::setFilterRules("musicbrainz.api.debug=true")`
2. Check network connectivity
3. Verify API rate limiting compliance
4. Validate request parameters

#### UI Problems
1. Check signal-slot connections
2. Verify model-view relationships
3. Inspect widget hierarchy
4. Review layout constraints

## 🔧 Development Workflow

### Feature Development

1. **Create Feature Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Write Tests First** (TDD)
   - Write failing tests for new functionality
   - Implement minimal code to pass tests
   - Refactor while keeping tests green

3. **Implement Feature**
   - Follow coding standards
   - Add comprehensive documentation
   - Update relevant .ui files

4. **Test Thoroughly**
   - Run unit tests
   - Test manually across platforms
   - Check for memory leaks

5. **Submit Pull Request**
   - Write clear commit messages
   - Include screenshots for UI changes
   - Reference related issues

### Code Review Guidelines

#### For Authors
- Keep changes focused and atomic
- Write descriptive commit messages
- Include tests for new functionality
- Update documentation as needed

#### For Reviewers
- Check code style compliance
- Verify functionality matches requirements
- Look for potential security issues
- Ensure proper error handling

## 📦 Release Process

### Version Management

We follow semantic versioning (SemVer):
- **MAJOR**: Incompatible API changes
- **MINOR**: New functionality (backwards compatible)
- **PATCH**: Bug fixes (backwards compatible)

### Release Checklist

1. **Update Version Numbers**
   - CMakeLists.txt
   - musicbrainz-qt.pro
   - About dialog

2. **Update Documentation**
   - README.md
   - CHANGELOG.md
   - API documentation

3. **Testing**
   - Run full test suite
   - Manual testing on target platforms
   - Performance benchmarks

4. **Build Packages**
   - Windows installer
   - macOS app bundle
   - Linux packages (AppImage, deb, rpm)

5. **Tag Release**
   ```bash
   git tag -a v1.0.0 -m "Release version 1.0.0"
   git push origin v1.0.0
   ```

## 🚀 Performance Guidelines

### General Principles
- Profile before optimizing
- Cache expensive operations
- Use Qt's model-view for large datasets
- Implement progressive loading for UI

### Specific Optimizations
- Use `QStringLiteral` for string constants
- Prefer `const` references for function parameters
- Implement lazy loading for detailed entity information
- Use Qt's concurrent APIs for background operations

## 🔒 Security Considerations

### API Security
- Never expose API keys in code
- Validate all user inputs
- Sanitize data from external sources
- Implement proper rate limiting

### Data Handling
- Use Qt's secure string handling
- Validate JSON parsing results
- Handle network timeouts gracefully
- Implement proper error propagation

## 📚 Additional Resources

- [Qt Documentation](https://doc.qt.io/)
- [MusicBrainz API Documentation](https://musicbrainz.org/doc/MusicBrainz_API)
- [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines)
- [Qt Coding Style](https://wiki.qt.io/Qt_Coding_Style)

## 🤝 Getting Help

- **GitHub Issues**: Report bugs and request features
- **Discussions**: Ask questions and share ideas
- **Code Reviews**: Get feedback on your contributions
- **Documentation**: Improve and expand these guides

Remember: Good code is not just working code—it's code that others can understand, maintain, and extend.

## 🔧 Development Best Practices

### Utility Layer Usage

**Important**: Use the correct utility layer for your code:

#### UI Code should use `WidgetHelpers`
```cpp
#include "ui/widget_helpers.h"

// ✅ Correct - UI code using UI utilities
void MyWidget::copyToClipboard(const QString &text) {
    WidgetHelpers::copyToClipboard(text);
}

void MyWidget::openUrl(const QString &url) {
    WidgetHelpers::openUrl(url);
}

QString MyWidget::formatDuration(int seconds) {
    return WidgetHelpers::formatDuration(seconds);
}
```

#### API/Business Logic should use `MusicBrainzUtils`
```cpp
#include "api/musicbrainz_utils.h"

// ✅ Correct - API code using API utilities
QString ApiClass::buildEntityUrl(EntityType type, const QString &mbid) {
    return MusicBrainzUtils::buildEntityUrl(type, mbid);
}

QString ApiClass::getIncludes(EntityType type) {
    return MusicBrainzUtils::getDefaultIncludes(type);
}
```

### Entity Type Handling

Use the modern constexpr-based entity type utilities:

```cpp
// Convert enum to string
EntityType type = EntityType::Artist;
QString typeStr = MusicBrainzUtils::entityTypeToString(type); // "artist"

// Convert string to enum
QString typeStr = "release-group";
EntityType type = MusicBrainzUtils::stringToEntityType(typeStr); // EntityType::ReleaseGroup

// Get plural form
QString plural = MusicBrainzUtils::getEntityPluralName(EntityType::Artist); // "artists"
```

### Network Requests

All network requests should go through the `NetworkManager`:

```cpp
#include "api/network_manager.h"

// Create request
QNetworkRequest request(url);
request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);

// Use NetworkManager for rate-limited requests
auto reply = NetworkManager::instance().get(request);
connect(reply, &QNetworkReply::finished, this, &MyClass::onRequestFinished);
```

### Error Handling

Use the standardized error types:

```cpp
#include "core/error_types.h"

// Handle different error types
void handleError(const ApiError &error) {
    switch (error.type) {
        case ApiErrorType::NetworkError:
            // Handle network issues
            break;
        case ApiErrorType::RateLimitExceeded:
            // Handle rate limiting
            break;
        case ApiErrorType::ParseError:
            // Handle JSON parsing errors
            break;
    }
}
```

### JSON Parsing

Use the universal `MusicBrainzParser` for all entity types:

```cpp
#include "api/musicbrainzparser.h"

// Parse any entity type
QJsonObject jsonObj = // ... from API response
ResultItem item = MusicBrainzParser::parseEntity(jsonObj, EntityType::Artist);
```
