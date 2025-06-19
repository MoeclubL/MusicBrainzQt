#include "itemdetailtab.h"
#include "ui_itemdetailtab.h"
#include "entitylistwidget.h"
#include "../models/resultitem.h"
#include "../utils/logger.h"
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include <QGuiApplication>
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
    
    // 基本信息区域
    QWidget *basicInfoWidget = new QWidget();
    QVBoxLayout *basicInfoLayout = new QVBoxLayout(basicInfoWidget);
    basicInfoWidget->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
    
    // 标题
    QLabel *basicInfoTitle = new QLabel(tr("<h3>Basic Information</h3>"));
    basicInfoTitle->setAlignment(Qt::AlignLeft);
    basicInfoLayout->addWidget(basicInfoTitle);
    
    // Release Group类型
    QString releaseGroupType = detailData.value("type", tr("Unknown")).toString();
    if (!releaseGroupType.isEmpty()) {
        createInfoItem(basicInfoLayout, tr("Type"), releaseGroupType);
    }
    
    // 主要类型 (primary-type)
    QString primaryType = detailData.value("primary-type", "").toString();
    if (!primaryType.isEmpty()) {
        createInfoItem(basicInfoLayout, tr("Primary Type"), primaryType);
    }
    
    // 次要类型 (secondary-types)
    QVariantList secondaryTypes = detailData.value("secondary-types", QVariantList()).toList();
    if (!secondaryTypes.isEmpty()) {
        QStringList typeStrings;
        for (const QVariant &type : secondaryTypes) {
            typeStrings << type.toString();
        }
        createInfoItem(basicInfoLayout, tr("Secondary Types"), typeStrings.join(", "));
    }
    
    // 开始时间
    QString beginDate = detailData.value("first-release-date", "").toString();
    if (!beginDate.isEmpty()) {
        createInfoItem(basicInfoLayout, tr("First Release Date"), beginDate);
    }
    
    // 消歧信息
    QString disambiguation = m_item->getDisambiguation();
    if (!disambiguation.isEmpty()) {
        createInfoItem(basicInfoLayout, tr("Disambiguation"), disambiguation);
    }
    
    layout->addWidget(basicInfoWidget);
    
    // 艺术家信息区域
    QVariantList artistCredits = detailData.value("artist-credit", QVariantList()).toList();
    if (!artistCredits.isEmpty()) {
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
        
        layout->addWidget(artistWidget);
    }
    
    layout->addStretch();
    
    overviewWidget->setLayout(layout);
    scrollArea->setWidget(overviewWidget);
    scrollArea->setWidgetResizable(true);
    
    ui->subTabWidget->addTab(scrollArea, tr("Overview"));
    
    qCDebug(logUI) << "Created Overview tab with real data";
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
    
    // 获取标签数据
    QVariantMap detailData = m_item->getDetailData();
    QVariantList tags = detailData.value("tags", QVariantList()).toList();
    
    if (tags.isEmpty()) {
        QLabel *noTagsLabel = new QLabel(tr("No tags found for this release group."));
        noTagsLabel->setStyleSheet("padding: 20px; text-align: center; color: #666; font-style: italic;");
        noTagsLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(noTagsLabel);
    } else {
        // 创建标签说明
        QLabel *descLabel = new QLabel(tr("Tags are user-generated keywords that describe this release group:"));
        descLabel->setStyleSheet("padding: 10px; font-size: 12px; color: #666; border-bottom: 1px solid #dee2e6; margin-bottom: 10px;");
        descLabel->setWordWrap(true);
        layout->addWidget(descLabel);
        
        // 创建标签网格布局
        QWidget *tagsContainer = new QWidget();
        QGridLayout *tagsGrid = new QGridLayout(tagsContainer);
        tagsGrid->setSpacing(10);
        
        // 按照投票数排序标签
        std::sort(tags.begin(), tags.end(), [](const QVariant &a, const QVariant &b) {
            QVariantMap tagA = a.toMap();
            QVariantMap tagB = b.toMap();
            int countA = tagA.value("count", 0).toInt();
            int countB = tagB.value("count", 0).toInt();
            return countA > countB;
        });
        
        int row = 0, col = 0;
        const int maxCols = 3;
        
        for (const QVariant &tagVar : tags) {
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
                
                // 显示投票数
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
        
        layout->addWidget(tagsContainer);
    }
    
    layout->addStretch();
    
    tagsWidget->setLayout(layout);
    scrollArea->setWidget(tagsWidget);
    scrollArea->setWidgetResizable(true);
    
    ui->subTabWidget->addTab(scrollArea, tr("Tags"));
    
    qCDebug(logUI) << "Created Tags tab with" << tags.count() << "tags";
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
        
        if (subData.canConvert<QVariantList>()) {
            QVariantList itemList = subData.toList();
            qCDebug(logUI) << "Sub data list size:" << itemList.size();
            
            // 转换为ResultItem列表
            QList<QSharedPointer<ResultItem>> resultItems;
            for (const QVariant &itemData : itemList) {
                if (itemData.canConvert<QVariantMap>()) {
                    QVariantMap itemMap = itemData.toMap();
                    
                    // 根据标签页类型设置实体类型
                    EntityType entityType = EntityType::Unknown;
                    if (key == "releases") entityType = EntityType::Release;
                    else if (key == "release-groups") entityType = EntityType::ReleaseGroup;
                    else if (key == "recordings") entityType = EntityType::Recording;
                    else if (key == "works") entityType = EntityType::Work;
                    
                    // 创建ResultItem
                    QString id = itemMap.value("id").toString();
                    QString name = itemMap.value("title", itemMap.value("name")).toString();
                    QSharedPointer<ResultItem> resultItem = QSharedPointer<ResultItem>::create(id, name, entityType);
                    
                    // 设置其他详细属性
                    for (auto mapIt = itemMap.begin(); mapIt != itemMap.end(); ++mapIt) {
                        resultItem->setDetailProperty(mapIt.key(), mapIt.value());
                    }
                    
                    resultItems << resultItem;
                    qCDebug(logUI) << "Created result item:" << name << "id:" << id;
                }
            }
            
            // 设置到EntityListWidget
            widget->setItems(resultItems);
            qCDebug(logUI) << "Populated sub tab" << key << "with" << resultItems.size() << "items";
        } else {
            qCDebug(logUI) << "No valid data for sub tab:" << key;
        }
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
    emit copyId(itemId);
    qCDebug(logUI) << "Forwarded copyId signal for:" << itemId;
}

QString ItemDetailTab::buildMusicBrainzUrl(const QString &itemId, EntityType type) const
{
    QString baseUrl = "https://musicbrainz.org/";
    QString typeStr;
    
    switch (type) {
        case EntityType::Artist:
            typeStr = "artist";
            break;
        case EntityType::Release:
            typeStr = "release";
            break;
        case EntityType::ReleaseGroup:
            typeStr = "release-group";
            break;
        case EntityType::Recording:
            typeStr = "recording";
            break;
        case EntityType::Work:
            typeStr = "work";
            break;
        case EntityType::Label:
            typeStr = "label";
            break;
        default:
            return QString();
    }
    
    return baseUrl + typeStr + "/" + itemId;
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
            createInfoItem(layout, displayName, value.toString());
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
            createInfoItem(layout, displayName, value.toString());
            hasData = true;
        } else if (value.canConvert<QVariantMap>()) {
            // 处理复杂对象
            QVariantMap subMap = value.toMap();
            QString displayValue = formatComplexValue(subMap);
            if (!displayValue.isEmpty()) {
                QString displayName = fieldNames.value(key);
                createInfoItem(layout, displayName, displayValue);
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

void ItemDetailTab::createInfoItem(QVBoxLayout *layout, const QString &label, const QString &value)
{
    // 创建一个包含标签和值的水平布局
    QFrame *itemFrame = new QFrame();
    itemFrame->setFrameStyle(QFrame::NoFrame);
    itemFrame->setStyleSheet("QFrame { border-bottom: 1px solid #eee; padding: 3px 0px; }");
    
    QHBoxLayout *itemLayout = new QHBoxLayout(itemFrame);
    itemLayout->setContentsMargins(0, 3, 0, 3);
    itemLayout->setSpacing(10);
    
    // 标签
    QLabel *keyLabel = new QLabel(label + ":");
    keyLabel->setStyleSheet("font-weight: bold; color: #333;");
    keyLabel->setMinimumWidth(80);
    keyLabel->setMaximumWidth(120);
    keyLabel->setWordWrap(true);
    
    // 值
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setStyleSheet("color: #555;");
    valueLabel->setWordWrap(true);
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    itemLayout->addWidget(keyLabel);
    itemLayout->addWidget(valueLabel, 1);
    
    layout->addWidget(itemFrame);
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