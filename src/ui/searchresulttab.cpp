#include "searchresulttab.h"
#include "ui_searchresulttab.h"
#include "entitylistwidget.h"
#include "../models/resultitem.h"
#include "../services/entitydetailmanager.h"
#include "../utils/logger.h"

SearchResultTab::SearchResultTab(const QString &query, EntityType type, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui_SearchResultTab)
    , m_query(query)
    , m_entityType(type)
    , m_detailManager(new EntityDetailManager(this))
{
    ui->setupUi(this);
    setupUI();
    
    // 连接详细信息管理器信号
    connect(m_detailManager, &EntityDetailManager::entityDetailsLoaded,
            this, &SearchResultTab::onEntityDetailsLoaded);
    connect(m_detailManager, &EntityDetailManager::detailsLoadingFailed,
            this, &SearchResultTab::onDetailLoadingFailed);
            
    LOG_UI_INFO("SearchResultTab created for query: %s, type: %d", 
                query.toUtf8().constData(), static_cast<int>(type));
}

SearchResultTab::~SearchResultTab()
{
    delete ui;
}

void SearchResultTab::setupUI()
{
    // 创建EntityListWidget
    m_entityListWidget = new EntityListWidget(this);
    ui->mainLayout->addWidget(m_entityListWidget);
    
    // 设置标题和分页
    m_entityListWidget->setTitle(tr("Search Results for: %1").arg(m_query));
    m_entityListWidget->setPaginationVisible(true);
    m_entityListWidget->setToolBarVisible(false); // 搜索结果不需要工具栏
      // 连接信号
    connect(m_entityListWidget, &EntityListWidget::itemDoubleClicked,
            this, &SearchResultTab::itemDoubleClicked);
    connect(m_entityListWidget, &EntityListWidget::prevPageRequested,
            this, &SearchResultTab::prevPageRequested);
    connect(m_entityListWidget, &EntityListWidget::nextPageRequested,
            this, &SearchResultTab::nextPageRequested);
    
    // 连接右键菜单信号
    connect(m_entityListWidget, &EntityListWidget::openInBrowser,
            this, &SearchResultTab::openInBrowser);
    connect(m_entityListWidget, &EntityListWidget::copyId,
            this, &SearchResultTab::copyId);
}

void SearchResultTab::setResults(const QList<QSharedPointer<ResultItem>> &results, const SearchResults &stats)
{
    m_currentStats = stats;
    
    // 设置数据到EntityListWidget
    m_entityListWidget->setItems(results);
    
    // 更新分页控件
    updatePaginationControls(stats);
    
    // 不再自动批量加载详细信息，只有在用户双击项目时才加载
}

void SearchResultTab::clearResults()
{
    m_entityListWidget->clear();
    m_currentStats = SearchResults();
    updatePaginationControls(m_currentStats);
}

void SearchResultTab::updatePaginationControls(const SearchResults &stats)
{
    if (stats.totalCount == 0) {
        m_entityListWidget->setPaginationEnabled(false, false);
        m_entityListWidget->setPaginationInfo(tr("No results"));
        return;
    }
    
    int currentPage = stats.offset / qMax(1, stats.count) + 1;
    int totalPages = (stats.totalCount + stats.count - 1) / qMax(1, stats.count);
    
    bool prevEnabled = stats.offset > 0;
    bool nextEnabled = stats.offset + stats.count < stats.totalCount;
    
    m_entityListWidget->setPaginationEnabled(prevEnabled, nextEnabled);
    m_entityListWidget->setPaginationInfo(tr("Showing %1-%2 of %3 results (Page %4 of %5)")
                                        .arg(stats.offset + 1)
                                        .arg(stats.offset + stats.count)
                                        .arg(stats.totalCount)
                                        .arg(currentPage)
                                        .arg(totalPages));
}

void SearchResultTab::onEntityDetailsLoaded(const QString &entityId, const QVariantMap &details)
{
    Q_UNUSED(entityId)
    Q_UNUSED(details)
    
    // 通知详细信息已更新
    // 这个信号会被MainWindow接收，用于刷新相应的ItemDetailTab
    emit itemDetailsUpdated(nullptr); // 需要根据实际情况修改
}

void SearchResultTab::onDetailLoadingFailed(const QString &entityId, const ErrorInfo &error)
{
    qCDebug(logUI) << "SearchResultTab: Failed to load details for entity" << entityId 
                   << "Error:" << error.message;
}
