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
    // 创建EntityListWidget并插入到标题和分页控件之间
    m_entityListWidget = new EntityListWidget(this);
    
    // 将EntityListWidget插入到标题后面，分页控件前面
    ui->leftLayout->insertWidget(1, m_entityListWidget);
    
    // 设置标题
    ui->resultsLabel->setText(tr("Search Results for: %1").arg(m_query));
    
    // 配置EntityListWidget
    m_entityListWidget->setPaginationVisible(false); // 使用UI文件中的分页控件
    m_entityListWidget->setToolBarVisible(false); // 搜索结果不需要工具栏
      // 连接信号
    connect(m_entityListWidget, &EntityListWidget::itemDoubleClicked,
            this, &SearchResultTab::itemDoubleClicked);
    connect(m_entityListWidget, &EntityListWidget::itemSelectionChanged,
            this, &SearchResultTab::onItemSelectionChanged);
    connect(ui->prevPageButton, &QPushButton::clicked,
            this, &SearchResultTab::prevPageRequested);
    connect(ui->nextPageButton, &QPushButton::clicked,
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
        ui->prevPageButton->setEnabled(false);
        ui->nextPageButton->setEnabled(false);
        ui->pageInfoLabel->setText(tr("No results"));
        return;
    }
    
    int currentPage = stats.offset / qMax(1, stats.count) + 1;
    int totalPages = (stats.totalCount + stats.count - 1) / qMax(1, stats.count);
    
    bool prevEnabled = stats.offset > 0;
    bool nextEnabled = stats.offset + stats.count < stats.totalCount;
    
    ui->prevPageButton->setEnabled(prevEnabled);
    ui->nextPageButton->setEnabled(nextEnabled);
    ui->pageInfoLabel->setText(tr("Showing %1-%2 of %3 results (Page %4 of %5)")
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

void SearchResultTab::onItemSelectionChanged()
{
    QSharedPointer<ResultItem> selectedItem = m_entityListWidget->getCurrentItem();
    
    // 清空右侧面板
    clearRightPanel();
    
    if (selectedItem) {
        // 创建迷你详情显示组件
        createItemPreview(selectedItem);
        LOG_UI_INFO("SearchResultTab: Selected item %s for preview", 
                    selectedItem->getName().toUtf8().constData());
    }
}

void SearchResultTab::clearRightPanel()
{
    // 清空右侧布局中的所有组件
    while (QLayoutItem *item = ui->rightLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void SearchResultTab::createItemPreview(const QSharedPointer<ResultItem> &item)
{
    if (!item) return;
    
    // 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // 创建预览内容组件
    QWidget *previewWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(previewWidget);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    
    // 标题
    QLabel *titleLabel = new QLabel(tr("Item Preview"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333; margin-bottom: 10px;");
    layout->addWidget(titleLabel);
    
    // 实体名称
    QLabel *nameLabel = new QLabel(QString("<h3>%1</h3>").arg(item->getName()));
    nameLabel->setWordWrap(true);
    layout->addWidget(nameLabel);
    
    // 实体类型
    QLabel *typeLabel = new QLabel(QString("<b>Type:</b> %1").arg(item->getTypeString()));
    typeLabel->setStyleSheet("color: #666; margin-bottom: 5px;");
    layout->addWidget(typeLabel);
    
    // MBID
    QLabel *idLabel = new QLabel(QString("<b>MBID:</b> %1").arg(item->getId()));
    idLabel->setStyleSheet("color: #666; margin-bottom: 5px; font-family: monospace; font-size: 10px;");
    idLabel->setWordWrap(true);
    layout->addWidget(idLabel);
    
    // 评分信息
    if (item->getScore() > 0) {
        QLabel *scoreLabel = new QLabel(QString("<b>Match Score:</b> %1%").arg(item->getScore()));
        scoreLabel->setStyleSheet("color: #666; margin-bottom: 5px;");
        layout->addWidget(scoreLabel);
    }
    
    // 消歧义信息
    QString disambiguation = item->getDisambiguation();
    if (!disambiguation.isEmpty()) {
        QLabel *disambigLabel = new QLabel(QString("<b>Disambiguation:</b> %1").arg(disambiguation));
        disambigLabel->setStyleSheet("color: #666; margin-bottom: 5px;");
        disambigLabel->setWordWrap(true);
        layout->addWidget(disambigLabel);
    }
    
    // 分隔线
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setStyleSheet("color: #ccc;");
    layout->addWidget(separator);
    
    // 详细信息预览（从已有数据中显示）
    QVariantMap detailData = item->getDetailData();
    if (!detailData.isEmpty()) {
        addDetailPreview(layout, detailData, item->getType());
    }
    
    // 操作按钮
    QWidget *buttonWidget = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    
    QPushButton *detailsButton = new QPushButton(tr("View Details"));
    detailsButton->setStyleSheet("QPushButton { background-color: #007bff; color: white; padding: 5px 10px; border: none; border-radius: 3px; } QPushButton:hover { background-color: #0056b3; }");
    connect(detailsButton, &QPushButton::clicked, [this, item]() {
        emit itemDoubleClicked(item);
    });
    
    QPushButton *browserButton = new QPushButton(tr("Open in Browser"));
    browserButton->setStyleSheet("QPushButton { background-color: #28a745; color: white; padding: 5px 10px; border: none; border-radius: 3px; } QPushButton:hover { background-color: #1e7e34; }");
    connect(browserButton, &QPushButton::clicked, [this, item]() {
        emit openInBrowser(item->getId(), item->getType());
    });
    
    buttonLayout->addWidget(detailsButton);
    buttonLayout->addWidget(browserButton);
    buttonLayout->addStretch();
    
    layout->addWidget(buttonWidget);
    layout->addStretch();
    
    // 设置预览组件到滚动区域
    scrollArea->setWidget(previewWidget);
    
    // 添加到右侧布局
    ui->rightLayout->addWidget(scrollArea);
}

void SearchResultTab::addDetailPreview(QVBoxLayout *layout, const QVariantMap &detailData, EntityType type)
{
    QLabel *detailTitle = new QLabel(tr("Additional Information"));
    detailTitle->setStyleSheet("font-weight: bold; color: #333; margin-top: 10px; margin-bottom: 5px;");
    layout->addWidget(detailTitle);
    
    // 根据实体类型显示不同的详细信息
    switch (type) {
    case EntityType::Artist:
        addArtistDetailPreview(layout, detailData);
        break;
    case EntityType::Release:
        addReleaseDetailPreview(layout, detailData);
        break;
    case EntityType::Recording:
        addRecordingDetailPreview(layout, detailData);
        break;
    case EntityType::ReleaseGroup:
        addReleaseGroupDetailPreview(layout, detailData);
        break;
    case EntityType::Work:
        addWorkDetailPreview(layout, detailData);
        break;
    case EntityType::Label:
        addLabelDetailPreview(layout, detailData);
        break;
    default:
        break;
    }
}

void SearchResultTab::addArtistDetailPreview(QVBoxLayout *layout, const QVariantMap &detailData)
{
    if (detailData.contains("type")) {
        QString artistType = detailData["type"].toString();
        if (!artistType.isEmpty()) {
            QLabel *typeLabel = new QLabel(QString("<b>Artist Type:</b> %1").arg(artistType));
            typeLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(typeLabel);
        }
    }
    
    if (detailData.contains("area")) {
        QVariantMap area = detailData["area"].toMap();
        QString areaName = area["name"].toString();
        if (!areaName.isEmpty()) {
            QLabel *areaLabel = new QLabel(QString("<b>Area:</b> %1").arg(areaName));
            areaLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(areaLabel);
        }
    }
    
    if (detailData.contains("begin-area")) {
        QVariantMap beginArea = detailData["begin-area"].toMap();
        QString beginAreaName = beginArea["name"].toString();
        if (!beginAreaName.isEmpty()) {
            QLabel *beginAreaLabel = new QLabel(QString("<b>Begin Area:</b> %1").arg(beginAreaName));
            beginAreaLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(beginAreaLabel);
        }
    }
}

void SearchResultTab::addReleaseDetailPreview(QVBoxLayout *layout, const QVariantMap &detailData)
{
    if (detailData.contains("date")) {
        QString date = detailData["date"].toString();
        if (!date.isEmpty()) {
            QLabel *dateLabel = new QLabel(QString("<b>Release Date:</b> %1").arg(date));
            dateLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(dateLabel);
        }
    }
    
    if (detailData.contains("country")) {
        QString country = detailData["country"].toString();
        if (!country.isEmpty()) {
            QLabel *countryLabel = new QLabel(QString("<b>Country:</b> %1").arg(country));
            countryLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(countryLabel);
        }
    }
    
    if (detailData.contains("status")) {
        QString status = detailData["status"].toString();
        if (!status.isEmpty()) {
            QLabel *statusLabel = new QLabel(QString("<b>Status:</b> %1").arg(status));
            statusLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(statusLabel);
        }
    }
}

void SearchResultTab::addRecordingDetailPreview(QVBoxLayout *layout, const QVariantMap &detailData)
{
    if (detailData.contains("length")) {
        int length = detailData["length"].toInt();
        if (length > 0) {
            int minutes = length / (1000 * 60);
            int seconds = (length / 1000) % 60;
            QString lengthStr = QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
            QLabel *lengthLabel = new QLabel(QString("<b>Length:</b> %1").arg(lengthStr));
            lengthLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(lengthLabel);
        }
    }
}

void SearchResultTab::addReleaseGroupDetailPreview(QVBoxLayout *layout, const QVariantMap &detailData)
{
    if (detailData.contains("primary-type")) {
        QString primaryType = detailData["primary-type"].toString();
        if (!primaryType.isEmpty()) {
            QLabel *typeLabel = new QLabel(QString("<b>Primary Type:</b> %1").arg(primaryType));
            typeLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(typeLabel);
        }
    }
    
    if (detailData.contains("first-release-date")) {
        QString firstReleaseDate = detailData["first-release-date"].toString();
        if (!firstReleaseDate.isEmpty()) {
            QLabel *dateLabel = new QLabel(QString("<b>First Release:</b> %1").arg(firstReleaseDate));
            dateLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(dateLabel);
        }
    }
}

void SearchResultTab::addWorkDetailPreview(QVBoxLayout *layout, const QVariantMap &detailData)
{
    if (detailData.contains("type")) {
        QString workType = detailData["type"].toString();
        if (!workType.isEmpty()) {
            QLabel *typeLabel = new QLabel(QString("<b>Work Type:</b> %1").arg(workType));
            typeLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(typeLabel);
        }
    }
}

void SearchResultTab::addLabelDetailPreview(QVBoxLayout *layout, const QVariantMap &detailData)
{
    if (detailData.contains("type")) {
        QString labelType = detailData["type"].toString();
        if (!labelType.isEmpty()) {
            QLabel *typeLabel = new QLabel(QString("<b>Label Type:</b> %1").arg(labelType));
            typeLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(typeLabel);
        }
    }
    
    if (detailData.contains("label-code")) {
        int labelCode = detailData["label-code"].toInt();
        if (labelCode > 0) {
            QLabel *codeLabel = new QLabel(QString("<b>Label Code:</b> LC-%1").arg(labelCode, 5, 10, QChar('0')));
            codeLabel->setStyleSheet("color: #666; font-size: 11px;");
            layout->addWidget(codeLabel);
        }
    }
}
