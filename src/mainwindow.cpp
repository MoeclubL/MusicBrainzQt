#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/advancedsearchwidget.h"
#include "ui/searchresulttab.h"
#include "ui/itemdetailtab.h"
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
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include <QGuiApplication>

// =============================================================================
// 构造函数和析构函数
// =============================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_searchService(new SearchService(this))
    , m_detailManager(new EntityDetailManager(this))
{
    ui->setupUi(this);
    
    // 设置窗口最小尺寸
    setMinimumSize(800, 600);
    
    // 设置主窗口布局
    ui->verticalLayout->setStretch(0, 0);  // 搜索区域不拉伸
    ui->verticalLayout->setStretch(1, 1);  // 结果区域拉伸
    ui->mainSplitter->setStretchFactor(0, 1);
    ui->mainSplitter->setStretchFactor(1, 1);

    // 初始化各个组件
    initializeUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
    
    setWindowTitle(tr("MusicBrainz Qt Client"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// =============================================================================
// UI 初始化方法
// =============================================================================

void MainWindow::initializeUI()
{
    // 设置窗口基本属性
    resize(1400, 900);
    setMinimumSize(1200, 700);
    
    // 初始化各部分 UI
    setupSearchDock();
    setupMainTabWidget();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
}

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

void MainWindow::setupMenuBar()
{
    // 连接已存在的菜单动作
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, [this]() {
        QMessageBox::about(this, tr("About MusicBrainz Qt"),
                           tr("MusicBrainz Qt Client\n\n"
                              "A cross-platform MusicBrainz client allowing you to search, "
                              "browse and edit MusicBrainz data."));
    });
    
    // 连接现有的Preferences动作（如果需要的话）
    connect(ui->actionPreferences, &QAction::triggered, [this]() {
        // TODO: 实现偏好设置对话框
        statusBar()->showMessage(tr("Preferences not implemented yet"), 2000);
    });
    
    // 设置快捷键
    ui->actionExit->setShortcut(QKeySequence::Quit);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupMainTabWidget()
{
    // 创建主标签页容器
    m_mainTabWidget = new QTabWidget(this);
    m_mainTabWidget->setTabsClosable(true);
    m_mainTabWidget->setMovable(true);
    
    // 设置为中央部件
    setCentralWidget(m_mainTabWidget);
    
    // 创建欢迎页面
    QWidget *welcomePage = new QWidget();
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomePage);
    
    QLabel *welcomeLabel = new QLabel(tr("Welcome to MusicBrainz Qt"));
    welcomeLabel->setAlignment(Qt::AlignCenter);
    QFont welcomeFont = welcomeLabel->font();
    welcomeFont.setPointSize(welcomeFont.pointSize() + 4);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    
    QLabel *instructionLabel = new QLabel(tr("Use the search panel to find music information.\n"
                                             "Search results will appear in new tabs."));
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setWordWrap(true);
    
    welcomeLayout->addStretch();
    welcomeLayout->addWidget(welcomeLabel);
    welcomeLayout->addWidget(instructionLabel);
    welcomeLayout->addStretch();
    
    // 添加欢迎页面作为第一个标签页
    m_mainTabWidget->addTab(welcomePage, tr("Welcome"));
    
    // 连接标签页信号
    connect(m_mainTabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onTabCloseRequested);
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
    // 不允许关闭欢迎页面（第0个标签页）
    if (index > 0) {
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
    // 构建MusicBrainz URL
    QString baseUrl = "https://musicbrainz.org";
    QString typeString;
    
    switch (entityType) {
        case EntityType::Artist:
            typeString = "artist";
            break;
        case EntityType::Release:
            typeString = "release";
            break;
        case EntityType::ReleaseGroup:
            typeString = "release-group";
            break;
        case EntityType::Recording:
            typeString = "recording";
            break;
        case EntityType::Work:
            typeString = "work";
            break;
        case EntityType::Label:
            typeString = "label";
            break;
        case EntityType::Area:
            typeString = "area";
            break;
        case EntityType::Place:
            typeString = "place";
            break;
        default:
            statusBar()->showMessage(tr("Unsupported entity type for browser opening"), 3000);
            return;
    }
    
    QString url = QString("%1/%2/%3").arg(baseUrl).arg(typeString).arg(entityId);
    
    // 在默认浏览器中打开URL
    if (QDesktopServices::openUrl(QUrl(url))) {
        statusBar()->showMessage(tr("Opened in browser: %1").arg(url), 3000);
    } else {
        statusBar()->showMessage(tr("Failed to open URL in browser"), 3000);
    }
}

void MainWindow::onCopyId(const QString &entityId)
{
    // 复制ID到剪贴板
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(entityId);
    
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