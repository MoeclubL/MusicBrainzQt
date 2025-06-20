#include "itemdetailtab.h"
#include "ui_itemdetailtab.h"
#include "entitylistwidget.h"
#include "ui_utils.h"
#include "../models/resultitem.h"
#include "../utils/logger.h"
#include <QDesktopServices>
#include <QUrl>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <algorithm>

ItemDetailTab::ItemDetailTab(const QSharedPointer<ResultItem> &item, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ItemDetailTab)
    , m_item(item)
{
    ui->setupUi(this);
    setupUI();
    setupSubTabs();
    populateItemInfo();
    populateSubTabs();
}

ItemDetailTab::~ItemDetailTab()
{
    delete ui;
}

QString ItemDetailTab::getItemId() const
{
    return m_item ? m_item->getId() : QString();
}

QString ItemDetailTab::getItemName() const
{
    return m_item ? m_item->getName() : QString();
}

void ItemDetailTab::setupUI()
{
    // 连接信号
    connect(ui->subTabWidget, &QTabWidget::currentChanged,
            this, &ItemDetailTab::onSubTabChanged);
}

void ItemDetailTab::setupSubTabs()
{
    // 根据实体类型创建相应的子标签页
    EntityType itemType = m_item->getType();
    
    switch (itemType) {        case EntityType::Artist:
            createOverviewTab();
            createSubTab(tr("Releases"), "releases", EntityType::Release);
            createSubTab(tr("Release Groups"), "release-groups", EntityType::ReleaseGroup);
            createSubTab(tr("Recordings"), "recordings", EntityType::Recording);
            createSubTab(tr("Works"), "works", EntityType::Work);
            createAliasesTab();
            createTagsTab();
            createRelationshipsTab();
            break;
              case EntityType::Release:
            createOverviewTab();
            createSubTab(tr("Recordings"), "recordings", EntityType::Recording);
            createSubTab(tr("Artists"), "artist-credits", EntityType::Artist);
            createAliasesTab();
            createTagsTab();
            createRelationshipsTab();
            break;
              case EntityType::ReleaseGroup:
            createOverviewTab();
            createSubTab(tr("Releases"), "releases", EntityType::Release);
            createAliasesTab();
            createTagsTab();
            createRelationshipsTab();
            break;
              case EntityType::Recording:
            createOverviewTab();
            createSubTab(tr("Releases"), "releases", EntityType::Release);
            createSubTab(tr("Artists"), "artist-credits", EntityType::Artist);
            createSubTab(tr("Works"), "works", EntityType::Work);
            createAliasesTab();
            createTagsTab();
            createRelationshipsTab();
            break;
              case EntityType::Work:
            createOverviewTab();
            createSubTab(tr("Recordings"), "recordings", EntityType::Recording);
            createSubTab(tr("Artists"), "artist-credits", EntityType::Artist);
            createAliasesTab();
            createTagsTab();
            createRelationshipsTab();
            break;
            
        case EntityType::Label:
            createSubTab(tr("Releases"), "releases", EntityType::Release);
            createSubTab(tr("Artists"), "artists", EntityType::Artist);
            break;
            
        case EntityType::Area:
            createSubTab(tr("Artists"), "artists", EntityType::Artist);
            createSubTab(tr("Labels"), "labels", EntityType::Label);
            createSubTab(tr("Places"), "places", EntityType::Place);
            break;
            
        case EntityType::Place:
            createSubTab(tr("Artists"), "artists", EntityType::Artist);
            createSubTab(tr("Labels"), "labels", EntityType::Label);
            break;
            
        default:
            // 为未知类型创建一个通用的相关项目标签页
            qCDebug(logUI) << "Unknown entity type for sub tabs:" << static_cast<int>(itemType);
            break;
    }
}

void ItemDetailTab::createSubTab(const QString &title, const QString &key, EntityType /*entityType*/)
{
    EntityListWidget *listWidget = new EntityListWidget();
    
    // 连接信号
    connect(listWidget, &EntityListWidget::itemDoubleClicked,
            this, &ItemDetailTab::onSubItemDoubleClicked);
    connect(listWidget, &EntityListWidget::openInBrowser,
            this, &ItemDetailTab::onOpenInBrowser);
    connect(listWidget, &EntityListWidget::copyId,
            this, &ItemDetailTab::onCopyId);
    
    // 添加到标签页
    ui->subTabWidget->addTab(listWidget, title);
    
    // 保存映射
    m_subTabWidgets[key] = listWidget;
    m_subTabKeys << key;
    
    qCDebug(logUI) << "Created sub tab:" << title << "with key:" << key;
}

void ItemDetailTab::createOverviewTab()
{
    // 创建概览标签页 - 显示基本信息和摘要
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *overviewWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(overviewWidget);
    
    // 获取项目详细数据
    QVariantMap detailData = m_item->getDetailData();
    EntityType itemType = m_item->getType();
    
    // 基本信息区域
    QWidget *basicInfoWidget = new QWidget();
    QVBoxLayout *basicInfoLayout = new QVBoxLayout(basicInfoWidget);
    basicInfoWidget->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
    
    // 标题
    QLabel *basicInfoTitle = new QLabel(tr("<h3>Basic Information</h3>"));
    basicInfoTitle->setAlignment(Qt::AlignLeft);
    basicInfoLayout->addWidget(basicInfoTitle);
    
    // MBID
    QString mbid = m_item->getId();
    if (!mbid.isEmpty()) {
        QWidget *mbidItem = UiUtils::createInfoItem(tr("MBID"), mbid, overviewWidget);
        basicInfoLayout->addWidget(mbidItem);
    }
    
    // 根据实体类型显示不同的字段
    switch (itemType) {
    case EntityType::Artist:
        populateArtistOverview(basicInfoLayout, detailData, overviewWidget);
        break;
    case EntityType::Release:
        populateReleaseOverview(basicInfoLayout, detailData, overviewWidget);
        break;
    case EntityType::Recording:
        populateRecordingOverview(basicInfoLayout, detailData, overviewWidget);
        break;
    case EntityType::ReleaseGroup:
        populateReleaseGroupOverview(basicInfoLayout, detailData, overviewWidget);
        break;
    case EntityType::Label:
        populateLabelOverview(basicInfoLayout, detailData, overviewWidget);
        break;
    case EntityType::Work:
        populateWorkOverview(basicInfoLayout, detailData, overviewWidget);
        break;
    default:
        break;
    }
    
    // 评分信息（如果有）
    QVariantMap rating = detailData.value("rating", QVariantMap()).toMap();
    if (!rating.isEmpty()) {
        double ratingValue = rating.value("value", 0.0).toDouble();
        int votesCount = rating.value("votes-count", 0).toInt();
        if (ratingValue > 0) {
            QString ratingText = QString("%1/5 (%2 votes)").arg(ratingValue, 0, 'f', 1).arg(votesCount);
            QWidget *ratingItem = UiUtils::createInfoItem(tr("Rating"), ratingText, overviewWidget);
            basicInfoLayout->addWidget(ratingItem);
        }
    }
    
    // 消歧信息
    QString disambiguation = m_item->getDisambiguation();
    if (!disambiguation.isEmpty()) {
        QWidget *disambiguationItem = UiUtils::createInfoItem(tr("Disambiguation"), disambiguation, overviewWidget);
        basicInfoLayout->addWidget(disambiguationItem);
    }
    
    layout->addWidget(basicInfoWidget);
      // 艺术家信息区域（对于有artist-credit的实体）
    QVariantList artistCredits = detailData.value("artist-credit", QVariantList()).toList();
    if (!artistCredits.isEmpty()) {
        layout->addWidget(createArtistCreditWidget(artistCredits));
    }
    
    // 流派信息
    QVariantList genres = detailData.value("genres", QVariantList()).toList();
    if (!genres.isEmpty()) {
        layout->addWidget(createGenresWidget(genres));
    }
    
    layout->addStretch();
    
    overviewWidget->setLayout(layout);
    scrollArea->setWidget(overviewWidget);
    scrollArea->setWidgetResizable(true);
    
    ui->subTabWidget->addTab(scrollArea, tr("Overview"));
    
    qCDebug(logUI) << "Created Overview tab with enhanced data for" << static_cast<int>(itemType);
}

void ItemDetailTab::createAliasesTab()
{
    // 创建别名标签页
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *aliasesWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(aliasesWidget);
    
    // 获取别名数据
    QVariantMap detailData = m_item->getDetailData();
    QVariantList aliases = detailData.value("aliases", QVariantList()).toList();
    
    if (aliases.isEmpty()) {
        QLabel *noAliasesLabel = new QLabel(tr("No aliases found for this release group."));
        noAliasesLabel->setStyleSheet("padding: 20px; text-align: center; color: #666; font-style: italic;");
        noAliasesLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(noAliasesLabel);
    } else {
        // 创建别名列表
        for (const QVariant &aliasVar : aliases) {
            QVariantMap alias = aliasVar.toMap();
            
            QWidget *aliasItem = new QWidget();
            QVBoxLayout *aliasLayout = new QVBoxLayout(aliasItem);
            aliasItem->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
            
            // 别名名称
            QString aliasName = alias.value("name", "").toString();
            if (!aliasName.isEmpty()) {
                QLabel *nameLabel = new QLabel(QString("<b>%1</b>").arg(aliasName));
                nameLabel->setStyleSheet("font-size: 14px; margin-bottom: 5px;");
                aliasLayout->addWidget(nameLabel);
            }
            
            // 别名类型
            QString aliasType = alias.value("type", "").toString();
            if (!aliasType.isEmpty()) {
                QLabel *typeLabel = new QLabel(tr("Type: %1").arg(aliasType));
                typeLabel->setStyleSheet("color: #666; font-size: 12px;");
                aliasLayout->addWidget(typeLabel);
            }
            
            // 排序名称
            QString sortName = alias.value("sort-name", "").toString();
            if (!sortName.isEmpty() && sortName != aliasName) {
                QLabel *sortLabel = new QLabel(tr("Sort name: %1").arg(sortName));
                sortLabel->setStyleSheet("color: #666; font-size: 12px;");
                aliasLayout->addWidget(sortLabel);
            }
            
            // 语言和地区
            QString locale = alias.value("locale", "").toString();
            if (!locale.isEmpty()) {
                QLabel *localeLabel = new QLabel(tr("Locale: %1").arg(locale));
                localeLabel->setStyleSheet("color: #666; font-size: 12px;");
                aliasLayout->addWidget(localeLabel);
            }
            
            // 是否为主要别名
            bool isPrimary = alias.value("primary", false).toBool();
            if (isPrimary) {
                QLabel *primaryLabel = new QLabel(tr("Primary alias"));
                primaryLabel->setStyleSheet("color: #28a745; font-weight: bold; font-size: 12px;");
                aliasLayout->addWidget(primaryLabel);
            }
            
            layout->addWidget(aliasItem);
        }
    }
    
    layout->addStretch();
    
    aliasesWidget->setLayout(layout);
    scrollArea->setWidget(aliasesWidget);
    scrollArea->setWidgetResizable(true);
    
    ui->subTabWidget->addTab(scrollArea, tr("Aliases"));
    
    qCDebug(logUI) << "Created Aliases tab with" << aliases.count() << "aliases";
}

void ItemDetailTab::createTagsTab()
{
    // 创建标签页
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *tagsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tagsWidget);
    
    // 获取标签和流派数据
    QVariantMap detailData = m_item->getDetailData();
    QVariantList tags = detailData.value("tags", QVariantList()).toList();
    QVariantList genres = detailData.value("genres", QVariantList()).toList();
    
    bool hasData = !tags.isEmpty() || !genres.isEmpty();
    
    if (!hasData) {
        QString entityTypeName = m_item->getType() == EntityType::ReleaseGroup ? tr("release group") : 
                                 m_item->getType() == EntityType::Artist ? tr("artist") :
                                 m_item->getType() == EntityType::Release ? tr("release") :
                                 m_item->getType() == EntityType::Recording ? tr("recording") :
                                 m_item->getType() == EntityType::Work ? tr("work") :
                                 m_item->getType() == EntityType::Label ? tr("label") : tr("entity");
        
        QLabel *noDataLabel = new QLabel(tr("No tags or genres found for this %1.").arg(entityTypeName));
        noDataLabel->setStyleSheet("padding: 20px; text-align: center; color: #666; font-style: italic;");
        noDataLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(noDataLabel);
    } else {
        // 流派部分
        if (!genres.isEmpty()) {
            QWidget *genresSection = createGenresSection(genres);
            layout->addWidget(genresSection);
        }
        
        // 标签部分
        if (!tags.isEmpty()) {
            QWidget *tagsSection = createTagsSection(tags);
            layout->addWidget(tagsSection);
        }
    }
    
    layout->addStretch();
    
    tagsWidget->setLayout(layout);
    scrollArea->setWidget(tagsWidget);
    scrollArea->setWidgetResizable(true);
    
    ui->subTabWidget->addTab(scrollArea, tr("Tags & Genres"));
    
    qCDebug(logUI) << "Created Tags & Genres tab with" << tags.size() << "tags and" << genres.size() << "genres";
}

QWidget* ItemDetailTab::createGenresSection(const QVariantList &genres)
{
    QWidget *genresSection = new QWidget();
    QVBoxLayout *genresLayout = new QVBoxLayout(genresSection);
    genresSection->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
    
    // 标题
    QLabel *genresTitle = new QLabel(tr("<h3>Genres</h3>"));
    genresTitle->setAlignment(Qt::AlignLeft);
    genresLayout->addWidget(genresTitle);
    
    // 说明
    QLabel *genresDesc = new QLabel(tr("Genres are standardized musical categories:"));
    genresDesc->setStyleSheet("font-size: 12px; color: #666; margin-bottom: 10px;");
    genresDesc->setWordWrap(true);
    genresLayout->addWidget(genresDesc);
    
    // 创建流派网格
    QWidget *genresContainer = new QWidget();
    QGridLayout *genresGrid = new QGridLayout(genresContainer);
    genresGrid->setSpacing(8);
    
    int row = 0, col = 0;
    const int maxCols = 4;
    
    for (const QVariant &genreVar : genres) {
        QVariantMap genre = genreVar.toMap();
        QString genreName = genre.value("name", "").toString();
        int genreCount = genre.value("count", 0).toInt();
        
        if (!genreName.isEmpty()) {
            QWidget *genreItem = new QWidget();
            QHBoxLayout *genreLayout = new QHBoxLayout(genreItem);
            genreLayout->setContentsMargins(10, 6, 10, 6);
            
            genreItem->setStyleSheet("QWidget { background-color: #28a745; color: #ffffff; border-radius: 12px; font-weight: bold; }");
            
            QLabel *genreLabel = new QLabel(genreName);
            genreLabel->setStyleSheet("border: none; background: transparent;");
            genreLayout->addWidget(genreLabel);
            
            if (genreCount > 0) {
                QLabel *countLabel = new QLabel(QString("(%1)").arg(genreCount));
                countLabel->setStyleSheet("border: none; background: transparent; font-size: 10px; opacity: 0.8;");
                genreLayout->addWidget(countLabel);
            }
            
            genresGrid->addWidget(genreItem, row, col);
            
            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
    }
    
    genresLayout->addWidget(genresContainer);
    return genresSection;
}

QWidget* ItemDetailTab::createTagsSection(const QVariantList &tags)
{
    QWidget *tagsSection = new QWidget();
    QVBoxLayout *tagsLayout = new QVBoxLayout(tagsSection);
    tagsSection->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
    
    // 标题
    QLabel *tagsTitle = new QLabel(tr("<h3>User Tags</h3>"));
    tagsTitle->setAlignment(Qt::AlignLeft);
    tagsLayout->addWidget(tagsTitle);
    
    // 说明
    QLabel *tagsDesc = new QLabel(tr("Tags are user-generated keywords that describe this entity:"));
    tagsDesc->setStyleSheet("font-size: 12px; color: #666; margin-bottom: 10px;");
    tagsDesc->setWordWrap(true);
    tagsLayout->addWidget(tagsDesc);
    
    // 创建标签网格
    QWidget *tagsContainer = new QWidget();
    QGridLayout *tagsGrid = new QGridLayout(tagsContainer);
    tagsGrid->setSpacing(8);
    
    // 按照投票数排序标签
    QVariantList sortedTags = tags;
    std::sort(sortedTags.begin(), sortedTags.end(), [](const QVariant &a, const QVariant &b) {
        QVariantMap tagA = a.toMap();
        QVariantMap tagB = b.toMap();
        int countA = tagA.value("count", 0).toInt();
        int countB = tagB.value("count", 0).toInt();
        return countA > countB;
    });
    
    int row = 0, col = 0;
    const int maxCols = 3;
    
    for (const QVariant &tagVar : sortedTags) {
        QVariantMap tag = tagVar.toMap();
        QString tagName = tag.value("name", "").toString();
        int tagCount = tag.value("count", 0).toInt();
        
        if (!tagName.isEmpty()) {
            QWidget *tagItem = new QWidget();
            QHBoxLayout *tagLayout = new QHBoxLayout(tagItem);
            tagLayout->setContentsMargins(8, 4, 8, 4);
            
            // 根据标签使用频率设置样式
            QString bgColor = "#e9ecef";
            QString textColor = "#495057";
            if (tagCount >= 10) {
                bgColor = "#007bff";
                textColor = "#ffffff";
            } else if (tagCount >= 5) {
                bgColor = "#6c757d";
                textColor = "#ffffff";
            }
            
            tagItem->setStyleSheet(QString("QWidget { background-color: %1; color: %2; border-radius: 15px; font-size: 12px; }")
                                  .arg(bgColor, textColor));
            
            QLabel *tagLabel = new QLabel(tagName);
            tagLabel->setStyleSheet("border: none; background: transparent;");
            tagLayout->addWidget(tagLabel);
            
            if (tagCount > 0) {
                QLabel *countLabel = new QLabel(QString("(%1)").arg(tagCount));
                countLabel->setStyleSheet("border: none; background: transparent; font-size: 10px; opacity: 0.8;");
                tagLayout->addWidget(countLabel);
            }
            
            tagsGrid->addWidget(tagItem, row, col);
            
            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
    }
      tagsLayout->addWidget(tagsContainer);
    return tagsSection;
}

void ItemDetailTab::createRelationshipsTab()
{
    // 创建关系标签页
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *relationshipsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(relationshipsWidget);
    
    // 获取关系数据
    QVariantMap detailData = m_item->getDetailData();
    QVariantList relations = detailData.value("relations", QVariantList()).toList();
    
    if (relations.isEmpty()) {
        QLabel *noRelationsLabel = new QLabel(tr("No relationships found for this release group."));
        noRelationsLabel->setStyleSheet("padding: 20px; text-align: center; color: #666; font-style: italic;");
        noRelationsLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(noRelationsLabel);
    } else {
        // 按关系类型分组
        QMap<QString, QVariantList> relationsByType;
        for (const QVariant &relationVar : relations) {
            QVariantMap relation = relationVar.toMap();
            QString type = relation.value("type", tr("Unknown")).toString();
            relationsByType[type].append(relationVar);
        }
        
        // 为每种关系类型创建区域
        for (auto it = relationsByType.begin(); it != relationsByType.end(); ++it) {
            const QString &relationType = it.key();
            const QVariantList &typeRelations = it.value();
            
            QWidget *typeWidget = new QWidget();
            QVBoxLayout *typeLayout = new QVBoxLayout(typeWidget);
            typeWidget->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
            
            // 关系类型标题
            QLabel *typeTitle = new QLabel(QString("<h4>%1</h4>").arg(relationType));
            typeTitle->setAlignment(Qt::AlignLeft);
            typeLayout->addWidget(typeTitle);
            
            // 关系项目
            for (const QVariant &relationVar : typeRelations) {
                QVariantMap relation = relationVar.toMap();
                
                QWidget *relationItem = new QWidget();
                QHBoxLayout *relationLayout = new QHBoxLayout(relationItem);
                relationItem->setStyleSheet("QWidget { background-color: #ffffff; border: 1px solid #e9ecef; border-radius: 3px; padding: 8px; margin: 2px; }");
                
                // 目标实体信息
                QString targetType = relation.value("target-type", "").toString();
                QVariantMap target;
                
                if (targetType == "artist") {
                    target = relation.value("artist", QVariantMap()).toMap();
                } else if (targetType == "release-group") {
                    target = relation.value("release-group", QVariantMap()).toMap();
                } else if (targetType == "release") {
                    target = relation.value("release", QVariantMap()).toMap();
                } else if (targetType == "recording") {
                    target = relation.value("recording", QVariantMap()).toMap();
                } else if (targetType == "work") {
                    target = relation.value("work", QVariantMap()).toMap();
                } else if (targetType == "url") {
                    target = relation.value("url", QVariantMap()).toMap();
                }
                
                QString targetName = target.value("name", target.value("resource", "")).toString();
                QString targetId = target.value("id", "").toString();
                
                if (!targetName.isEmpty()) {
                    QLabel *targetLabel = new QLabel(targetName);
                    if (!targetId.isEmpty()) {
                        targetLabel->setStyleSheet("color: #0066cc; text-decoration: underline; cursor: pointer;");
                        targetLabel->setToolTip(tr("Click to view %1").arg(targetName));
                    }
                    relationLayout->addWidget(targetLabel);
                }
                
                // 关系方向
                QString direction = relation.value("direction", "").toString();
                if (direction == "backward") {
                    QLabel *dirLabel = new QLabel(tr("(reverse)"));
                    dirLabel->setStyleSheet("color: #666; font-size: 11px; font-style: italic;");
                    relationLayout->addWidget(dirLabel);
                }
                
                relationLayout->addStretch();
                
                // 日期信息
                QString beginDate = relation.value("begin", "").toString();
                QString endDate = relation.value("end", "").toString();
                if (!beginDate.isEmpty() || !endDate.isEmpty()) {
                    QString dateStr;
                    if (!beginDate.isEmpty() && !endDate.isEmpty()) {
                        dateStr = QString("%1 - %2").arg(beginDate, endDate);
                    } else if (!beginDate.isEmpty()) {
                        dateStr = tr("from %1").arg(beginDate);
                    } else {
                        dateStr = tr("until %1").arg(endDate);
                    }
                    
                    QLabel *dateLabel = new QLabel(dateStr);
                    dateLabel->setStyleSheet("color: #666; font-size: 11px;");
                    relationLayout->addWidget(dateLabel);
                }
                
                typeLayout->addWidget(relationItem);
            }
            
            layout->addWidget(typeWidget);
        }
    }
    
    layout->addStretch();
    
    relationshipsWidget->setLayout(layout);
    scrollArea->setWidget(relationshipsWidget);
    scrollArea->setWidgetResizable(true);
    
    ui->subTabWidget->addTab(scrollArea, tr("Relationships"));
    
    qCDebug(logUI) << "Created Relationships tab with" << relations.count() << "relationships";
}

void ItemDetailTab::populateItemInfo()
{
    if (!m_item) return;
    
    // 设置基本信息
    ui->nameLabel->setText(m_item->getName());
    ui->typeLabel->setText(m_item->getTypeString());
    
    qCDebug(logUI) << "ItemDetailTab::populateItemInfo - Item:" << m_item->getName() << "Type:" << m_item->getTypeString();
    
    // 清空现有的条目
    clearInfoLayout(ui->infoContentLayout);
    clearInfoLayout(ui->detailsContentLayout);

    // 填充信息区域
    QMap<QString, QVariant> details = m_item->getDetails();
    populateInfoSection(ui->infoContentLayout, details);

    // 填充详细信息区域
    QVariantMap detailData = m_item->getDetailData();
    populateDetailsSection(ui->detailsContentLayout, detailData);

    qCDebug(logUI) << "Populated item info for:" << m_item->getName() 
                   << "with" << details.size() << "basic fields and" 
                   << detailData.size() << "detail fields";
}

void ItemDetailTab::populateSubTabs()
{
    if (!m_item) return;
    
    qCDebug(logUI) << "ItemDetailTab::populateSubTabs - Starting for item:" << m_item->getName();
    qCDebug(logUI) << "Available detail keys:" << m_item->getDetailData().keys();
    
    // 遍历所有子标签页，填充数据
    for (auto it = m_subTabWidgets.begin(); it != m_subTabWidgets.end(); ++it) {
        const QString &key = it.key();
        EntityListWidget *widget = it.value();
        
        qCDebug(logUI) << "Processing sub tab key:" << key;
        
        // 从item中获取子数据
        QVariant subData = m_item->getDetailProperty(key);
        qCDebug(logUI) << "Sub data for key" << key << ":" << subData.typeName() << "canConvert:" << subData.canConvert<QVariantList>();
        
        QList<QSharedPointer<ResultItem>> resultItems;
        
        // 特殊处理不同的数据键
        if (key == "artist-credits" || key == "artist-credit") {
            // 尝试两种可能的键名
            QVariant artistCreditData = m_item->getDetailProperty("artist-credit");
            if (!artistCreditData.isValid()) {
                artistCreditData = m_item->getDetailProperty("artist-credits");
            }
            
            if (artistCreditData.canConvert<QVariantList>()) {
                QVariantList artistList = artistCreditData.toList();
                qCDebug(logUI) << "Artist credit list size:" << artistList.size();
                
                for (const QVariant &artistData : artistList) {
                    if (artistData.canConvert<QVariantMap>()) {
                        QVariantMap artistMap = artistData.toMap();
                        QVariantMap artistInfo = artistMap.value("artist").toMap();
                        
                        QString id = artistInfo.value("id").toString();
                        QString name = artistInfo.value("name").toString();
                        
                        if (!id.isEmpty() && !name.isEmpty()) {
                            QSharedPointer<ResultItem> resultItem = QSharedPointer<ResultItem>::create(id, name, EntityType::Artist);
                            
                            // 设置其他属性
                            for (auto mapIt = artistInfo.begin(); mapIt != artistInfo.end(); ++mapIt) {
                                resultItem->setDetailProperty(mapIt.key(), mapIt.value());
                            }
                            
                            resultItems << resultItem;
                            qCDebug(logUI) << "Created artist item:" << name << "id:" << id;
                        }
                    }
                }
            }
        } else if (key == "recordings") {
            // 处理recordings数据 - 可能在media数组中
            QVariant mediaData = m_item->getDetailProperty("media");
            if (mediaData.canConvert<QVariantList>()) {
                QVariantList mediaList = mediaData.toList();
                qCDebug(logUI) << "Media list size:" << mediaList.size();
                
                for (const QVariant &mediaItem : mediaList) {
                    if (mediaItem.canConvert<QVariantMap>()) {
                        QVariantMap mediaMap = mediaItem.toMap();
                        QVariant tracksData = mediaMap.value("tracks");
                        
                        if (tracksData.canConvert<QVariantList>()) {
                            QVariantList tracksList = tracksData.toList();
                            
                            for (const QVariant &trackData : tracksList) {
                                if (trackData.canConvert<QVariantMap>()) {
                                    QVariantMap trackMap = trackData.toMap();
                                    QVariantMap recordingMap = trackMap.value("recording").toMap();
                                    
                                    QString id = recordingMap.value("id").toString();
                                    QString title = recordingMap.value("title").toString();
                                    
                                    if (!id.isEmpty() && !title.isEmpty()) {
                                        QSharedPointer<ResultItem> resultItem = QSharedPointer<ResultItem>::create(id, title, EntityType::Recording);
                                        
                                        // 设置录音属性
                                        for (auto mapIt = recordingMap.begin(); mapIt != recordingMap.end(); ++mapIt) {
                                            resultItem->setDetailProperty(mapIt.key(), mapIt.value());
                                        }
                                        
                                        // 设置曲目相关属性
                                        resultItem->setDetailProperty("track_number", trackMap.value("number"));
                                        resultItem->setDetailProperty("track_title", trackMap.value("title"));
                                        
                                        resultItems << resultItem;
                                        qCDebug(logUI) << "Created recording item:" << title << "id:" << id;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else if (subData.canConvert<QVariantList>()) {
            // 通用处理方式
            QVariantList itemList = subData.toList();
            qCDebug(logUI) << "Sub data list size:" << itemList.size();
            
            for (const QVariant &itemData : itemList) {
                if (itemData.canConvert<QVariantMap>()) {
                    QVariantMap itemMap = itemData.toMap();
                    
                    // 根据标签页类型设置实体类型
                    EntityType entityType = EntityType::Unknown;
                    if (key == "releases") entityType = EntityType::Release;
                    else if (key == "release-groups") entityType = EntityType::ReleaseGroup;
                    else if (key == "recordings") entityType = EntityType::Recording;
                    else if (key == "works") entityType = EntityType::Work;
                    else if (key == "artists") entityType = EntityType::Artist;
                    
                    // 创建ResultItem
                    QString id = itemMap.value("id").toString();
                    QString name = itemMap.value("title", itemMap.value("name")).toString();
                    
                    if (!id.isEmpty() && !name.isEmpty()) {
                        QSharedPointer<ResultItem> resultItem = QSharedPointer<ResultItem>::create(id, name, entityType);
                        
                        // 设置其他详细属性
                        for (auto mapIt = itemMap.begin(); mapIt != itemMap.end(); ++mapIt) {
                            resultItem->setDetailProperty(mapIt.key(), mapIt.value());
                        }
                        
                        resultItems << resultItem;
                        qCDebug(logUI) << "Created result item:" << name << "id:" << id;
                    }
                }
            }
        } else {
            qCDebug(logUI) << "No valid data for sub tab:" << key;
        }
        
        // 设置到EntityListWidget
        widget->setItems(resultItems);
        qCDebug(logUI) << "Populated sub tab" << key << "with" << resultItems.size() << "items";
    }
}

void ItemDetailTab::refreshItemInfo()
{
    populateItemInfo();
    populateSubTabs();
}

void ItemDetailTab::onSubTabChanged(int index)
{
    qCDebug(logUI) << "Sub tab changed to index:" << index;
}

void ItemDetailTab::onSubItemDoubleClicked(QSharedPointer<ResultItem> item)
{
    if (item) {
        emit openDetailTab(item);
    }
}

void ItemDetailTab::onOpenInBrowser(const QString &itemId, EntityType type)
{
    // 转发信号到MainWindow处理
    emit openInBrowser(itemId, type);
    qCDebug(logUI) << "Forwarded openInBrowser signal for:" << itemId;
}

void ItemDetailTab::onCopyId(const QString &itemId)
{
    // 转发信号到MainWindow处理
    emit copyId(itemId);    qCDebug(logUI) << "Forwarded copyId signal for:" << itemId;
}

void ItemDetailTab::clearInfoLayout(QVBoxLayout *layout)
{
    // 清空布局中的所有项目
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void ItemDetailTab::populateInfoSection(QVBoxLayout *layout, const QMap<QString, QVariant> &data)
{
    // 核心信息字段映射 - 只显示最重要的基本信息
    QMap<QString, QString> fieldNames = {
        {"area", tr("Area")},
        {"country", tr("Country")},
        {"disambiguation", tr("Disambiguation")},
        {"id", tr("ID")},
        {"subtype", tr("Subtype")},
        {"type", tr("Type")},
        {"score", tr("Score")}
    };
    
    bool hasData = false;
    for (auto it = data.begin(); it != data.end(); ++it) {
        const QString &key = it.key();
        const QVariant &value = it.value();
          // 只显示在fieldNames中定义的字段，且值非空
        if (fieldNames.contains(key) && value.canConvert<QString>() && !value.toString().isEmpty()) {
            QString displayName = fieldNames.value(key);
            QWidget *infoItem = UiUtils::createInfoItem(displayName, value.toString(), layout->parentWidget());
            layout->addWidget(infoItem);
            hasData = true;
        }
    }
    
    if (!hasData) {
        QLabel *noDataLabel = new QLabel(tr("No basic information available."));
        noDataLabel->setStyleSheet("color: #666; font-style: italic;");
        layout->addWidget(noDataLabel);
    }
    
    // 添加弹性空间
    layout->addStretch();
}

void ItemDetailTab::populateDetailsSection(QVBoxLayout *layout, const QVariantMap &data)
{
    // 详细信息字段映射 - 只显示有用的字段
    QMap<QString, QString> fieldNames = {
        {"begin", tr("Begin")},
        {"end", tr("End")},
        {"ended", tr("Ended")},
        {"gender", tr("Gender")},
        {"life-span", tr("Life Span")},
        {"sort-name", tr("Sort Name")},
        {"status", tr("Status")},
        {"barcode", tr("Barcode")},
        {"date", tr("Date")},
        {"packaging", tr("Packaging")},
        {"country", tr("Country")},
        {"length", tr("Length")},
        {"quality", tr("Quality")},
        {"track-count", tr("Track Count")}
    };
    
    bool hasData = false;
    for (auto it = data.begin(); it != data.end(); ++it) {
        const QString &key = it.key();
        const QVariant &value = it.value();
        
        // 跳过子项列表数据
        if (key == "releases" || key == "release-groups" || key == "recordings" || 
            key == "works" || key == "artists" || key == "labels" || key == "places" ||
            key == "tags" || key == "rating" || key == "aliases") {
            continue;
        }
        
        // 只显示在fieldNames中定义的有用字段
        if (!fieldNames.contains(key)) {
            continue;
        }
          // 处理不同类型的数据
        if (value.canConvert<QString>() && !value.toString().isEmpty()) {
            QString displayName = fieldNames.value(key);
            QWidget *infoItem = UiUtils::createInfoItem(displayName, value.toString(), layout->parentWidget());
            layout->addWidget(infoItem);
            hasData = true;
        } else if (value.canConvert<QVariantMap>()) {
            // 处理复杂对象
            QVariantMap subMap = value.toMap();
            QString displayValue = formatComplexValue(subMap);
            if (!displayValue.isEmpty()) {
                QString displayName = fieldNames.value(key);
                QWidget *infoItem = UiUtils::createInfoItem(displayName, displayValue, layout->parentWidget());
                layout->addWidget(infoItem);
                hasData = true;
            }
        }
    }
    
    if (!hasData) {
        QLabel *noDataLabel = new QLabel(tr("No detailed information available."));
        noDataLabel->setStyleSheet("color: #666; font-style: italic;");
        layout->addWidget(noDataLabel);
    }
      // 添加弹性空间
    layout->addStretch();
}

QString ItemDetailTab::formatComplexValue(const QVariantMap &map)
{
    QStringList parts;
    
    // 处理常见的复杂值字段
    if (map.contains("name")) {
        parts << map.value("name").toString();
    }
    if (map.contains("title")) {
        parts << map.value("title").toString();
    }
    if (map.contains("language")) {
        parts << QString("(%1)").arg(map.value("language").toString());
    }
    if (map.contains("script")) {
        parts << QString("[%1]").arg(map.value("script").toString());
    }
    
    return parts.join(" ");
}

void ItemDetailTab::populateArtistOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    // 艺术家类型
    QString type = detailData.value("type", "").toString();
    if (!type.isEmpty()) {
        QWidget *typeItem = UiUtils::createInfoItem(tr("Type"), type, parent);
        layout->addWidget(typeItem);
    }
    
    // 性别
    QString gender = detailData.value("gender", "").toString();
    if (!gender.isEmpty()) {
        QWidget *genderItem = UiUtils::createInfoItem(tr("Gender"), gender, parent);
        layout->addWidget(genderItem);
    }
    
    // 生命周期
    QVariantMap lifeSpan = detailData.value("life-span", QVariantMap()).toMap();
    if (!lifeSpan.isEmpty()) {
        QString begin = lifeSpan.value("begin", "").toString();
        QString end = lifeSpan.value("end", "").toString();
        QString ended = lifeSpan.value("ended", false).toBool() ? tr("ended") : tr("active");
        
        QString lifeSpanText;
        if (!begin.isEmpty()) {
            lifeSpanText = begin;
            if (!end.isEmpty()) {
                lifeSpanText += " - " + end;
            } else if (lifeSpan.value("ended", false).toBool()) {
                lifeSpanText += " - " + tr("ended");
            } else {
                lifeSpanText += " - " + tr("present");
            }
        }
        
        if (!lifeSpanText.isEmpty()) {
            QWidget *lifeSpanItem = UiUtils::createInfoItem(tr("Life Span"), lifeSpanText, parent);
            layout->addWidget(lifeSpanItem);
        }
    }
    
    // 地区
    QVariantMap area = detailData.value("area", QVariantMap()).toMap();
    if (!area.isEmpty()) {
        QString areaName = area.value("name", "").toString();
        if (!areaName.isEmpty()) {
            QWidget *areaItem = UiUtils::createInfoItem(tr("Area"), areaName, parent);
            layout->addWidget(areaItem);
        }
    }
    
    // 开始地区
    QVariantMap beginArea = detailData.value("begin-area", QVariantMap()).toMap();
    if (!beginArea.isEmpty()) {
        QString beginAreaName = beginArea.value("name", "").toString();
        if (!beginAreaName.isEmpty()) {
            QWidget *beginAreaItem = UiUtils::createInfoItem(tr("Begin Area"), beginAreaName, parent);
            layout->addWidget(beginAreaItem);
        }
    }
}

void ItemDetailTab::populateReleaseOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    // 发行状态
    QString status = detailData.value("status", "").toString();
    if (!status.isEmpty()) {
        QWidget *statusItem = UiUtils::createInfoItem(tr("Status"), status, parent);
        layout->addWidget(statusItem);
    }
    
    // 发行日期
    QString date = detailData.value("date", "").toString();
    if (!date.isEmpty()) {
        QWidget *dateItem = UiUtils::createInfoItem(tr("Release Date"), date, parent);
        layout->addWidget(dateItem);
    }
    
    // 包装
    QString packaging = detailData.value("packaging", "").toString();
    if (!packaging.isEmpty()) {
        QWidget *packagingItem = UiUtils::createInfoItem(tr("Packaging"), packaging, parent);
        layout->addWidget(packagingItem);
    }
    
    // 条形码
    QString barcode = detailData.value("barcode", "").toString();
    if (!barcode.isEmpty()) {
        QWidget *barcodeItem = UiUtils::createInfoItem(tr("Barcode"), barcode, parent);
        layout->addWidget(barcodeItem);
    }
    
    // 国家
    QVariantMap releaseCountry = detailData.value("release-country", QVariantMap()).toMap();
    if (!releaseCountry.isEmpty()) {
        QString countryName = releaseCountry.value("name", "").toString();
        if (!countryName.isEmpty()) {
            QWidget *countryItem = UiUtils::createInfoItem(tr("Country"), countryName, parent);
            layout->addWidget(countryItem);
        }
    }
    
    // 音轨数量
    QVariantList media = detailData.value("media", QVariantList()).toList();
    if (!media.isEmpty()) {
        int totalTracks = 0;
        QStringList mediaInfo;
        for (const QVariant &medium : media) {
            QVariantMap mediumMap = medium.toMap();
            int trackCount = mediumMap.value("track-count", 0).toInt();
            QString format = mediumMap.value("format", "").toString();
            totalTracks += trackCount;
            if (!format.isEmpty()) {
                mediaInfo << QString("%1 (%2 tracks)").arg(format).arg(trackCount);
            }
        }
        
        if (totalTracks > 0) {
            QWidget *tracksItem = UiUtils::createInfoItem(tr("Total Tracks"), QString::number(totalTracks), parent);
            layout->addWidget(tracksItem);
        }
        
        if (!mediaInfo.isEmpty()) {
            QWidget *mediaItem = UiUtils::createInfoItem(tr("Media"), mediaInfo.join(", "), parent);
            layout->addWidget(mediaItem);
        }
    }
}

void ItemDetailTab::populateRecordingOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    // 录音长度
    int length = detailData.value("length", 0).toInt();
    if (length > 0) {
        int minutes = length / 60000;
        int seconds = (length % 60000) / 1000;
        QString lengthText = QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QLatin1Char('0'));
        QWidget *lengthItem = UiUtils::createInfoItem(tr("Length"), lengthText, parent);
        layout->addWidget(lengthItem);
    }
    
    // ISRC
    QVariantList isrcs = detailData.value("isrcs", QVariantList()).toList();
    if (!isrcs.isEmpty()) {
        QStringList isrcStrings;
        for (const QVariant &isrc : isrcs) {
            isrcStrings << isrc.toString();
        }
        QWidget *isrcItem = UiUtils::createInfoItem(tr("ISRC"), isrcStrings.join(", "), parent);
        layout->addWidget(isrcItem);
    }
    
    // 录音状态
    QString status = detailData.value("status", "").toString();
    if (!status.isEmpty()) {
        QWidget *statusItem = UiUtils::createInfoItem(tr("Status"), status, parent);
        layout->addWidget(statusItem);
    }
}

void ItemDetailTab::populateReleaseGroupOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    // 发行组类型
    QString type = detailData.value("type", "").toString();
    if (!type.isEmpty()) {
        QWidget *typeItem = UiUtils::createInfoItem(tr("Type"), type, parent);
        layout->addWidget(typeItem);
    }
    
    // 主要类型
    QString primaryType = detailData.value("primary-type", "").toString();
    if (!primaryType.isEmpty()) {
        QWidget *primaryTypeItem = UiUtils::createInfoItem(tr("Primary Type"), primaryType, parent);
        layout->addWidget(primaryTypeItem);
    }
    
    // 次要类型
    QVariantList secondaryTypes = detailData.value("secondary-types", QVariantList()).toList();
    if (!secondaryTypes.isEmpty()) {
        QStringList typeStrings;
        for (const QVariant &type : secondaryTypes) {
            typeStrings << type.toString();
        }
        QWidget *secondaryTypesItem = UiUtils::createInfoItem(tr("Secondary Types"), typeStrings.join(", "), parent);
        layout->addWidget(secondaryTypesItem);
    }
    
    // 首次发行日期
    QString firstReleaseDate = detailData.value("first-release-date", "").toString();
    if (!firstReleaseDate.isEmpty()) {
        QWidget *dateItem = UiUtils::createInfoItem(tr("First Release Date"), firstReleaseDate, parent);
        layout->addWidget(dateItem);
    }
}

void ItemDetailTab::populateLabelOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    // 标签类型
    QString type = detailData.value("type", "").toString();
    if (!type.isEmpty()) {
        QWidget *typeItem = UiUtils::createInfoItem(tr("Type"), type, parent);
        layout->addWidget(typeItem);
    }
    
    // 标签代码
    QString labelCode = detailData.value("label-code", "").toString();
    if (!labelCode.isEmpty()) {
        QWidget *codeItem = UiUtils::createInfoItem(tr("Label Code"), labelCode, parent);
        layout->addWidget(codeItem);
    }
    
    // 地区
    QVariantMap area = detailData.value("area", QVariantMap()).toMap();
    if (!area.isEmpty()) {
        QString areaName = area.value("name", "").toString();
        if (!areaName.isEmpty()) {
            QWidget *areaItem = UiUtils::createInfoItem(tr("Area"), areaName, parent);
            layout->addWidget(areaItem);
        }
    }
    
    // 活动期间
    QVariantMap lifeSpan = detailData.value("life-span", QVariantMap()).toMap();
    if (!lifeSpan.isEmpty()) {
        QString begin = lifeSpan.value("begin", "").toString();
        QString end = lifeSpan.value("end", "").toString();
        
        QString lifeSpanText;
        if (!begin.isEmpty()) {
            lifeSpanText = begin;
            if (!end.isEmpty()) {
                lifeSpanText += " - " + end;
            } else if (!lifeSpan.value("ended", false).toBool()) {
                lifeSpanText += " - " + tr("present");
            }
        }
        
        if (!lifeSpanText.isEmpty()) {
            QWidget *lifeSpanItem = UiUtils::createInfoItem(tr("Active Period"), lifeSpanText, parent);
            layout->addWidget(lifeSpanItem);
        }
    }
}

void ItemDetailTab::populateWorkOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    // 作品类型
    QString type = detailData.value("type", "").toString();
    if (!type.isEmpty()) {
        QWidget *typeItem = UiUtils::createInfoItem(tr("Type"), type, parent);
        layout->addWidget(typeItem);
    }
    
    // 语言
    QVariantList languages = detailData.value("languages", QVariantList()).toList();
    if (!languages.isEmpty()) {
        QStringList languageStrings;
        for (const QVariant &lang : languages) {
            languageStrings << lang.toString();
        }
        QWidget *languageItem = UiUtils::createInfoItem(tr("Languages"), languageStrings.join(", "), parent);
        layout->addWidget(languageItem);
    }
    
    // ISWC
    QVariantList iswcs = detailData.value("iswcs", QVariantList()).toList();
    if (!iswcs.isEmpty()) {
        QStringList iswcStrings;
        for (const QVariant &iswc : iswcs) {
            iswcStrings << iswc.toString();
        }
        QWidget *iswcItem = UiUtils::createInfoItem(tr("ISWC"), iswcStrings.join(", "), parent);
        layout->addWidget(iswcItem);
    }
}

QWidget* ItemDetailTab::createArtistCreditWidget(const QVariantList &artistCredits)
{
    QWidget *artistWidget = new QWidget();
    QVBoxLayout *artistLayout = new QVBoxLayout(artistWidget);
    artistWidget->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
    
    QLabel *artistTitle = new QLabel(tr("<h3>Artist Credit</h3>"));
    artistTitle->setAlignment(Qt::AlignLeft);
    artistLayout->addWidget(artistTitle);
    
    for (const QVariant &credit : artistCredits) {
        QVariantMap creditMap = credit.toMap();
        QVariantMap artist = creditMap.value("artist", QVariantMap()).toMap();
        QString artistName = artist.value("name", "").toString();
        QString joinPhrase = creditMap.value("joinphrase", "").toString();
        
        if (!artistName.isEmpty()) {
            QString displayText = artistName;
            if (!joinPhrase.isEmpty()) {
                displayText += " " + joinPhrase;
            }
            QLabel *artistLabel = new QLabel(displayText);
            artistLabel->setStyleSheet("color: #0066cc; text-decoration: underline; cursor: pointer;");
            artistLayout->addWidget(artistLabel);
        }
    }
    
    return artistWidget;
}

QWidget* ItemDetailTab::createGenresWidget(const QVariantList &genres)
{
    QWidget *genreWidget = new QWidget();
    QVBoxLayout *genreLayout = new QVBoxLayout(genreWidget);
    genreWidget->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
    
    QLabel *genreTitle = new QLabel(tr("<h3>Genres</h3>"));
    genreTitle->setAlignment(Qt::AlignLeft);
    genreLayout->addWidget(genreTitle);
    
    QStringList genreNames;
    for (const QVariant &genre : genres) {
        QVariantMap genreMap = genre.toMap();
        QString name = genreMap.value("name", "").toString();
        if (!name.isEmpty()) {
            genreNames << name;
        }
    }
    
    if (!genreNames.isEmpty()) {
        QLabel *genreLabel = new QLabel(genreNames.join(", "));
        genreLabel->setWordWrap(true);
        genreLayout->addWidget(genreLabel);
    }
    
    return genreWidget;
}