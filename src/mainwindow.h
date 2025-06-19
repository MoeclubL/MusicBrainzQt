#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QSharedPointer>
#include <QSplitter>
#include <QDockWidget>
#include "core/types.h"
#include "models/resultitem.h"
#include "ui/advancedsearchwidget.h"
#include "services/searchservice.h"
#include "services/entitydetailmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SearchResultTab;
class ItemDetailTab;

/**
 * @brief 主窗口类 - MusicBrainzQt应用程序的主要用户界面
 * 
 * MainWindow是整个应用程序的中心控制器，负责：
 * - 协调搜索操作和结果显示
 * - 管理标签页和窗口布局
 * - 处理用户交互和应用程序状态
 * - 连接各个服务层组件
 * 
 * 架构模式：
 * - 使用Qt的信号槽机制进行组件间通信
 * - 采用服务层模式分离业务逻辑
 * - 支持多标签页的工作流程
 * 
 * @see SearchService 处理搜索相关逻辑
 * @see EntityDetailManager 管理实体详细信息
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口组件
     */
    MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~MainWindow();

private:
    // =============================================================================
    // 核心组件
    // =============================================================================
    
    Ui::MainWindow *ui;                     ///< UI对象，由Qt Designer生成
    SearchService *m_searchService;         ///< 搜索服务，处理MusicBrainz API调用
    EntityDetailManager *m_detailManager;   ///< 详情管理器，处理实体详细信息
    
    // =============================================================================
    // UI 组件
    // =============================================================================
    
    QDockWidget *m_searchDock;              ///< 搜索面板停靠窗口
    AdvancedSearchWidget *m_searchWidget;   ///< 高级搜索界面组件
    QTabWidget *m_mainTabWidget;            ///< 主标签页容器
    
    // =============================================================================
    // 应用程序状态
    // =============================================================================
    
    SearchParameters m_currentSearchParams; ///< 当前搜索参数
    
    // 详细标签页映射 - 用于跟踪ItemDetailTab和对应的ResultItem
    // Key: entityId, Value: ItemDetailTab实例指针
    QMap<QString, ItemDetailTab*> m_itemDetailTabs;

    // =============================================================================
    // UI 初始化方法
    // 这些私有方法负责设置界面的不同部分
    // =============================================================================
    
    /**
     * @brief 初始化整个用户界面
     */
    void initializeUI();
    
    /**
     * @brief 设置搜索停靠窗口
     */
    void setupSearchDock();
    
    /**
     * @brief 设置主标签页容器
     */
    void setupMainTabWidget();
    
    /**
     * @brief 设置菜单栏
     */
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    
    // 标签页管理
    SearchResultTab* createSearchResultTab(const QString &query, EntityType type);
    ItemDetailTab* createItemDetailTab(const QSharedPointer<ResultItem> &item);
    void closeTab(int index);
    QString generateTabTitle(const QString &query, EntityType type);
    QString generateDetailTabTitle(const QSharedPointer<ResultItem> &item);
    
private slots:
    void onAdvancedSearchRequested(const SearchParameters &params);
    void onSearchCompleted(const QList<QSharedPointer<ResultItem>> &results, const SearchResults &stats);
    void onSearchFailed(const QString &error);
    void onItemDoubleClicked(const QSharedPointer<ResultItem> &item);
    void onTabCloseRequested(int index);
    void onTabChanged(int index);
    void onPrevPageRequested();
    void onNextPageRequested();
    void onItemDetailsUpdated(const QSharedPointer<ResultItem> &item);
    
    // 右键菜单相关槽函数
    void onOpenInBrowser(const QString &entityId, EntityType entityType);
    void onCopyId(const QString &entityId);
    
    // 详细信息加载相关槽函数
    void onEntityDetailsLoaded(const QString &entityId, const QVariantMap &details);
};

#endif // MAINWINDOW_H