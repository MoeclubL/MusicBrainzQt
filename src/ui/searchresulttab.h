#ifndef SEARCHRESULTTAB_H
#define SEARCHRESULTTAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QSharedPointer>
#include "../core/types.h"
#include "../core/error_types.h"

QT_BEGIN_NAMESPACE
class Ui_SearchResultTab;
QT_END_NAMESPACE

class ResultTableModel;
class ResultItem;
class EntityDetailManager;
class EntityListWidget;

/**
 * @class SearchResultTab
 * @brief 搜索结果标签页组件 - 搜索结果的主要展示界面
 * 
 * SearchResultTab是展示MusicBrainz搜索结果的核心UI组件，
 * 提供了丰富的结果展示和交互功能。
 * 
 * **主要功能：**
 * - **结果展示**: 以表格形式展示搜索结果，支持多列排序
 * - **分页控制**: 内置分页导航，支持上一页/下一页操作
 * - **详情加载**: 自动加载并展示实体的详细信息
 * - **交互支持**: 双击打开详情，右键菜单操作
 * - **进度反馈**: 加载状态指示和错误处理
 * 
 * **UI组件结构：**
 * ```
 * SearchResultTab
 * ├── 查询信息区域（显示查询条件和结果统计）
 * ├── EntityListWidget（主要结果列表）
 * └── 分页控制区域（上一页/下一页按钮）
 * ```
 * 
 * **使用模式：**
 * ```cpp
 * SearchResultTab *tab = new SearchResultTab("Beatles", EntityType::Artist);
 * connect(tab, &SearchResultTab::itemDoubleClicked,
 *         this, &MainWindow::onItemDoubleClicked);
 * 
 * // 设置搜索结果
 * tab->setResults(results, stats);
 * ```
 * 
 * **数据流程：**
 * 1. 接收搜索结果和统计信息
 * 2. 更新UI展示和分页控件
 * 3. 自动触发详细信息加载
 * 4. 响应用户交互（双击、右键等）
 * 5. 发送相应的信号通知父组件
 * 
 * @author MusicBrainzQt Team
 * @see EntityListWidget, EntityDetailManager, ResultItem
 */
class SearchResultTab : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param query 搜索查询字符串
     * @param type 搜索的实体类型
     * @param parent 父窗口组件
     * 
     * 创建一个新的搜索结果标签页，初始化UI组件和数据管理器。
     */
    explicit SearchResultTab(const QString &query, EntityType type, QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 
     * 清理UI和数据资源。
     */
    ~SearchResultTab();

    // =============================================================================
    // 数据接口
    // =============================================================================
    
    /**
     * @brief 设置搜索结果数据
     * @param results 搜索结果项列表
     * @param stats 搜索统计信息
     * 
     * 更新标签页显示的搜索结果，同时更新分页控件状态。
     * 此方法会触发详细信息的自动加载过程。
     */
    void setResults(const QList<QSharedPointer<ResultItem>> &results, const SearchResults &stats);
    
    // =============================================================================
    // 状态查询接口
    // =============================================================================
    
    /**
     * @brief 获取搜索查询字符串
     * @return 当前标签页对应的搜索查询
     */
    QString getQuery() const { return m_query; }
    
    /**
     * @brief 获取搜索实体类型
     * @return 当前标签页对应的实体类型
     */
    EntityType getEntityType() const { return m_entityType; }
    
    /**
     * @brief 获取详细信息管理器
     * @return EntityDetailManager实例指针
     * 
     * 提供对内部详细信息管理器的访问，用于外部组件的协调。
     */
    EntityDetailManager* getDetailManager() const { return m_detailManager; }
    
    /**
     * @brief 清空所有结果
     * 
     * 清除当前显示的所有搜索结果，重置UI状态。
     */
    void clearResults();

signals:
    /**
     * @brief 项目双击信号
     * @param item 被双击的结果项
     * 
     * 当用户双击搜索结果中的某个项目时发出，
     * 通常用于打开该项目的详细信息标签页。
     */
    void itemDoubleClicked(const QSharedPointer<ResultItem> &item);
    
    /**
     * @brief 上一页请求信号
     * 
     * 当用户点击"上一页"按钮时发出，
     * 由父组件处理实际的分页逻辑。
     */
    void prevPageRequested();
    
    /**
     * @brief 下一页请求信号
     * 
     * 当用户点击"下一页"按钮时发出，
     * 由父组件处理实际的分页逻辑。
     */
    void nextPageRequested();
    
    /**
     * @brief 项目详情更新信号
     * @param item 详情已更新的结果项
     * 
     * 当某个结果项的详细信息加载完成并更新后发出，
     * 用于通知其他组件数据已更新。
     */
    void itemDetailsUpdated(const QSharedPointer<ResultItem> &item);
    
    // =============================================================================
    // 右键菜单相关信号
    // =============================================================================
    
    /**
     * @brief 在浏览器中打开信号
     * @param entityId 实体ID
     * @param entityType 实体类型
     * 
     * 当用户选择在浏览器中打开某个实体时发出。
     */
    void openInBrowser(const QString &entityId, EntityType entityType);
    
    /**
     * @brief 复制ID信号  
     * @param entityId 要复制的实体ID
     * 
     * 当用户选择复制实体ID到剪贴板时发出。
     */
    void copyId(const QString &entityId);

private slots:
    /**
     * @brief 处理实体详情加载完成
     * @param entityId 实体ID
     * @param details 详细信息数据
     */
    void onEntityDetailsLoaded(const QString &entityId, const QVariantMap &details);
    
    /**
     * @brief 处理详情加载失败
     * @param entityId 实体ID
     * @param error 错误信息
     */
    void onDetailLoadingFailed(const QString &entityId, const ErrorInfo &error);

private:
    /**
     * @brief 设置UI布局和组件
     * 
     * 初始化所有UI组件，建立布局结构和信号连接。
     */
    void setupUI();
    
    /**
     * @brief 更新分页控件状态
     * @param stats 搜索结果统计信息
     * 
     * 根据搜索结果统计信息更新分页按钮的可用状态和显示文本。
     */
    void updatePaginationControls(const SearchResults &stats);
    
    /**
     * @brief 获取MusicBrainz网站URL
     * @param item 结果项
     * @return 该实体在MusicBrainz网站上的URL
     * 
     * 构建指向MusicBrainz官方网站相应实体页面的URL。
     */
    QString getMusicBrainzUrl(const QSharedPointer<ResultItem> &item) const;
    
    // =============================================================================
    // 成员变量
    // =============================================================================
    
    // UI组件
    Ui_SearchResultTab *ui;                     ///< UI定义对象
    EntityListWidget *m_entityListWidget;      ///< 实体列表展示组件
    
    // 搜索数据
    QString m_query;                            ///< 搜索查询字符串
    EntityType m_entityType;                    ///< 搜索实体类型
    SearchResults m_currentStats;               ///< 当前搜索统计信息
    
    // 服务组件
    EntityDetailManager *m_detailManager;      ///< 详细信息管理器
};

#endif // SEARCHRESULTTAB_H
