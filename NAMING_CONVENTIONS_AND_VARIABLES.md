# MusicBrainzQt 命名规范与项目架构

## 重要更新 (最新)

### 2025年6月19日 - 缓存禁用与API现代化
- **缓存功能完全禁用**：EntityDetailManager中的m_detailsCache不再使用，所有数据从服务器实时获取
- **utils/cache.h/.cpp已删除**：移除了不再使用的缓存工具类
- **LibMusicBrainzApi引入**：创建新的API适配器(src/api/libmusicbrainzapi.h/.cpp)，作为向官方libmusicbrainz库迁移的第一步
- **SearchService/EntityDetailManager更新**：现在使用LibMusicBrainzApi替代旧的MusicBrainzApi
- **临时文件清理**：删除所有*_old.cpp、*_new.cpp、*_clean.cpp临时文件
- **Results per page功能已生效**：分页逻辑已正确实现并测试

## 项目结构

```
MusicBrainzQt/
├── src/                    # 源代码目录
│   ├── core/              # 核心类型和错误处理
│   │   ├── types.h        # EntityType 等核心类型定义
│   │   └── error_types.h  # ErrorInfo, Result<T> 等错误处理
│   ├── api/               # MusicBrainz API 相关
│   │   ├── musicbrainzapi.h/.cpp (旧实现，待替换)
│   │   ├── libmusicbrainzapi.h/.cpp (新实现，基于官方API)
│   │   ├── musicbrainzrequestmanager.h/.cpp
│   │   ├── musicbrainzurlbuilder.h/.cpp
│   │   ├── musicbrainztypes.h
│   │   └── networkrequest.h/.cpp
│   ├── models/            # 数据模型
│   │   ├── resultitem.h/.cpp
│   │   ├── resulttablemodel.h/.cpp
│   │   └── (旧模型文件待清理)
│   ├── services/          # 业务服务
│   │   ├── searchservice.h/.cpp
│   │   └── entitydetailmanager.h/.cpp
│   ├── ui/                # UI 组件
│   │   ├── advancedsearchwidget.h/.cpp
│   │   ├── searchresulttab.h/.cpp
│   │   ├── itemdetailtab.h/.cpp
│   │   └── entitylistwidget.h/.cpp
│   ├── utils/             # 工具类
│   │   ├── logger.h/.cpp
│   │   ├── config_manager.h/.cpp
│   │   ├── cache.h/.cpp
│   │   └── settings.h/.cpp
│   ├── main.cpp
│   ├── mainwindow.h/.cpp
├── ui/                    # Qt Designer UI 文件
│   ├── mainwindow.ui
│   ├── itemdetailtab.ui
│   ├── entitylistwidget.ui
│   ├── advancedsearchwidget.ui
│   └── searchresulttab.ui
├── resources/             # 资源文件
│   ├── images.qrc
│   └── icons/
└── tests/                 # 测试文件
```

## 命名规范

### 类名 (Classes)
- **格式**: PascalCase
- **示例**: `ResultTableModel`, `ItemDetailTab`, `MusicBrainzApi`

### 变量名 (Variables)
- **成员变量**: `m_` 前缀 + camelCase
- **局部变量**: camelCase
- **静态变量**: `s_` 前缀 + camelCase  
- **常量**: `k` 前缀 + PascalCase 或全大写 + 下划线

### 函数/方法名 (Functions/Methods)
- **格式**: camelCase
- **示例**: `getAvailableColumns`, `setItems`, `populateSubTabs`

### 枚举 (Enums)
- **枚举类型**: PascalCase
- **枚举值**: PascalCase
- **示例**: `EntityType::Artist`, `EntityType::Release`

### 文件名 (Files)
- **头文件**: lowercase + .h
- **源文件**: lowercase + .cpp
- **UI文件**: lowercase + .ui

## 核心类型定义

### EntityType (src/core/types.h)
```cpp
enum class EntityType {
    Artist,         // 艺术家
    Release,        // 发行版  
    ReleaseGroup,   // 发行组
    Recording,      // 录音
    Work,           // 作品
    Label,          // 厂牌
    Area,           // 地区
    Unknown         // 未知类型
};
```

## 主要类成员变量

### MainWindow (src/mainwindow.h)
```cpp
private:
    Ui::MainWindow *ui;                                    // UI 对象
    SearchService *m_searchService;                        // 搜索服务
    QDockWidget *m_searchDock;                            // 搜索停靠窗口
    AdvancedSearchWidget *m_searchWidget;                 // 高级搜索组件
    QTabWidget *m_mainTabWidget;                          // 主标签页组件
    SearchParameters m_currentSearchParams;               // 当前搜索参数
    QMap<QString, ItemDetailTab*> m_itemDetailTabs;       // 详细信息标签页映射
```

### ResultTableModel (src/models/resulttablemodel.h)
```cpp
private:
    QList<QSharedPointer<ResultItem>> m_items;            // 数据项列表
    EntityType m_type;                                     // 当前实体类型
    QList<ColumnInfo> m_visibleColumns;                   // 当前显示的列
```

### ResultItem (src/models/resultitem.h)
```cpp
protected:
    QString m_id;                                          // MusicBrainz ID
    QString m_name;                                        // 实体名称
    EntityType m_type;                                     // 实体类型
    QVariantMap m_detailData;                             // 详细数据
```

### ItemDetailTab (src/ui/itemdetailtab.h)
```cpp
private:
    Ui::ItemDetailTab *ui;                                 // UI 对象
    QSharedPointer<ResultItem> m_item;                     // 当前项目
    QHash<QString, EntityListWidget*> m_subTabWidgets;    // 子标签页组件映射
    QStringList m_subTabKeys;                              // 子标签页键名列表
```

### AdvancedSearchWidget (src/ui/advancedsearchwidget.h)
```cpp
private:
    Ui_AdvancedSearchWidget *ui;                           // UI 对象
    QWidget *m_fieldsWidget;                               // 高级搜索字段容器
    QGridLayout *m_fieldsLayout;                           // 字段布局
    QMap<QString, QWidget*> m_fields;                      // 字段映射
    EntityType m_currentType;                              // 当前实体类型
```

### EntityListWidget (src/ui/entitylistwidget.h)
```cpp
private:
    Ui::EntityListWidget *ui;                              // UI 对象
    ResultTableModel *m_model;                             // 表格模型
    QMenu *m_contextMenu;                                  // 右键菜单
    QAction *m_openInBrowserAction;                        // 在浏览器打开动作
    QAction *m_copyIdAction;                               // 复制ID动作
    QAction *m_columnSettingsAction;                       // 列设置动作
```

### SearchResultTab (src/ui/searchresulttab.h) - 重构后
```cpp
private:
    Ui_SearchResultTab *ui;                                // UI 对象（简化版）
    EntityListWidget *m_entityListWidget;                 // 核心列表组件
    QString m_query;                                       // 查询字符串
    EntityType m_entityType;                               // 实体类型
    SearchResults m_currentStats;                          // 当前搜索统计
    EntityDetailManager *m_detailManager;                  // 详细信息管理器
```

### EntityDetailManager (src/services/entitydetailmanager.h)
```cpp
private:
    QMap<QString, QVariantMap> m_detailsCache;             // 详细信息缓存
    QList<DetailRequest> m_batchQueue;                     // 批量加载队列
    QStringList m_currentBatch;                            // 当前批次
    QSet<QString> m_loadingItems;                          // 正在加载的项目
    int m_batchLoadedCount;                                // 已加载计数
    LoadingStatistics m_stats;                             // 加载统计
    QSharedPointer<MusicBrainzApi> m_api;                 // API接口
```

## 架构设计原则

### UI 组件设计
- **统一组件**: EntityListWidget 用于所有列表显示（搜索结果、详情子列表）
- **UI 文件优先**: 所有布局使用 .ui 文件，减少代码中的 UI 操作
- **分离关注点**: UI 逻辑与业务逻辑分离

### 数据模型设计
- **动态列检测**: ResultTableModel 支持根据数据内容自动检测和显示列
- **统一数据模型**: ResultItem 统一承载所有实体类型的数据
- **详细信息管理**: EntityDetailManager 统一管理详细信息的加载和缓存

### 错误处理
- **结构化错误**: 使用 ErrorInfo 统一错误信息格式
- **Result 模式**: 使用 Result<T> 模式处理可能失败的操作

## 开发规范

### 添加新功能前检查
1. 检查变量命名是否符合规范 (`m_` 前缀等)
2. 确认是否有同名变量已存在
3. 更新本文档的相关部分
4. 确保使用统一的组件和模型

### 变量命名检查清单
- [ ] 成员变量使用 `m_` 前缀
- [ ] 私有成员变量在 private 部分
- [ ] 变量名使用 camelCase
- [ ] 避免缩写，使用完整的描述性名称
- [ ] UI 组件名体现功能和位置

### 代码质量要求
- 使用现代 C++/Qt 特性
- 优先使用 QSharedPointer 管理对象生命周期
- 统一的日志记录 (logUI, logAPI 等)
- 完善的错误处理和用户反馈

## 待清理项目

### ✅ 已删除的旧文件
以下文件已成功删除：
- ~~src/models/album.h/.cpp~~ (已删除，已由 ResultItem 替代)
- ~~src/models/artist.h/.cpp~~ (已删除，已由 ResultItem 替代)  
- ~~src/models/release.h/.cpp~~ (已删除，已由 ResultItem 替代)
- ~~src/models/track.h/.cpp~~ (已删除，已由 ResultItem 替代)
- ~~src/utils/settings.h/.cpp~~ (已删除，已由 config_manager 替代)

### 🔧 已修复的UI问题
- ✅ EntityListWidget 右键菜单位置问题已修复
- ✅ ItemDetailTab 与 MainWindow 的信号连接已修复
- ✅ 详细信息加载调试信息已增强

### 变量命名规范化
- 确保所有成员变量都有 `m_` 前缀
- 检查是否有违反 camelCase 的变量名
- 统一UI组件的命名方式

## 命名规范检查结果

### ✅ 已修复的问题
- `SearchResultTab::m_EntityType` → `m_entityType` (修复PascalCase违规)

### ⚠️ 项目清理状态
~~以下文件已不再被使用，建议删除~~：
```bash
# 旧的数据模型文件 (已被 ResultItem 统一替代) - ✅ 已删除
# rm src/models/album.h src/models/album.cpp
# rm src/models/artist.h src/models/artist.cpp  
# rm src/models/release.h src/models/release.cpp
# rm src/models/track.h src/models/track.cpp

# 旧的设置文件 (已被 config_manager 替代) - ✅ 已删除  
# rm src/utils/settings.h src/utils/settings.cpp
```

### ✅ 最新修复项目
- **右键菜单位置修复**: EntityListWidget 的右键菜单现在正确显示在鼠标位置
- **详情页信号连接**: ItemDetailTab 的 openDetailTab 信号已连接到 MainWindow
- **调试信息增强**: 详细信息加载过程增加了完整的调试日志
- **UI文件化改造**: AdvancedSearchWidget 和 SearchResultTab 已改为使用 .ui 文件设计
- **配置文件更新**: 项目配置文件已更新包含新的 UI 文件

### 🎯 UI设计原则落实
- ✅ 所有UI组件均使用 .ui 文件设计（MainWindow, ItemDetailTab, EntityListWidget, AdvancedSearchWidget, SearchResultTab）
- ✅ 代码中不再有大量UI创建逻辑，只保留信号连接和数据绑定
- ✅ 遵循关注点分离原则，UI逻辑与业务逻辑彻底分离

### ✅ 命名规范检查通过
经检查，项目中的变量命名基本符合规范：
- 所有成员变量都正确使用 `m_` 前缀
- 变量名都使用 camelCase 格式
- 类名使用 PascalCase 格式
- 文件名使用 lowercase 格式
