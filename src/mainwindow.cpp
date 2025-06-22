#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/advancedsearchwidget.h"
#include "ui/searchresulttab.h"
#include "ui/itemdetailtab.h"
#include "ui/widget_helpers.h"
#include "services/searchservice.h"
#include "services/entitydetailmanager.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QMessageBox>
#include <QKeySequence>
#include <QIcon>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QFont>
#include <QListWidget>
#include <QMessageBox>

// =============================================================================
// 构造函数和析构函数
// MainWindow是整个应用程序的主要入口点，负责协调所有UI组件和服务
// =============================================================================

/**
 * @brief MainWindow构造函数
 * @param parent 父窗口组件
 * 
 * 初始化主窗口及其所有子组件，建立信号槽连接，设置初始状态。
 * 这是应用程序的中央协调器，管理搜索、结果显示和详情查看等功能。
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_searchService(new SearchService(this))       // 搜索服务，处理所有搜索相关操作
    , m_detailManager(new EntityDetailManager(this)) // 详情管理器，处理实体详细信息加载
{
    ui->setupUi(this);
    
    // 设置窗口最小尺寸，确保界面元素可见性
    setMinimumSize(800, 600);
    
    // 配置主窗口布局权重
    // 搜索区域(0)保持固定高度，结果区域(1)自动拉伸填充剩余空间
    ui->verticalLayout->setStretch(0, 0);  // 搜索区域不拉伸
    ui->verticalLayout->setStretch(1, 1);  // 结果区域拉伸
    ui->mainSplitter->setStretchFactor(0, 1);
    ui->mainSplitter->setStretchFactor(1, 1);

    // 按顺序初始化各个组件
    initializeUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
    
    setWindowTitle(tr("MusicBrainz Qt Client"));
}

/**
 * @brief MainWindow析构函数
 * 
 * 清理UI对象。其他成员对象通过Qt的父子关系自动清理。
 */
MainWindow::~MainWindow()
{
    delete ui;
}

// =============================================================================
// UI 初始化方法
// 这些方法负责设置和配置用户界面的各个部分
// 采用分层初始化模式，确保组件按正确顺序创建和配置
// =============================================================================

/**
 * @brief 初始化整个用户界面
 * 
 * 协调所有UI组件的初始化过程，按依赖关系顺序调用各个设置方法。
 * 这是UI初始化的主要入口点。
 */
void MainWindow::initializeUI()
{
    // 设置窗口基本属性
    resize(1400, 900);  // 设置合适的默认窗口大小
    setMinimumSize(1200, 700);  // 确保最小尺寸下界面仍然可用
    
    // 按依赖关系顺序初始化各部分UI
    setupSearchDock();      // 设置搜索面板
    setupMainTabWidget();   // 设置主标签页容器
    setupMenuBar();         // 设置菜单栏
    setupStatusBar();       // 设置状态栏
    setupConnections();     // 建立信号槽连接
}

/**
 * @brief 设置搜索停靠窗口
 * 
 * 创建并配置高级搜索功能的停靠窗口，提供搜索选项和过滤器。
 * 搜索面板作为应用程序的主要输入接口，支持多种搜索模式和过滤条件。
 * 
 * **窗口特性：**
 * - 可停靠到左侧或右侧
 * - 支持浮动窗口模式
 * - 固定宽度范围（300-400px）
 * - 自动连接搜索信号
 * 
 * **UI布局：**
 * - 左侧停靠（默认）
 * - 可移动和浮动
 * - 自适应内容高度
 */
void MainWindow::setupSearchDock()
{
    // 创建搜索面板作为左侧停靠窗口
    m_searchDock = new QDockWidget(tr("Search"), this);
    m_searchDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_searchDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    
    // 创建高级搜索组件
    m_searchWidget = new AdvancedSearchWidget();
    m_searchWidget->setMinimumWidth(300);
    m_searchWidget->setMaximumWidth(400);
    
    // 设置搜索组件到停靠窗口
    m_searchDock->setWidget(m_searchWidget);
    
    // 添加到主窗口左侧
    addDockWidget(Qt::LeftDockWidgetArea, m_searchDock);
    
    // 连接搜索信号
    connect(m_searchWidget, &AdvancedSearchWidget::searchRequested,
            this, &MainWindow::onAdvancedSearchRequested);
}

// =============================================================================
// 菜单栏和状态栏设置
// =============================================================================

/**
 * @brief 设置菜单栏
 * 
 * 配置应用程序的主菜单栏，包括文件菜单、编辑菜单、帮助菜单等。
 * 建立菜单项与相应功能的信号槽连接。
 * 
 * **菜单结构：**
 * - File菜单：退出等基本操作
 * - Edit菜单：偏好设置等配置选项
 * - Help菜单：关于对话框等帮助信息
 * 
 * **快捷键设置：**
 * - 退出：Ctrl+Q（或系统默认）
 * - 其他快捷键按Qt标准设置
 */
void MainWindow::setupMenuBar()
{
    // 连接退出动作到窗口关闭
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    
    // 连接关于动作到关于对话框
    connect(ui->actionAbout, &QAction::triggered, [this]() {
        QMessageBox::about(this, tr("About MusicBrainz Qt"),
                           tr("MusicBrainz Qt Client\n\n"
                              "A cross-platform MusicBrainz client allowing you to search, "
                              "browse and edit MusicBrainz data."));
    });
    
    // 连接偏好设置动作（功能待实现）
    connect(ui->actionPreferences, &QAction::triggered, [this]() {
        // TODO: 实现偏好设置对话框
        // 未来版本将包含：
        // - API服务器配置
        // - 界面主题设置
        // - 搜索选项配置
        // - 缓存和性能设置
        statusBar()->showMessage(tr("Preferences not implemented yet"), 2000);
    });
    
    // 设置标准快捷键
    ui->actionExit->setShortcut(QKeySequence::Quit);
}

/**
 * @brief 设置状态栏
 * 
 * 初始化应用程序底部的状态栏，用于显示操作状态、进度信息等。
 * 状态栏是用户获取应用程序状态反馈的主要途径。
 */
void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

/**
 * @brief 设置主标签页容器
 * 
 * 创建并配置应用程序的主要内容区域 - 标签页容器。
 * 支持多标签页展示搜索结果和详细信息。
 * 
 * **标签页特性：**
 * - 可关闭标签页（除欢迎页）
 * - 可拖拽重排标签页顺序
 * - 动态添加搜索结果和详情标签页
 * - 智能标签页标题生成
 * 
 * **初始状态：**
 * - 创建欢迎页作为默认标签页
 * - 提供使用指导信息
 * - 设置适当的字体和布局
 */
void MainWindow::setupMainTabWidget()
{
    // 创建主标签页容器
    m_mainTabWidget = new QTabWidget(this);
    m_mainTabWidget->setTabsClosable(true);     // 允许关闭标签页
    m_mainTabWidget->setMovable(true);          // 允许拖拽移动标签页
    
    // 设置为中央部件
    setCentralWidget(m_mainTabWidget);
    
    // =============================================================================
    // 连接标签页信号
    // =============================================================================
    
    // 标签页关闭请求
    connect(m_mainTabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onTabCloseRequested);
    
    // 标签页切换
    connect(m_mainTabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onTabChanged);
}

// =============================================================================
// 信号连接设置
// =============================================================================

void MainWindow::setupConnections()
{
    // SearchService 连接
    connect(m_searchService, &SearchService::searchCompleted, 
            this, &MainWindow::onSearchCompleted);
    connect(m_searchService, &SearchService::searchFailed,
            this, &MainWindow::onSearchFailed);
    
    // EntityDetailManager 连接
    connect(m_detailManager, &EntityDetailManager::entityDetailsLoaded,
            this, &MainWindow::onEntityDetailsLoaded);
}

// =============================================================================
// 搜索相关槽函数
// =============================================================================

void MainWindow::onAdvancedSearchRequested(const SearchParameters &params)
{
    if (!params.isValid()) {
        statusBar()->showMessage(tr("Please enter a search query"));
        return;
    }
    
    m_currentSearchParams = params;
    statusBar()->showMessage(tr("Searching for: %1...").arg(params.query));
    m_searchService->search(params);
}

void MainWindow::onSearchCompleted(const QList<QSharedPointer<ResultItem>> &results, const SearchResults &stats)
{
    // 创建或更新搜索结果标签页
    SearchResultTab *resultTab = nullptr;
    
    // 检查是否已有相同搜索的标签页
    QString tabTitle = generateTabTitle(m_currentSearchParams.query, m_currentSearchParams.type);
    for (int i = 0; i < m_mainTabWidget->count(); ++i) {
        if (m_mainTabWidget->tabText(i) == tabTitle) {
            resultTab = qobject_cast<SearchResultTab*>(m_mainTabWidget->widget(i));
            break;
        }
    }
    
    // 如果没有找到，创建新的标签页
    if (!resultTab) {
        resultTab = createSearchResultTab(m_currentSearchParams.query, m_currentSearchParams.type);
        int tabIndex = m_mainTabWidget->addTab(resultTab, tabTitle);
        m_mainTabWidget->setCurrentIndex(tabIndex);
    }
    
    // 设置搜索结果
    resultTab->setResults(results, stats);
    
    // 更新状态栏
    if (results.isEmpty()) {
        statusBar()->showMessage(tr("No results found"));
    } else {
        statusBar()->showMessage(tr("Found %1 results").arg(stats.totalCount));
    }
}

void MainWindow::onSearchFailed(const QString &error)
{
    statusBar()->showMessage(tr("Search failed: %1").arg(error));
    QMessageBox::warning(this, tr("Search Error"), error);
}

// =============================================================================
// 标签页管理方法
// =============================================================================

SearchResultTab* MainWindow::createSearchResultTab(const QString &query, EntityType type)
{
    SearchResultTab *tab = new SearchResultTab(query, type);
    
    // 连接信号
    connect(tab, &SearchResultTab::itemDoubleClicked,
            this, &MainWindow::onItemDoubleClicked);
    connect(tab, &SearchResultTab::prevPageRequested,
            this, &MainWindow::onPrevPageRequested);
    connect(tab, &SearchResultTab::nextPageRequested,
            this, &MainWindow::onNextPageRequested);
    connect(tab, &SearchResultTab::itemDetailsUpdated,
            this, &MainWindow::onItemDetailsUpdated);
    
    // 连接右键菜单信号
    connect(tab, &SearchResultTab::openInBrowser,
            this, &MainWindow::onOpenInBrowser);
    connect(tab, &SearchResultTab::copyId,
            this, &MainWindow::onCopyId);
    
    return tab;
}

ItemDetailTab* MainWindow::createItemDetailTab(const QSharedPointer<ResultItem> &item)
{
    ItemDetailTab *tab = new ItemDetailTab(item);
    
    // 连接子项打开详情信号
    connect(tab, &ItemDetailTab::openDetailTab,
            this, &MainWindow::onItemDoubleClicked);
    
    // 连接右键菜单信号
    connect(tab, &ItemDetailTab::openInBrowser,
            this, &MainWindow::onOpenInBrowser);
    connect(tab, &ItemDetailTab::copyId,
            this, &MainWindow::onCopyId);
    
    return tab;
}

void MainWindow::onItemDoubleClicked(const QSharedPointer<ResultItem> &item)
{
    if (!item) {
        return;
    }
    
    // 检查是否已有相同项目的详细信息标签页
    QString tabTitle = generateDetailTabTitle(item);
    for (int i = 0; i < m_mainTabWidget->count(); ++i) {
        if (m_mainTabWidget->tabText(i) == tabTitle) {
            m_mainTabWidget->setCurrentIndex(i);
            return;
        }
    }
    
    // 创建新的详细信息标签页
    ItemDetailTab *detailTab = createItemDetailTab(item);
    int tabIndex = m_mainTabWidget->addTab(detailTab, tabTitle);
    m_mainTabWidget->setCurrentIndex(tabIndex);
    
    // 将ItemDetailTab添加到映射中
    m_itemDetailTabs[item->getId()] = detailTab;
    
    // 使用MainWindow的EntityDetailManager加载详细信息
    QList<QSharedPointer<ResultItem>> singleItem = {item};
    m_detailManager->loadEntitiesDetails(singleItem);
    
    statusBar()->showMessage(tr("Opened details for: %1").arg(item->getName()));
}

void MainWindow::onTabCloseRequested(int index)
{
    // 允许关闭任何标签页
    if (index >= 0 && index < m_mainTabWidget->count()) {
        QWidget *widget = m_mainTabWidget->widget(index);
        
        // 如果是ItemDetailTab，从映射中移除
        ItemDetailTab *detailTab = qobject_cast<ItemDetailTab*>(widget);
        if (detailTab) {
            QString itemId = detailTab->getItemId();
            m_itemDetailTabs.remove(itemId);
        }
        
        m_mainTabWidget->removeTab(index);
        widget->deleteLater();
    }
}

void MainWindow::onTabChanged(int index)
{
    if (index >= 0) {
        QString tabText = m_mainTabWidget->tabText(index);
        statusBar()->showMessage(tr("Current tab: %1").arg(tabText), 2000);
    }
}

void MainWindow::onPrevPageRequested()
{
    SearchResultTab *currentTab = qobject_cast<SearchResultTab*>(m_mainTabWidget->currentWidget());
    if (currentTab && m_searchService->canGoPrevPage()) {
        statusBar()->showMessage(tr("Loading previous page..."));
        m_searchService->searchPrevPage();
    }
}

void MainWindow::onNextPageRequested()
{
    SearchResultTab *currentTab = qobject_cast<SearchResultTab*>(m_mainTabWidget->currentWidget());
    if (currentTab && m_searchService->canGoNextPage()) {
        statusBar()->showMessage(tr("Loading next page..."));
        m_searchService->searchNextPage();
    }
}

QString MainWindow::generateTabTitle(const QString &query, EntityType type)
{
    QString typeStr;
    switch (type) {
        case EntityType::Artist: typeStr = tr("Artists"); break;
        case EntityType::Release: typeStr = tr("Releases"); break;
        case EntityType::ReleaseGroup: typeStr = tr("Release Groups"); break;
        case EntityType::Recording: typeStr = tr("Recordings"); break;
        case EntityType::Label: typeStr = tr("Labels"); break;
        case EntityType::Work: typeStr = tr("Works"); break;
        case EntityType::Area: typeStr = tr("Areas"); break;
        case EntityType::Place: typeStr = tr("Places"); break;
        default: typeStr = tr("Results"); break;
    }
    
    QString shortQuery = query.length() > 20 ? query.left(17) + "..." : query;
    return QString("%1: %2").arg(typeStr).arg(shortQuery);
}

QString MainWindow::generateDetailTabTitle(const QSharedPointer<ResultItem> &item)
{
    QString name = item->getName();
    if (name.length() > 25) {
        name = name.left(22) + "...";
    }
    return QString("%1 (%2)").arg(name).arg(item->getTypeString());
}

void MainWindow::onItemDetailsUpdated(const QSharedPointer<ResultItem> &item)
{
    if (!item) {
        return;
    }
    
    // 查找对应的ItemDetailTab并刷新它
    auto it = m_itemDetailTabs.find(item->getId());
    if (it != m_itemDetailTabs.end()) {
        ItemDetailTab *detailTab = it.value();
        if (detailTab) {
            detailTab->refreshItemInfo();
        }
    }
}

// =============================================================================
// 右键菜单相关槽函数
// =============================================================================

void MainWindow::onOpenInBrowser(const QString &entityId, EntityType entityType)
{
    QString url = WidgetHelpers::buildMusicBrainzUrl(entityId, entityType);
    
    if (url.isEmpty()) {
        statusBar()->showMessage(tr("Unsupported entity type for browser opening"), 3000);
        return;
    }
    
    // 在默认浏览器中打开URL
    if (WidgetHelpers::openUrlInBrowser(url)) {
        statusBar()->showMessage(tr("Opened in browser: %1").arg(url), 3000);
    } else {
        statusBar()->showMessage(tr("Failed to open URL in browser"), 3000);
    }
}

void MainWindow::onCopyId(const QString &entityId)
{
    WidgetHelpers::copyToClipboard(entityId);
    statusBar()->showMessage(tr("Copied ID to clipboard: %1").arg(entityId), 2000);
}

void MainWindow::onEntityDetailsLoaded(const QString &entityId, const QVariantMap &details)
{
    // 查找对应的ItemDetailTab
    auto it = m_itemDetailTabs.find(entityId);
    if (it != m_itemDetailTabs.end()) {
        ItemDetailTab *detailTab = it.value();
        
        // 更新ResultItem的详细数据
        QSharedPointer<ResultItem> item = detailTab->getItem();
        if (item) {
            // 设置详细数据到ResultItem
            for (auto detailIt = details.begin(); detailIt != details.end(); ++detailIt) {
                item->setDetailProperty(detailIt.key(), detailIt.value());
            }
            
            // 通知ItemDetailTab刷新显示
            detailTab->refreshItemInfo();
            
            statusBar()->showMessage(tr("Details loaded for: %1").arg(item->getName()), 2000);
        }
    }
}