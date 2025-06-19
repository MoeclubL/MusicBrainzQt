# Contributing to MusicBrainzQt

Thank you for your interest in contributing to MusicBrainzQt! This document provides guidelines and information for contributors.

## 🤝 Ways to Contribute

### Code Contributions
- **Bug Fixes**: Fix reported issues
- **New Features**: Implement requested functionality
- **Performance Improvements**: Optimize existing code
- **Code Refactoring**: Improve code quality and maintainability

### Non-Code Contributions
- **Documentation**: Improve or expand documentation
- **Testing**: Write tests or report bugs
- **Translation**: Localize the application
- **Design**: UI/UX improvements and suggestions

## 🚀 Getting Started

### 1. Fork and Clone

```bash
# Fork the repository on GitHub
# Then clone your fork
git clone https://github.com/YOUR_USERNAME/MusicBrainzQt.git
cd MusicBrainzQt

# Add upstream remote
git remote add upstream https://github.com/ORIGINAL_OWNER/MusicBrainzQt.git
```

### 2. Set Up Development Environment

Follow the [Development Guide](docs/DEVELOPMENT.md) to set up your development environment.

### 3. Create a Feature Branch

```bash
# Create and switch to a new branch
git checkout -b feature/your-feature-name

# Or for bug fixes
git checkout -b fix/issue-description
```

## 📝 Development Workflow

### 1. Planning Your Contribution

#### For Bug Fixes
1. Check if the issue is already reported
2. If not, create an issue first
3. Discuss the approach in the issue comments
4. Reference the issue in your PR

#### For New Features
1. Create a feature request issue
2. Wait for maintainer approval
3. Discuss implementation details
4. Start development after agreement

### 2. Writing Code

#### Code Style Guidelines

Follow the established code style:

```cpp
// Class definition example
class MyClass : public QObject
{
    Q_OBJECT
    
public:
    explicit MyClass(QObject *parent = nullptr);
    ~MyClass() override;
    
    // Public methods
    void publicMethod();
    QString getValue() const;
    
signals:
    void valueChanged(const QString &newValue);
    
public slots:
    void setValue(const QString &value);
    
private slots:
    void onSomethingHappened();
    
private:
    // Private methods
    void helperMethod();
    
    // Member variables (m_ prefix)
    QString m_value;
    QSharedPointer<SomeService> m_service;
};
```

#### Naming Conventions
- **Classes**: PascalCase (`SearchService`)
- **Methods**: camelCase (`searchEntity`)
- **Variables**: camelCase (`searchQuery`)
- **Member Variables**: m_ prefix (`m_searchService`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_RESULTS`)

#### Documentation
- Add Doxygen comments for public APIs
- Include inline comments for complex logic
- Update relevant documentation files

### 3. Testing

#### Writing Tests
```cpp
#include <QtTest>
#include "your_class.h"

class TestYourClass : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase();
    void testBasicFunctionality();
    void testErrorHandling();
    void cleanupTestCase();
    
private:
    YourClass *m_testObject;
};

void TestYourClass::testBasicFunctionality()
{
    // Arrange
    QString input = "test input";
    QString expected = "expected output";
    
    // Act
    QString result = m_testObject->processInput(input);
    
    // Assert
    QCOMPARE(result, expected);
}
```

#### Running Tests
```bash
# Using CMake
cd build
ctest

# Using qmake
cd tests
qmake tests.pro
make
./test_runner
```

### 4. UI Development

#### Using Qt Designer
- Always use .ui files for layouts
- Separate UI logic from business logic
- Follow platform UI guidelines
- Test on multiple screen sizes

#### Example UI Integration
```cpp
// In constructor
ui->setupUi(this);

// Connect signals
connect(ui->searchButton, &QPushButton::clicked,
        this, &MyWidget::onSearchClicked);

// Update UI in separate method
void MyWidget::updateSearchResults(const QList<Result> &results)
{
    ui->resultsList->clear();
    for (const auto &result : results) {
        ui->resultsList->addItem(result.name);
    }
}
```

## 🧪 Testing Guidelines

### Test Categories

#### Unit Tests
- Test individual classes in isolation
- Mock external dependencies
- Cover edge cases and error conditions

#### Integration Tests
- Test component interactions
- Verify data flow between layers
- Test API integration with mock responses

#### UI Tests
- Test user interactions
- Verify UI state changes
- Test keyboard and mouse events

### Coverage Expectations
- New code should have >80% test coverage
- Critical path code should have >95% coverage
- Include both positive and negative test cases

## 📋 Commit Guidelines

### Commit Message Format

```
type(scope): brief description

Detailed explanation of changes (if needed)

Fixes #issue_number
```

#### Types
- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style changes (no functional changes)
- **refactor**: Code refactoring
- **test**: Adding or updating tests
- **chore**: Maintenance tasks

#### Examples
```bash
feat(search): add advanced search functionality

Implement advanced search with multiple criteria:
- Artist type filtering
- Date range selection
- Country filtering

Fixes #123

fix(api): handle network timeout gracefully

Add proper error handling for network timeouts
and show user-friendly error messages.

Fixes #456
```

### Making Commits

```bash
# Stage your changes
git add .

# Commit with descriptive message
git commit -m "feat(ui): improve search result display"

# Push to your branch
git push origin feature/your-feature-name
```

## 🔍 Code Review Process

### Before Submitting PR

#### Self-Review Checklist
- [ ] Code follows project style guidelines
- [ ] All tests pass
- [ ] New functionality has tests
- [ ] Documentation is updated
- [ ] No debug code or console logs
- [ ] Performance impact considered
- [ ] Memory leaks checked

#### Creating Pull Request

1. **Update your branch**:
```bash
git fetch upstream
git rebase upstream/main
```

2. **Push final changes**:
```bash
git push origin feature/your-feature-name
```

3. **Create PR on GitHub**:
   - Use descriptive title
   - Fill out PR template
   - Link related issues
   - Add screenshots for UI changes

### Review Process

#### What Reviewers Look For
- **Functionality**: Does it work as intended?
- **Code Quality**: Is it readable and maintainable?
- **Performance**: Any performance implications?
- **Security**: Any security concerns?
- **Tests**: Adequate test coverage?
- **Documentation**: Is documentation updated?

#### Review Timeline
- Initial review within 48 hours
- Follow-up reviews within 24 hours
- Complex PRs may take longer

#### Addressing Review Feedback

```bash
# Make requested changes
# ... edit files ...

# Commit changes
git add .
git commit -m "address review feedback: fix error handling"

# Push updates
git push origin feature/your-feature-name
```

## 🐛 Bug Reports

### Before Reporting

1. **Search existing issues**
2. **Check if it's already fixed** in the latest version
3. **Try to reproduce** the issue consistently

### Bug Report Template

```markdown
**Bug Description**
Clear description of what the bug is.

**Steps to Reproduce**
1. Go to '...'
2. Click on '....'
3. Scroll down to '....'
4. See error

**Expected Behavior**
What you expected to happen.

**Actual Behavior**
What actually happened.

**Screenshots**
If applicable, add screenshots.

**Environment**
- OS: [e.g., Windows 10, macOS 12, Ubuntu 20.04]
- Qt Version: [e.g., 6.2.0]
- Application Version: [e.g., 1.0.0]

**Additional Context**
Any other context about the problem.
```

## 💡 Feature Requests

### Before Requesting

1. **Check existing feature requests**
2. **Consider if it fits** the project scope
3. **Think about implementation** complexity

### Feature Request Template

```markdown
**Feature Description**
Clear description of the feature you'd like to see.

**Use Case**
Explain why this feature would be useful.

**Proposed Implementation**
If you have ideas on how to implement this.

**Alternatives Considered**
Other solutions you've considered.

**Additional Context**
Screenshots, mockups, or other context.
```

## 🌐 Translation

### Adding New Languages

1. **Create translation file**:
```bash
# In project root
lupdate musicbrainz-qt.pro
# This creates/updates .ts files
```

2. **Translate strings**:
   - Use Qt Linguist to edit .ts files
   - Translate all strings completely
   - Test with the target language

3. **Submit translation**:
   - Create PR with translated .ts file
   - Include information about your native language level

### Updating Existing Translations

1. **Update source strings**:
```bash
lupdate musicbrainz-qt.pro
```

2. **Translate new/changed strings**
3. **Submit PR with updates**

## 📚 Documentation

### Types of Documentation

#### Code Documentation
- Doxygen comments for public APIs
- Inline comments for complex logic
- README updates for new features

#### User Documentation
- User guides and tutorials
- Feature documentation
- Troubleshooting guides

#### Developer Documentation
- Architecture documentation
- API documentation
- Development guides

### Documentation Standards

```cpp
/**
 * @brief Searches for entities using the MusicBrainz API
 * @param entityType Type of entity to search for
 * @param query Search query string
 * @param limit Maximum number of results to return
 * @param offset Number of results to skip
 * @return True if search was initiated successfully
 * 
 * This method performs an asynchronous search operation.
 * Results are returned via the searchCompleted signal.
 * 
 * @see searchCompleted()
 * @see searchError()
 */
bool searchEntity(EntityType entityType, const QString &query, int limit = 25, int offset = 0);
```

## 🎯 Issue Triage

### Issue Labels

- **bug**: Something isn't working
- **enhancement**: New feature or request
- **documentation**: Improvements or additions to documentation
- **good first issue**: Good for newcomers
- **help wanted**: Extra attention is needed
- **priority-high**: High priority issue
- **priority-low**: Low priority issue

### Milestones

Issues are organized into milestones representing:
- Release versions (v1.0.0, v1.1.0, etc.)
- Feature themes (Search Improvements, UI Enhancements)
- Bug fix cycles

## 🏆 Recognition

### Contributors

All contributors are recognized in:
- **README.md**: Contributors section
- **About Dialog**: Application credits
- **Release Notes**: Major contributions highlighted

### Contribution Types

We value all types of contributions:
- 💻 Code contributions
- 📖 Documentation improvements
- 🐛 Bug reports and testing
- 🌐 Translations
- 🎨 Design and UI improvements
- 💡 Feature suggestions and feedback

## 📞 Getting Help

### Communication Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: General questions and discussions
- **Pull Request Reviews**: Code-specific discussions

### Response Times

- **Issues**: Acknowledged within 48 hours
- **Pull Requests**: Initial review within 48 hours
- **Questions**: Responses within 24-48 hours

### Code of Conduct

This project follows the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). Please read it before participating.

## 🔄 Release Process

### Version Management

We follow [Semantic Versioning](https://semver.org/):
- **MAJOR.MINOR.PATCH**
- **MAJOR**: Breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes (backward compatible)

### Release Cycle

- **Patch releases**: Monthly (bug fixes)
- **Minor releases**: Quarterly (new features)
- **Major releases**: Yearly (major changes)

---

Thank you for contributing to MusicBrainzQt! Your contributions help make this project better for everyone. 🎵
