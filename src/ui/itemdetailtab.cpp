#include "itemdetailtab.h"
#include "ui_itemdetailtab.h"
#include "entitylistwidget.h"
#include "widget_helpers.h"
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
#include <QVariantMap>
#include <QVariantList>
#include <QDebug>
#include <QPushButton>
#include <QMap>
#include <QLoggingCategory>
#include <algorithm>

ItemDetailTab::ItemDetailTab(const QSharedPointer<ResultItem> &item, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ItemDetailTab)
    , m_item(item)
{
    ui->setupUi(this);
    
    // 检查项目是否有效
    if (!m_item) {
        qDebug() << "ItemDetailTab: Cannot create tab with null item!";
        // 隐藏所有UI组件并显示错误信息
        if (ui->mainSplitter) {
            ui->mainSplitter->setVisible(false);
        }
        
        // 创建错误显示标签并添加到现有布局中
        QLabel *errorLabel = new QLabel(tr("Error: No item data available"));
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLabel->setStyleSheet("color: red; font-size: 14px; padding: 20px;");
        
        // 将错误标签添加到主布局中
        if (layout()) {
            layout()->addWidget(errorLabel);
        } else {
            // 如果没有现有布局，创建一个新的
            QVBoxLayout *errorLayout = new QVBoxLayout(this);
            errorLayout->addWidget(errorLabel);
        }
        return;
    }
    
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
    connect(ui->subTabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(onSubTabChanged(int)));
}

void ItemDetailTab::setupSubTabs()
{
    if (!m_item) {
        qDebug() << "ItemDetailTab::setupSubTabs: m_item is null, skipping sub tabs setup";
        return;
    }
    
    // 根据实体类型创建相应的子标签页
    EntityType itemType = m_item->getType();
    
    switch (itemType) {          case EntityType::Artist:
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
            createReviewsTab();  // 新添加的Reviews页面
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
            createAliasesTab();
            createTagsTab();
            createRelationshipsTab();
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
    connect(listWidget, SIGNAL(itemDoubleClicked(QSharedPointer<ResultItem>)),
            this, SLOT(onSubItemDoubleClicked(QSharedPointer<ResultItem>)));
    connect(listWidget, SIGNAL(openInBrowser(QString,EntityType)),
            this, SLOT(onOpenInBrowser(QString,EntityType)));
    connect(listWidget, SIGNAL(copyId(QString)),
            this, SLOT(onCopyId(QString)));
    
    // 添加到标签页控件
    ui->subTabWidget->addTab(listWidget, title);
    
    // 存储映射关系
    m_subTabWidgets[key] = listWidget;
    m_subTabKeys << key;
    
    qCDebug(logUI) << "Created sub tab:" << title << "with key:" << key;
}

void ItemDetailTab::createOverviewTab()
{
    if (!m_item) {
        qCWarning(logUI) << "ItemDetailTab::createOverviewTab: m_item is null, skipping overview tab creation";
        return;
    }
    
    // 创建概览标签页 - 显示基本信息和摘要
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *overviewWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(overviewWidget);
    
    // 获取项目详细数据
    QVariantMap detailData = m_item->getDetailData();
    EntityType itemType = m_item->getType();
    
    // 基本信息部分
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
        QWidget *mbidItem = WidgetHelpers::createInfoItem(tr("MBID"), mbid, overviewWidget);
        basicInfoLayout->addWidget(mbidItem);
    }
      // 根据实体类型显示不同字段
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
    }      // 评分信息（如果可用）
    if (detailData.contains("rating")) {
        QVariantMap rating = detailData["rating"].toMap();
        if (!rating.empty()) {
            if (rating.contains("value")) {
                double ratingValue = rating["value"].toDouble();
                int votesCount = rating.contains("votes-count") ? rating["votes-count"].toInt() : 0;
                if (ratingValue > 0) {
                    QString ratingText = QString("%1/5 (%2 votes)").arg(QString::number(ratingValue, 'f', 1)).arg(QString::number(votesCount));
                    QWidget *ratingItem = WidgetHelpers::createInfoItem(tr("Rating"), ratingText, overviewWidget);
                    basicInfoLayout->addWidget(ratingItem);
                }
            }
        }
    }
      // 消歧义信息
    QString disambiguation = m_item->getDisambiguation();
    if (!disambiguation.isEmpty()) {
        QWidget *disambiguationItem = WidgetHelpers::createInfoItem(tr("Disambiguation"), disambiguation, overviewWidget);
        basicInfoLayout->addWidget(disambiguationItem);
    }    layout->addWidget(basicInfoWidget);    // 艺术家信息部分（用于有艺术家署名的实体）
    auto artistCreditsIt = detailData.find("artist-credit");
    if (artistCreditsIt != detailData.end()) {
        QVariantList artistCredits = artistCreditsIt.value().toList();
        if (!artistCredits.empty()) {
            layout->addWidget(createArtistCreditWidget(artistCredits));
        }
    }
    
    // 风格信息
    auto genresIt = detailData.find("genres");
    if (genresIt != detailData.end()) {
        QVariantList genres = genresIt.value().toList();
        if (!genres.empty()) {
            layout->addWidget(createGenresWidget(genres));
        }
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
    if (!m_item) {
        qCWarning(logUI) << "ItemDetailTab::createAliasesTab: m_item is null, skipping aliases tab creation";
        return;
    }
    
    // 创建别名标签页
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *aliasesWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(aliasesWidget);
    
    // 获取别名数据
    QVariantMap detailData = m_item->getDetailData();
    auto aliasesIt = detailData.find("aliases");
      if (aliasesIt == detailData.end() || aliasesIt.value().toList().empty()) {
        QLabel *noAliasesLabel = new QLabel(tr("No aliases found for this release group."));
        noAliasesLabel->setStyleSheet("padding: 20px; text-align: center; color: #666; font-style: italic;");
        noAliasesLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(noAliasesLabel);
    } else {        // 创建别名列表
        QVariantList aliases = aliasesIt.value().toList();
        for (const QVariant &aliasVar : aliases) {
            QVariantMap alias = aliasVar.toMap();
            
            QWidget *aliasItem = new QWidget();
            QVBoxLayout *aliasLayout = new QVBoxLayout(aliasItem);
            aliasItem->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
              // 别名名称
            auto nameIt = alias.find("name");
            if (nameIt != alias.end()) {
                QString aliasName = nameIt.value().toString();
                if (!aliasName.isEmpty()) {
                    QLabel *nameLabel = new QLabel(QString("<b>%1</b>").arg(aliasName));
                    nameLabel->setStyleSheet("font-size: 14px; margin-bottom: 5px;");
                    aliasLayout->addWidget(nameLabel);
                }
            }
            
            // 别名类型
            auto typeIt = alias.find("type");
            if (typeIt != alias.end()) {
                QString aliasType = typeIt.value().toString();
                if (!aliasType.isEmpty()) {
                    QLabel *typeLabel = new QLabel(tr("Type: %1").arg(aliasType));
                    typeLabel->setStyleSheet("color: #666; font-size: 12px;");
                    aliasLayout->addWidget(typeLabel);
                }
            }
              // 排序名称
            auto sortIt = alias.find("sort-name");
            if (sortIt != alias.end()) {
                QString sortName = sortIt.value().toString();
                QString aliasName = alias.find("name") != alias.end() ? alias.find("name").value().toString() : "";
                if (!sortName.isEmpty() && sortName != aliasName) {
                    QLabel *sortLabel = new QLabel(tr("Sort name: %1").arg(sortName));
                sortLabel->setStyleSheet("color: #666; font-size: 12px;");                }
            }
              // 语言和地区
            auto localeIt = alias.find("locale");
            if (localeIt != alias.end()) {
                QString locale = localeIt.value().toString();
                if (!locale.isEmpty()) {
                    QLabel *localeLabel = new QLabel(tr("Locale: %1").arg(locale));
                    localeLabel->setStyleSheet("color: #666; font-size: 12px;");
                    aliasLayout->addWidget(localeLabel);
                }
            }
            
            // 是否为主要别名
            auto primaryIt = alias.find("primary");
            if (primaryIt != alias.end()) {
                bool isPrimary = primaryIt.value().toBool();
                if (isPrimary) {
                    QLabel *primaryLabel = new QLabel(tr("Primary alias"));
                    primaryLabel->setStyleSheet("color: #28a745; font-weight: bold; font-size: 12px;");
                    aliasLayout->addWidget(primaryLabel);
                }
            }
            
            layout->addWidget(aliasItem);
        }
    }
    
    layout->addStretch();
    
    aliasesWidget->setLayout(layout);
    scrollArea->setWidget(aliasesWidget);
    scrollArea->setWidgetResizable(true);
      ui->subTabWidget->addTab(scrollArea, tr("Aliases"));
    
    if (aliasesIt != detailData.end()) {
        QVariantList aliases = aliasesIt.value().toList();
        qCDebug(logUI) << "Created Aliases tab with" << aliases.size() << "aliases";
    } else {
        qCDebug(logUI) << "Created Aliases tab with 0 aliases";
    }
}

void ItemDetailTab::createTagsTab()
{
    if (!m_item) {
        qCWarning(logUI) << "ItemDetailTab::createTagsTab: m_item is null, skipping tags tab creation";
        return;
    }
    
    // 创建标签页
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *tagsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tagsWidget);
    
    // 获取标签和风格数据
    QVariantMap detailData = m_item->getDetailData();
    auto tagsIt = detailData.find("tags");
    auto genresIt = detailData.find("genres");
    
    QVariantList tags = (tagsIt != detailData.end()) ? tagsIt.value().toList() : QVariantList();
    QVariantList genres = (genresIt != detailData.end()) ? genresIt.value().toList() : QVariantList();
    
    bool hasData = !tags.empty() || !genres.empty();
    
    if (!hasData) {        QString entityTypeName = m_item->getType() == EntityType::ReleaseGroup ? tr("release group") : 
                                 m_item->getType() == EntityType::Artist ? tr("artist") :
                                 m_item->getType() == EntityType::Release ? tr("release") :
                                 m_item->getType() == EntityType::Recording ? tr("recording") :
                                 m_item->getType() == EntityType::Work ? tr("work") :
                                 m_item->getType() == EntityType::Label ? tr("label") : tr("entity");
        
        QLabel *noDataLabel = new QLabel(tr("No tags or genres found for this %1.").arg(entityTypeName));
        noDataLabel->setStyleSheet("padding: 20px; text-align: center; color: #666; font-style: italic;");
        noDataLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(noDataLabel);
    } else {        // 风格部分
        if (!genres.empty()) {
            QWidget *genresSection = createGenresSection(genres);
            layout->addWidget(genresSection);
        }        
        // 标签部分
        if (!tags.empty()) {
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
    
    // Description
    QLabel *genresDesc = new QLabel(tr("Genres are standardized musical categories:"));
    genresDesc->setStyleSheet("font-size: 12px; color: #666; margin-bottom: 10px;");
    genresDesc->setWordWrap(true);
    genresLayout->addWidget(genresDesc);
      // 创建风格网格
    QWidget *genresContainer = new QWidget();
    QGridLayout *genresGrid = new QGridLayout(genresContainer);
    genresGrid->setSpacing(8);
    
    int row = 0, col = 0;
    const int maxCols = 4;
      for (const QVariant &genreVar : genres) {
        QVariantMap genre = genreVar.toMap();
        auto nameIt = genre.find("name");
        auto countIt = genre.find("count");
        
        QString genreName = (nameIt != genre.end()) ? nameIt.value().toString() : "";
        int genreCount = (countIt != genre.end()) ? countIt.value().toInt() : 0;
        
        if (!genreName.isEmpty()) {
            QWidget *genreItem = new QWidget();
            QHBoxLayout *genreLayout = new QHBoxLayout(genreItem);
            genreLayout->setContentsMargins(10, 6, 10, 6);
            
            genreItem->setStyleSheet("QWidget { background-color: #28a745; color: #ffffff; border-radius: 12px; font-weight: bold; }");
              QLabel *genreLabel = new QLabel(genreItem);
            genreLabel->setText(genreName);
            genreLabel->setStyleSheet("border: none; background: transparent;");
            genreLayout->addWidget(genreLabel);
            
            if (genreCount > 0) {
                QLabel *countLabel = new QLabel(genreItem);
                countLabel->setText(QString("(%1)").arg(QString::number(genreCount)));
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
    
    // 描述
    QLabel *tagsDesc = new QLabel(tr("Tags are user-generated keywords that describe this entity:"));
    tagsDesc->setStyleSheet("font-size: 12px; color: #666; margin-bottom: 10px;");
    tagsDesc->setWordWrap(true);
    tagsLayout->addWidget(tagsDesc);
    
    // 创建标签网格
    QWidget *tagsContainer = new QWidget();
    QGridLayout *tagsGrid = new QGridLayout(tagsContainer);
    tagsGrid->setSpacing(8);      // 按投票数排序标签
    QVariantList sortedTags = tags;
    std::sort(sortedTags.begin(), sortedTags.end(), [](const QVariant &a, const QVariant &b) {
        QVariantMap tagA = a.toMap();
        QVariantMap tagB = b.toMap();
        auto countAIt = tagA.find("count");
        auto countBIt = tagB.find("count");
        int countA = (countAIt != tagA.end()) ? countAIt.value().toInt() : 0;
        int countB = (countBIt != tagB.end()) ? countBIt.value().toInt() : 0;
        return countA > countB;
    });
    
    int row = 0, col = 0;
    const int maxCols = 3;
    
    for (const QVariant &tagVar : sortedTags) {
        QVariantMap tag = tagVar.toMap();
        auto nameIt = tag.find("name");
        auto countIt = tag.find("count");
        
        QString tagName = (nameIt != tag.end()) ? nameIt.value().toString() : "";
        int tagCount = (countIt != tag.end()) ? countIt.value().toInt() : 0;
        
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
            
            QLabel *tagLabel = new QLabel(tagItem);
            tagLabel->setText(tagName);
            tagLabel->setStyleSheet("border: none; background: transparent;");
            tagLayout->addWidget(tagLabel);
            
            if (tagCount > 0) {
                QLabel *countLabel = new QLabel(tagItem);
                countLabel->setText(QString("(%1)").arg(QString::number(tagCount)));
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
    if (!m_item) {
        qCWarning(logUI) << "ItemDetailTab::createRelationshipsTab: m_item is null, skipping relationships tab creation";
        return;
    }
    
    // 创建关系标签页 - 简单的单列布局
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(15);
      // 获取关系数据
    QVariantMap detailData = m_item->getDetailData();
    auto relationsIt = detailData.find("relationships");
    QVariantList relations;
    
    if (relationsIt != detailData.end()) {
        relations = relationsIt.value().toList();
    } else {
        // 支持旧键名
        auto relationsOldIt = detailData.find("relations");
        if (relationsOldIt != detailData.end()) {
            relations = relationsOldIt.value().toList();
        }
    }
    
    if (relations.empty()) {
        QLabel *noRelationsLabel = new QLabel(contentWidget);
        noRelationsLabel->setText(tr("No relationships found for this %1.").arg(m_item->getTypeString().toLower()));
        noRelationsLabel->setStyleSheet("padding: 40px; text-align: center; color: #6c757d; font-style: italic; font-size: 14px;");
        noRelationsLabel->setAlignment(Qt::AlignCenter);
        contentLayout->addWidget(noRelationsLabel);
    } else {
        // 按关系类型分组和排序
        QMap<QString, QVariantList> relationsByType;
        for (const QVariant &relationVar : relations) {
            QVariantMap relation = relationVar.toMap();
            auto typeIt = relation.find("type");
            QString type = (typeIt != relation.end()) ? typeIt.value().toString() : tr("Unknown");
            relationsByType[type].append(relationVar);
        }        
        // 为每种关系类型创建部分
        for (auto it = relationsByType.begin(); it != relationsByType.end(); ++it) {
            const QString &relationType = it.key();
            const QVariantList &typeRelations = it.value();
            
            // 关系类型标题
            QLabel *typeTitle = new QLabel(contentWidget);
            typeTitle->setText(QString("<h3 style='color: #495057; margin: 20px 0 10px 0;'>%1:</h3>").arg(relationType));
            typeTitle->setWordWrap(true);
            contentLayout->addWidget(typeTitle);
            
            // 关系项目列表
            for (const QVariant &relationVar : typeRelations) {
                QVariantMap relation = relationVar.toMap();
                QWidget *relationItem = createRelationshipItem(relation);
                contentLayout->addWidget(relationItem);
            }
        }
    }
    
    // 设置滚动区域
    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // 添加到标签页
    ui->subTabWidget->addTab(scrollArea, tr("Relationships"));
    
    qCDebug(logUI) << "Created Relationships tab with" << relations.size() << "relationships";
}

QWidget* ItemDetailTab::createRelationshipItem(const QVariantMap &relation)
{
    QWidget *item = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(item);
    layout->setContentsMargins(10, 8, 10, 8);      // 目标实体信息
    QVariantMap target;
    QString targetName;
    QString targetId;
    
    // 根据关系类型获取目标实体
    auto artistIt = relation.find("artist");
    if (artistIt != relation.end()) {
        target = artistIt.value().toMap();
        auto nameIt = target.find("name");
        auto idIt = target.find("id");
        targetName = (nameIt != target.end()) ? nameIt.value().toString() : "";
        targetId = (idIt != target.end()) ? idIt.value().toString() : "";
    } else {
        auto releaseIt = relation.find("release");
        if (releaseIt != relation.end()) {
            target = releaseIt.value().toMap();
            auto titleIt = target.find("title");
            auto idIt = target.find("id");
            targetName = (titleIt != target.end()) ? titleIt.value().toString() : "";
            targetId = (idIt != target.end()) ? idIt.value().toString() : "";
        } else {
            auto releaseGroupIt = relation.find("release-group");
            if (releaseGroupIt != relation.end()) {
                target = releaseGroupIt.value().toMap();
                auto titleIt = target.find("title");
                auto idIt = target.find("id");
                targetName = (titleIt != target.end()) ? titleIt.value().toString() : "";
                targetId = (idIt != target.end()) ? idIt.value().toString() : "";
            } else {
                auto recordingIt = relation.find("recording");
                if (recordingIt != relation.end()) {
                    target = recordingIt.value().toMap();
                    auto titleIt = target.find("title");
                    auto idIt = target.find("id");
                    targetName = (titleIt != target.end()) ? titleIt.value().toString() : "";
                    targetId = (idIt != target.end()) ? idIt.value().toString() : "";
                } else {
                    auto workIt = relation.find("work");
                    if (workIt != relation.end()) {
                        target = workIt.value().toMap();
                        auto titleIt = target.find("title");
                        auto idIt = target.find("id");
                        targetName = (titleIt != target.end()) ? titleIt.value().toString() : "";
                        targetId = (idIt != target.end()) ? idIt.value().toString() : "";                    } else {
                        auto urlIt = relation.find("url");
                        if (urlIt != relation.end()) {
                            target = urlIt.value().toMap();
                            auto resourceIt = target.find("resource");
                            auto idIt = target.find("id");
                            targetName = (resourceIt != target.end()) ? resourceIt.value().toString() : "";
                            targetId = (idIt != target.end()) ? idIt.value().toString() : "";
                        }
                    }
                }
            }
        }
    }
      // 创建目标名称标签
    if (!targetName.isEmpty()) {
        QLabel *nameLabel = new QLabel(item);
        nameLabel->setText(targetName);
        auto urlIt = relation.find("url");
        if (!targetId.isEmpty() && urlIt == relation.end()) {
            // 对于非URL关系，使用可点击样式
            nameLabel->setStyleSheet("color: #0066cc; font-weight: bold;");
            nameLabel->setCursor(Qt::PointingHandCursor);
            nameLabel->setToolTip(tr("Click to view details"));
        } else if (relation.contains("url")) {
            // 对于URL关系，使用链接样式
            nameLabel->setStyleSheet("color: #0066cc; text-decoration: underline;");
            nameLabel->setCursor(Qt::PointingHandCursor);
            nameLabel->setToolTip(tr("Click to open URL"));
        } else {
            nameLabel->setStyleSheet("font-weight: bold;");
        }
        layout->addWidget(nameLabel);    }
    
    // Add attribute information
    if (relation.contains("attributes")) {
        QVariantList attributes = relation.value("attributes").toList();
        if (!attributes.isEmpty()) {
            QStringList attrStrings;
            for (const QVariant &attr : attributes) {
                attrStrings.append(attr.toString());
            }
            QLabel *attrLabel = new QLabel(QString("(%1)").arg(attrStrings.join(", ")));
            attrLabel->setStyleSheet("color: #6c757d; font-style: italic; margin-left: 10px;");
            layout->addWidget(attrLabel);
        }
    }
    
    layout->addStretch();
    
    // Add date information
    QString beginDate = relation.value("begin").toString();
    QString endDate = relation.value("end").toString();
    if (!beginDate.isEmpty() || !endDate.isEmpty()) {
        QString dateStr;
        if (!beginDate.isEmpty() && !endDate.isEmpty()) {
            dateStr = QString("%1 – %2").arg(beginDate, endDate);
        } else if (!beginDate.isEmpty()) {
            dateStr = tr("from %1").arg(beginDate);
        } else {
            dateStr = tr("until %1").arg(endDate);
        }
        
        QLabel *dateLabel = new QLabel(dateStr);
        dateLabel->setStyleSheet("color: #6c757d; font-size: 12px;");
        layout->addWidget(dateLabel);
    }
    
    // Set item style
    item->setStyleSheet(
        "QWidget { background-color: #ffffff; border: 1px solid #e9ecef; border-radius: 4px; margin: 2px 0; }"
        "QWidget:hover { background-color: #f8f9fa; }"
    );
    
    return item;
}



void ItemDetailTab::populateItemInfo()
{
    if (!m_item) return;
    
    // 设置基本信息
    ui->nameLabel->setText(m_item->getName());
    ui->typeLabel->setText(m_item->getTypeString());
    
    qCDebug(logUI) << "ItemDetailTab::populateItemInfo - Item:" << m_item->getName() << "Type:" << m_item->getTypeString();    // 无需清除布局项目，因为populateEntityInformation会处理各个容器

    // 填充信息区域 - 根据图片样式排列
    populateEntityInformation();
    
    qCDebug(logUI) << "Populated item info for:" << m_item->getName() 
                   << "with entity information";
}

void ItemDetailTab::populateSubTabs()
{
    if (!m_item) return;
    qCDebug(logUI) << "ItemDetailTab::populateSubTabs - Starting for item:" << m_item->getName();    qCDebug(logUI) << "Available detail keys:" << m_item->getDetailData().keys();
    
    // 遍历所有子标签页并填充数据
    for (auto it = m_subTabWidgets.begin(); it != m_subTabWidgets.end(); ++it) {
        const QString &key = it.key();
        EntityListWidget *widget = it.value();
        qCDebug(logUI) << "Processing sub tab key:" << key;
          // 自动查找数据（优先没有后缀，然后检查_list后缀）
        QVariant subData = m_item->getDetailProperty(key);
        if (!subData.isValid()) {
            subData = m_item->getDetailProperty(key + "_list");
        }
        if (!subData.isValid()) {
            subData = m_item->getDetailProperty(key + "s"); // 支持复数形式
        }
        if (!subData.isValid()) {
            subData = m_item->getDetailProperty(key + "-list");
        }
        
        qCDebug(logUI) << "Sub data for key" << key << ":" << subData.typeName() << "canConvert:" << subData.canConvert<QVariantList>();
        
        QList<QSharedPointer<ResultItem>> resultItems;
          if (key == "artist-credits" || key == "artist-credit") {
            QVariant artistCreditData = m_item->getDetailProperty("artist-credits");
            if (!artistCreditData.isValid()) {
                artistCreditData = m_item->getDetailProperty("artist-credit");
            }
            if (artistCreditData.canConvert<QVariantList>()) {
                QVariantList artistList = artistCreditData.toList();                for (const QVariant &artistData : artistList) {
                    if (artistData.canConvert<QVariantMap>()) {
                        QVariantMap artistMap = artistData.toMap();
                        QVariantMap artistInfo = artistMap.value("artist").toMap();
                        QString id = artistInfo.value("id").toString();
                        QString name = artistInfo.value("name").toString();
                        if (!id.isEmpty() && !name.isEmpty()) {
                            QSharedPointer<ResultItem> resultItem = QSharedPointer<ResultItem>::create(id, name, EntityType::Artist);
                            for (auto mapIt = artistInfo.begin(); mapIt != artistInfo.end(); ++mapIt) {
                                resultItem->setDetailProperty(mapIt.key(), mapIt.value());
                            }
                            resultItems << resultItem;
                        }
                    }
                }
            }
        } else if (key == "recordings" && m_item->getType() == EntityType::Release) {            // 特殊处理Release中的录音 - 从media->tracks->recording中提取
            QVariant mediaData = m_item->getDetailProperty("media");
            if (mediaData.canConvert<QVariantList>()) {
                QVariantList mediaList = mediaData.toList();
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
                                        for (auto mapIt = recordingMap.begin(); mapIt != recordingMap.end(); ++mapIt) {
                                            resultItem->setDetailProperty(mapIt.key(), mapIt.value());
                                        }
                                        // Add track information
                                        resultItem->setDetailProperty("track_number", trackMap.value("number"));
                                        resultItem->setDetailProperty("track_position", trackMap.value("position"));
                                        resultItems << resultItem;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else if (subData.canConvert<QVariantList>()) {
            QVariantList itemList = subData.toList();
            for (const QVariant &itemData : itemList) {                if (itemData.canConvert<QVariantMap>()) {
                    QVariantMap itemMap = itemData.toMap();
                    EntityType entityType = EntityType::Unknown;
                    if (key == "releases") entityType = EntityType::Release;
                    else if (key == "release-groups") entityType = EntityType::ReleaseGroup;
                    else if (key == "recordings") entityType = EntityType::Recording;
                    else if (key == "works") entityType = EntityType::Work;
                    else if (key == "artists") entityType = EntityType::Artist;
                    else if (key == "aliases") entityType = EntityType::Unknown;
                    else if (key == "relationships") entityType = EntityType::Unknown;
                    
                    QString id = itemMap.value("id").toString();
                    QString name = itemMap.value("title", itemMap.value("name")).toString();
                    if (!id.isEmpty() && !name.isEmpty()) {
                        QSharedPointer<ResultItem> resultItem = QSharedPointer<ResultItem>::create(id, name, entityType);
                        for (auto mapIt = itemMap.begin(); mapIt != itemMap.end(); ++mapIt) {
                            resultItem->setDetailProperty(mapIt.key(), mapIt.value());
                        }
                        resultItems << resultItem;
                    }
                }
            }
        } else {
            qCDebug(logUI) << "No valid data for sub tab:" << key;
        }
        
        widget->setItems(resultItems);
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
    // Forward signal to MainWindow for handling
    emit openInBrowser(itemId, type);
    qCDebug(logUI) << "Forwarded openInBrowser signal for:" << itemId;
}

void ItemDetailTab::onCopyId(const QString &itemId)
{
    // Forward signal to MainWindow for handling
    emit copyId(itemId);
    qCDebug(logUI) << "Forwarded copyId signal for:" << itemId;
}



void ItemDetailTab::addInfoItemIfExists(QVBoxLayout *layout, const QString &label, const QVariant &value)
{
    QString valueStr;
    if (value.metaType().id() == QMetaType::QVariantMap) {
        // 处理复杂对象
        QVariantMap map = value.toMap();
        valueStr = formatComplexValue(map);
    } else {
        valueStr = value.toString();
    }
    
    if (!valueStr.isEmpty()) {
        QWidget *item = WidgetHelpers::createInfoItem(label, valueStr);
        layout->addWidget(item);
    }
}

void ItemDetailTab::addTableRow(QGridLayout *layout, int row, const QString &label, const QString &value)
{
    if (value.isEmpty()) return;
    
    QLabel *labelWidget = new QLabel(label);
    labelWidget->setStyleSheet("background: transparent; border: none; font-weight: normal; color: #495057; text-align: right;");
    labelWidget->setAlignment(Qt::AlignRight | Qt::AlignTop);
    
    QLabel *valueWidget = new QLabel(value);
    valueWidget->setStyleSheet("background: transparent; border: none; color: #212529;");
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    
    layout->addWidget(labelWidget, row, 0);
    layout->addWidget(valueWidget, row, 1);
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
    Q_UNUSED(parent)
    
    // 使用通用辅助函数添加字段
    addInfoItemIfExists(layout, tr("Type:"), detailData.value("type"));
    addInfoItemIfExists(layout, tr("Area:"), detailData.value("area"));
    addInfoItemIfExists(layout, tr("Gender:"), detailData.value("gender"));
    addInfoItemIfExists(layout, tr("Country:"), detailData.value("country"));
    
    // 生命周期需要特殊处理
    QVariantMap lifeSpan = detailData.value("life-span", QVariantMap()).toMap();
    if (!lifeSpan.isEmpty()) {
        QString begin = lifeSpan.value("begin", "").toString();
        QString end = lifeSpan.value("end", "").toString();
        QString lifeSpanText = end.isEmpty() ? (begin.isEmpty() ? tr("Unknown") : begin + " - ") 
                              : begin + " - " + end;
        addInfoItemIfExists(layout, tr("Life span:"), lifeSpanText);
    }
}

void ItemDetailTab::populateReleaseOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // 使用通用辅助函数添加字段
    addInfoItemIfExists(layout, tr("Status:"), detailData.value("status"));
    addInfoItemIfExists(layout, tr("Release date:"), detailData.value("date"));
    addInfoItemIfExists(layout, tr("Country:"), detailData.value("country"));
    addInfoItemIfExists(layout, tr("Barcode:"), detailData.value("barcode"));
    addInfoItemIfExists(layout, tr("Packaging:"), detailData.value("packaging"));
    
    // 音轨数量需要特殊处理
    int trackCount = detailData.value("track-count", 0).toInt();
    if (trackCount > 0) {
        addInfoItemIfExists(layout, tr("Track count:"), QString::number(trackCount));
    }
}

void ItemDetailTab::populateRecordingOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // 持续时间需要特殊格式化
    int length = detailData.value("length", 0).toInt();
    if (length > 0) {
        QString duration = WidgetHelpers::formatDuration(length / 1000); // 转换为秒
        addInfoItemIfExists(layout, tr("Duration:"), duration);
    }
    
    // 使用通用辅助函数添加其他字段
    addInfoItemIfExists(layout, tr("Disambiguation:"), detailData.value("disambiguation"));
}

void ItemDetailTab::populateReleaseGroupOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // 使用通用辅助函数添加字段
    addInfoItemIfExists(layout, tr("Primary type:"), detailData.value("primary-type"));
    addInfoItemIfExists(layout, tr("First release date:"), detailData.value("first-release-date"));
    addInfoItemIfExists(layout, tr("Disambiguation:"), detailData.value("disambiguation"));
    
    // 次要类型需要特殊处理
    QVariantList secondaryTypes = detailData.value("secondary-types", QVariantList()).toList();
    if (!secondaryTypes.isEmpty()) {
        QStringList typeNames;
        for (const QVariant &type : secondaryTypes) {
            typeNames << type.toString();
        }
        addInfoItemIfExists(layout, tr("Secondary types:"), typeNames.join(", "));
    }
}

void ItemDetailTab::populateLabelOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // 使用通用辅助函数添加字段
    addInfoItemIfExists(layout, tr("Type:"), detailData.value("type"));
    addInfoItemIfExists(layout, tr("Area:"), detailData.value("area"));
    
    // 厂牌代码需要特殊格式化
    QString labelCode = detailData.value("label-code", "").toString();
    if (!labelCode.isEmpty()) {
        addInfoItemIfExists(layout, tr("Label code:"), QString("LC-%1").arg(labelCode));
    }
}

void ItemDetailTab::populateWorkOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // 使用通用辅助函数添加字段
    addInfoItemIfExists(layout, tr("Type:"), detailData.value("type"));
    addInfoItemIfExists(layout, tr("Disambiguation:"), detailData.value("disambiguation"));
    
    // 语言列表需要特殊处理
    QVariantList languages = detailData.value("languages", QVariantList()).toList();
    if (!languages.isEmpty()) {
        QStringList languageNames;
        for (const QVariant &language : languages) {
            languageNames << language.toString();
        }
        addInfoItemIfExists(layout, tr("Languages:"), languageNames.join(", "));
    }
}

QWidget* ItemDetailTab::createArtistCreditWidget(const QVariantList &artistCredits)
{
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
      for (int i = 0; i < artistCredits.size(); ++i) {
        QVariantMap credit = artistCredits[i].toMap();
        QString artistName = credit.value("name", "").toString();
        QString joinPhrase = credit.value("joinphrase", "").toString();
        
        QLabel *artistLabel = new QLabel(artistName);
        artistLabel->setStyleSheet("color: #0066cc; text-decoration: underline;");
        layout->addWidget(artistLabel);
        
        if (!joinPhrase.isEmpty() && i < artistCredits.size() - 1) {
            QLabel *joinLabel = new QLabel(joinPhrase);
            layout->addWidget(joinLabel);
        }
    }
    
    return widget;
}

QWidget* ItemDetailTab::createGenresWidget(const QVariantList &genres)
{
    QWidget *genreWidget = new QWidget();
    QHBoxLayout *genreLayout = new QHBoxLayout(genreWidget);
    genreLayout->setContentsMargins(0, 0, 0, 0);
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

void ItemDetailTab::populateEntityInformation()
{
    if (!m_item) {
        qCWarning(logUI) << "ItemDetailTab::populateEntityInformation: m_item is null, skipping entity information population";
        return;
    }
      // 检查UI组件是否有效
    if (!ui || !ui->entityInfoTitle || !ui->entityInfoContainer) {
        qCCritical(logUI) << "ItemDetailTab::populateEntityInformation: UI components are not available";
        return;
    }
    
    QVariantMap detailData = m_item->getDetailData();
    EntityType itemType = m_item->getType();
    
    // 1. Entity Information section (表格样式)
    QString sectionTitle;
    switch (itemType) {
        case EntityType::Artist:
            sectionTitle = tr("Artist Information");
            break;
        case EntityType::Release:
            sectionTitle = tr("Release Information");
            break;
        case EntityType::Recording:
            sectionTitle = tr("Recording Information");
            break;
        case EntityType::ReleaseGroup:
            sectionTitle = tr("Release Group Information");
            break;
        case EntityType::Work:
            sectionTitle = tr("Work Information");
            break;
        case EntityType::Label:
            sectionTitle = tr("Label Information");
            break;
        default:
            sectionTitle = tr("Information");
            break;
    }
    
    // 设置实体信息标题
    ui->entityInfoTitle->setText(QString("<b>%1</b>").arg(sectionTitle));
      // 清除现有内容
    QLayout *entityLayout = ui->entityInfoContainer->layout();
    if (entityLayout) {
        QLayoutItem *child;
        while ((child = entityLayout->takeAt(0)) != nullptr) {
            if (child->widget()) {
                child->widget()->deleteLater();
            }
            delete child;
        }
    }
    
    // 创建信息表格
    QWidget *tableWidget = new QWidget();
    QGridLayout *tableLayout = new QGridLayout(tableWidget);
    tableLayout->setContentsMargins(0, 5, 0, 0);
    tableLayout->setSpacing(3);
    tableLayout->setColumnStretch(1, 1); // 让第二列可以伸缩
    tableWidget->setStyleSheet("background: transparent; border: none;");
    
    int row = 0;
      // 根据实体类型添加相应字段
    switch (itemType) {        case EntityType::Artist:
            {
                if (detailData.contains("type")) {
                    QString type = detailData.value("type").toString();
                    addTableRow(tableLayout, row++, tr("Type:"), type);
                }
                
                QString area;
                if (detailData.contains("area")) {
                    QVariantMap areaMap = detailData.value("area").toMap();
                    if (areaMap.contains("name")) {
                        area = areaMap.value("name").toString();
                    }
                }
                if (area.isEmpty() && detailData.contains("country")) {
                    area = detailData.value("country").toString();
                }
                if (!area.isEmpty()) {
                    addTableRow(tableLayout, row++, tr("Area:"), area);
                }
            }
            break;
            
        case EntityType::Release:
            {
                if (detailData.contains("status")) {
                    QString status = detailData["status"].toString();
                    addTableRow(tableLayout, row++, tr("Status:"), status);
                }
                
                if (detailData.contains("date")) {
                    QString date = detailData["date"].toString();
                    addTableRow(tableLayout, row++, tr("Date:"), date);
                }
                
                if (detailData.contains("country")) {
                    QString country = detailData["country"].toString();
                    addTableRow(tableLayout, row++, tr("Country:"), country);
                }
            }
            break;
              case EntityType::Recording:
            {
                if (detailData.contains("length")) {
                    int length = detailData["length"].toInt();
                    if (length > 0) {
                        QString duration = WidgetHelpers::formatDuration(length / 1000);
                        addTableRow(tableLayout, row++, tr("Duration:"), duration);
                    }
                }
            }
            break;
            
        case EntityType::ReleaseGroup:
            {
                if (detailData.contains("primary-type")) {
                    QString primaryType = detailData["primary-type"].toString();
                    addTableRow(tableLayout, row++, tr("Type:"), primaryType);
                }
            }
            break;
            
        case EntityType::Work:
            {
                if (detailData.contains("type")) {
                    QString type = detailData["type"].toString();
                    addTableRow(tableLayout, row++, tr("Type:"), type);
                }
            }
            break;
            
        case EntityType::Label:
            {
                if (detailData.contains("type")) {
                    QString type = detailData["type"].toString();
                    addTableRow(tableLayout, row++, tr("Type:"), type);
                }
                
                if (detailData.contains("label-code")) {
                    QString labelCode = detailData["label-code"].toString();
                    if (!labelCode.isEmpty()) {
                        addTableRow(tableLayout, row++, tr("Label Code:"), QString("LC-%1").arg(labelCode));
                    }
                }
            }
            break;
            
        default:            // 对于其他实体类型，不显示特定字段
            break;
    }      // 将表格添加到实体信息容器中
    tableWidget->setStyleSheet("background: transparent; border: none; margin-bottom: 15px;");
    if (entityLayout) {
        entityLayout->addWidget(tableWidget);
    }// 2. Tags section
    if (detailData.contains("tags")) {
        QVariantList tags = detailData["tags"].toList();
        if (!tags.empty()) {
            // 显示Tags标题和容器
            ui->tagsTitle->setVisible(true);
            ui->tagsContainer->setVisible(true);
              // 清除现有Tags内容
            QLayout *tagsLayout = ui->tagsContainer->layout();
            if (tagsLayout) {
                QLayoutItem *child;
                while ((child = tagsLayout->takeAt(0)) != nullptr) {
                    if (child->widget()) {
                        child->widget()->deleteLater();
                    }
                    delete child;
                }
            }
            
            // 创建标签文本，每行显示多个标签，用逗号分隔
            QStringList tagNames;
            for (const QVariant &tag : tags) {
                QVariantMap tagMap = tag.toMap();
                if (tagMap.contains("name")) {
                    QString tagName = tagMap["name"].toString();
                    if (!tagName.isEmpty()) {
                        tagNames.append(tagName);
                    }
                }
            }
              if (!tagNames.isEmpty()) {
                QString tagsText = tagNames.join(", ");
                QLabel *tagsLabel = new QLabel(tagsText);
                tagsLabel->setWordWrap(true);
                tagsLabel->setStyleSheet("color: #6c757d; margin-bottom: 15px;");
                if (tagsLayout) {
                    tagsLayout->addWidget(tagsLabel);
                }
            }
        } else {
            // 隐藏Tags部分
            ui->tagsTitle->setVisible(false);
            ui->tagsContainer->setVisible(false);
        }
    } else {
        // 隐藏Tags部分
        ui->tagsTitle->setVisible(false);
        ui->tagsContainer->setVisible(false);
    }// 3. External Links section
    if (detailData.contains("relationships")) {
        QVariantList relationships = detailData["relationships"].toList();
        QMap<QString, QList<QPair<QString, QString>>> groupedLinks; // type -> list of (url, displayName)
        
        // 按类型分组收集外部链接
        for (const QVariant &rel : relationships) {
            QVariantMap relMap = rel.toMap();
            if (relMap.contains("url")) {
                QVariantMap urlMap = relMap["url"].toMap();
                QString resource = urlMap["resource"].toString();
                QString relationType = relMap["type"].toString();
                
                if (!resource.isEmpty() && !relationType.isEmpty()) {
                    QString displayName = getPlatformDisplayName(resource, relationType);
                    groupedLinks[relationType].append(qMakePair(resource, displayName));
                }
            }
        }
          if (!groupedLinks.isEmpty()) {
            // 显示外部链接标题
            ui->externalLinksTitle->setVisible(true);
              // 清除现有外部链接内容
            QLayout *externalLinksLayout = ui->externalLinksContainer->layout();
            if (externalLinksLayout) {
                QLayoutItem *child;
                while ((child = externalLinksLayout->takeAt(0)) != nullptr) {
                    if (child->widget()) {
                        child->widget()->deleteLater();
                    }
                    delete child;
                }
            }
            
            // 为每个类型创建可折叠的链接组
            for (auto it = groupedLinks.begin(); it != groupedLinks.end(); ++it) {
                QString linkType = it.key();
                QList<QPair<QString, QString>> links = it.value();
                
                if (links.isEmpty()) continue;
                
                // 创建组容器
                QWidget *linkGroupWidget = new QWidget();
                QVBoxLayout *groupLayout = new QVBoxLayout(linkGroupWidget);
                groupLayout->setContentsMargins(0, 0, 0, 5);
                groupLayout->setSpacing(2);
                
                // 第一个链接（总是显示）
                QPair<QString, QString> firstLink = links.first();
                QWidget *firstLinkWidget = new QWidget();
                QHBoxLayout *firstLinkLayout = new QHBoxLayout(firstLinkWidget);
                firstLinkLayout->setContentsMargins(0, 0, 0, 0);
                firstLinkLayout->setSpacing(5);
                  QLabel *firstLinkLabel = new QLabel(QString("<a href=\"%1\" style=\"color: #007bff; text-decoration: none;\">%2</a>")
                                                   .arg(firstLink.first, firstLink.second));
                firstLinkLabel->setOpenExternalLinks(true);
                firstLinkLabel->setTextFormat(Qt::RichText);
                firstLinkLabel->setStyleSheet("margin-bottom: 3px;");
                firstLinkLayout->addWidget(firstLinkLabel);
                
                // 如果有多个链接，添加展开按钮
                if (links.size() > 1) {
                    QPushButton *expandButton = new QPushButton(QString("▶ (+%1)").arg(links.size() - 1));
                    expandButton->setFixedSize(60, 20);
                    expandButton->setStyleSheet(
                        "QPushButton {"
                        "    background: #f8f9fa;"
                        "    border: 1px solid #dee2e6;"
                        "    border-radius: 3px;"
                        "    color: #6c757d;"
                        "    font-size: 10px;"
                        "    padding: 2px 4px;"
                        "}"
                        "QPushButton:hover {"
                        "    background: #e9ecef;"
                        "    color: #495057;"
                        "}"
                    );                    // 创建隐藏的附加链接容器
                    QWidget *additionalLinksWidget = new QWidget();
                    QVBoxLayout *additionalLayout = new QVBoxLayout(additionalLinksWidget);
                    additionalLayout->setContentsMargins(0, 0, 0, 0); // 完全无边距
                    additionalLayout->setSpacing(2);
                    
                    for (int i = 1; i < links.size(); ++i) {
                        QPair<QString, QString> link = links.at(i);
                        
                        // 为每个附加链接创建与第一个链接相同的布局结构
                        QWidget *linkWidget = new QWidget();
                        QHBoxLayout *linkLayout = new QHBoxLayout(linkWidget);
                        linkLayout->setContentsMargins(0, 0, 0, 0);
                        linkLayout->setSpacing(5);
                        
                        QLabel *linkLabel = new QLabel(QString("<a href=\"%1\" style=\"color: #007bff; text-decoration: none;\">%2</a>")
                                                      .arg(link.first, link.second));
                        linkLabel->setOpenExternalLinks(true);
                        linkLabel->setTextFormat(Qt::RichText);
                        linkLabel->setStyleSheet("margin-bottom: 3px;");
                        linkLayout->addWidget(linkLabel);
                        linkLayout->addStretch(); // 添加弹性空间，使布局一致
                        
                        additionalLayout->addWidget(linkWidget);
                    }
                    
                    additionalLinksWidget->setVisible(false);
                      // 连接展开按钮信号
                    connect(expandButton, &QPushButton::clicked, [expandButton, additionalLinksWidget, links]() {
                        bool isExpanded = additionalLinksWidget->isVisible();
                        additionalLinksWidget->setVisible(!isExpanded);
                        if (isExpanded) {
                            expandButton->setText(QString("▶ (+%1)").arg(links.size() - 1));
                        } else {
                            expandButton->setText("▼ Collapse");
                        }
                    });
                    
                    firstLinkLayout->addWidget(expandButton);
                    firstLinkLayout->addStretch();
                    firstLinkLayout->addStretch();
                    
                    groupLayout->addWidget(firstLinkWidget);
                    groupLayout->addWidget(additionalLinksWidget);
                } else {
                    firstLinkLayout->addStretch();
                    groupLayout->addWidget(firstLinkWidget);
                }
                
                if (externalLinksLayout) {
                    externalLinksLayout->addWidget(linkGroupWidget);
                }            }        } else {
            // 隐藏外部链接部分
            ui->externalLinksTitle->setVisible(false);
        }
    } else {
        // 隐藏外部链接部分
        ui->externalLinksTitle->setVisible(false);
    }
}

QString ItemDetailTab::getPlatformDisplayName(const QString &url, const QString &relationType)
{
    QString lowerUrl = url.toLower();
    QString lowerType = relationType.toLower();
    
    // 根据URL域名识别具体平台，添加emoji图标
    if (lowerUrl.contains("spotify.com")) {
        return "🎵 Spotify";
    }
    else if (lowerUrl.contains("apple.com") || lowerUrl.contains("itunes.apple.com") || lowerUrl.contains("music.apple.com")) {
        return "🍎 Apple Music";
    }
    else if (lowerUrl.contains("deezer.com")) {
        return "🎶 Deezer";
    }
    else if (lowerUrl.contains("music.163.com") || lowerUrl.contains("netease")) {
        return "🎧 网易云音乐";
    }
    else if (lowerUrl.contains("youtube.com") || lowerUrl.contains("youtu.be")) {
        return "📺 YouTube";
    }
    else if (lowerUrl.contains("twitter.com") || lowerUrl.contains("x.com")) {
        return "🐦 X (Twitter)";
    }
    else if (lowerUrl.contains("tiktok.com")) {
        return "🎵 TikTok";
    }
    else if (lowerUrl.contains("bilibili.com") || lowerUrl.contains("space.bilibili.com")) {
        return "📱 Bilibili";
    }
    else if (lowerUrl.contains("wikidata.org")) {
        return "📖 Wikidata";
    }
    else if (lowerType.contains("homepage") || lowerType.contains("official")) {
        return "🏠 Official homepage";
    }
    else if (lowerType.contains("social")) {
        // 尝试从URL中提取更具体的平台名
        if (lowerUrl.contains("facebook.com")) return "📘 Facebook";
        if (lowerUrl.contains("instagram.com")) return "📷 Instagram";
        return "🌐 Social networking";
    }
    else if (lowerType.contains("stream")) {
        if (lowerType.contains("free")) {
            return "🎵 Stream for free";
        }
        return "🎬 Streaming page";
    }
    
    // 如果无法识别，返回原始类型名
    return relationType;
}

void ItemDetailTab::createReviewsTab()
{
    if (!m_item) {
        qDebug() << "ItemDetailTab::createReviewsTab: m_item is null, skipping reviews tab creation";
        return;
    }
    
    // 创建评价标签页
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *reviewsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(reviewsWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
      // 获取评价数据
    QVariantMap detailData = m_item->getDetailData();
    
    bool hasRating = detailData.contains("rating") && !detailData.value("rating").toMap().isEmpty();
    bool hasReviews = detailData.contains("reviews") && !detailData.value("reviews").toList().isEmpty();
    
    if (!hasRating && !hasReviews) {
        QLabel *noDataLabel = new QLabel(tr("No ratings or reviews found for this release group."));
        noDataLabel->setStyleSheet("padding: 40px; text-align: center; color: #6c757d; font-style: italic; font-size: 14px;");
        noDataLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(noDataLabel);
    } else {        // 评分部分
        if (hasRating) {
            QWidget *ratingSection = createRatingSection(detailData.value("rating").toMap());
            layout->addWidget(ratingSection);
        }
        
        // 评论部分
        if (hasReviews) {
            QWidget *reviewsSection = createReviewsSection(detailData.value("reviews").toList());
            layout->addWidget(reviewsSection);
        }
    }
    
    layout->addStretch();
    
    reviewsWidget->setLayout(layout);
    scrollArea->setWidget(reviewsWidget);
    scrollArea->setWidgetResizable(true);
    
    ui->subTabWidget->addTab(scrollArea, tr("Reviews & Ratings"));
    
    qDebug() << "Created Reviews & Ratings tab";
}

QWidget* ItemDetailTab::createRatingSection(const QVariantMap &rating)
{
    QWidget *ratingSection = new QWidget();
    QVBoxLayout *ratingLayout = new QVBoxLayout(ratingSection);
    ratingSection->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 15px; margin: 5px; }");
    
    // 标题
    QLabel *ratingTitle = new QLabel(tr("<h3>Community Rating</h3>"));
    ratingTitle->setAlignment(Qt::AlignLeft);
    ratingLayout->addWidget(ratingTitle);
    
    // 评分信息
    QWidget *ratingInfo = new QWidget();
    QHBoxLayout *ratingInfoLayout = new QHBoxLayout(ratingInfo);
    ratingInfoLayout->setContentsMargins(0, 10, 0, 10);
      // 评分值
    if (rating.contains("value")) {
        double ratingValue = rating.value("value").toDouble();
        
        // 星级显示
        QLabel *starsLabel = new QLabel();
        QString starsText = generateStarsDisplay(ratingValue);
        starsLabel->setText(starsText);
        starsLabel->setStyleSheet("font-size: 20px; color: #ffc107;");
        ratingInfoLayout->addWidget(starsLabel);
        
        // 数值显示
        QLabel *valueLabel = new QLabel(QString("<b>%1/5</b>").arg(QString::number(ratingValue, 'f', 1)));
        valueLabel->setStyleSheet("font-size: 18px; color: #495057; margin-left: 10px;");
        ratingInfoLayout->addWidget(valueLabel);
    }
    
    ratingInfoLayout->addStretch();
      // 投票数
    if (rating.contains("votes-count")) {
        int votesCount = rating.value("votes-count").toInt();
        QLabel *votesLabel = new QLabel(tr("(%1 votes)").arg(QString::number(votesCount)));
        votesLabel->setStyleSheet("color: #6c757d; font-size: 14px;");
        ratingInfoLayout->addWidget(votesLabel);
    }
    
    ratingLayout->addWidget(ratingInfo);
    return ratingSection;
}

QWidget* ItemDetailTab::createReviewsSection(const QVariantList &reviews)
{
    QWidget *reviewsSection = new QWidget();
    QVBoxLayout *reviewsLayout = new QVBoxLayout(reviewsSection);
    reviewsSection->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 15px; margin: 5px; }");
    
    // 标题
    QLabel *reviewsTitle = new QLabel(tr("<h3>User Reviews</h3>"));
    reviewsTitle->setAlignment(Qt::AlignLeft);
    reviewsLayout->addWidget(reviewsTitle);
    
    // 评论列表
    for (const QVariant &reviewVar : reviews) {
        QVariantMap review = reviewVar.toMap();
        QWidget *reviewItem = createReviewItem(review);
        reviewsLayout->addWidget(reviewItem);
    }
    
    return reviewsSection;
}

QWidget* ItemDetailTab::createReviewItem(const QVariantMap &review)
{
    QWidget *reviewItem = new QWidget();
    QVBoxLayout *reviewLayout = new QVBoxLayout(reviewItem);
    reviewItem->setStyleSheet("QWidget { background-color: #ffffff; border: 1px solid #e9ecef; border-radius: 4px; padding: 12px; margin: 5px 0; }");
    
    // 评论标题行（作者、评分、日期）
    QWidget *headerWidget = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 5);
      // 作者
    if (review.contains("author")) {
        QString author = review.value("author").toString();
        QLabel *authorLabel = new QLabel(QString("<b>%1</b>").arg(author));
        authorLabel->setStyleSheet("color: #495057; font-size: 14px;");
        headerLayout->addWidget(authorLabel);
    }
    
    headerLayout->addStretch();
      // 评分
    if (review.contains("rating")) {
        double rating = review.value("rating").toDouble();
        QString starsText = generateStarsDisplay(rating);
        QLabel *ratingLabel = new QLabel(starsText);
        ratingLabel->setStyleSheet("color: #ffc107; font-size: 14px;");
        headerLayout->addWidget(ratingLabel);
    }
      // 日期
    if (review.contains("date")) {
        QString date = review.value("date").toString();
        QLabel *dateLabel = new QLabel(date);
        dateLabel->setStyleSheet("color: #6c757d; font-size: 12px; margin-left: 10px;");
        headerLayout->addWidget(dateLabel);
    }
    
    reviewLayout->addWidget(headerWidget);
      // 评论内容
    if (review.contains("content")) {
        QString content = review.value("content").toString();
        QLabel *contentLabel = new QLabel(content);
        contentLabel->setWordWrap(true);
        contentLabel->setStyleSheet("color: #495057; font-size: 13px; line-height: 1.4; margin-top: 5px;");
        reviewLayout->addWidget(contentLabel);
    }
    
    return reviewItem;
}

QString ItemDetailTab::generateStarsDisplay(double rating)
{
    QString stars;
    int fullStars = static_cast<int>(rating);
    bool hasHalfStar = (rating - fullStars) >= 0.5;
    
    // 添加满星
    for (int i = 0; i < fullStars; i++) {
        stars += "★";
    }
    
    // 添加半星
    if (hasHalfStar) {
        stars += "☆";
    }
    
    // 添加空星
    int emptyStars = 5 - fullStars - (hasHalfStar ? 1 : 0);
    for (int i = 0; i < emptyStars; i++) {
        stars += "☆";
    }
    
    return stars;
}


