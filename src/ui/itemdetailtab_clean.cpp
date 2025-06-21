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
    // Connect signals
    connect(ui->subTabWidget, &QTabWidget::currentChanged,
            this, &ItemDetailTab::onSubTabChanged);
}

void ItemDetailTab::setupSubTabs()
{
    // Create sub-tabs based on entity type
    EntityType itemType = m_item->getType();
    
    switch (itemType) {        
        case EntityType::Artist:
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
            // Create generic related items tab for unknown entity types
            qCDebug(logUI) << "Unknown entity type for sub tabs:" << static_cast<int>(itemType);
            break;
    }
}

void ItemDetailTab::createSubTab(const QString &title, const QString &key, EntityType /*entityType*/)
{
    EntityListWidget *listWidget = new EntityListWidget();
    
    // Connect signals
    connect(listWidget, &EntityListWidget::itemDoubleClicked,
            this, &ItemDetailTab::onSubItemDoubleClicked);
    connect(listWidget, &EntityListWidget::openInBrowser,
            this, &ItemDetailTab::onOpenInBrowser);
    connect(listWidget, &EntityListWidget::copyId,
            this, &ItemDetailTab::onCopyId);
    
    // Add to tab widget
    ui->subTabWidget->addTab(listWidget, title);
    
    // Store mapping
    m_subTabWidgets[key] = listWidget;
    m_subTabKeys << key;
    
    qCDebug(logUI) << "Created sub tab:" << title << "with key:" << key;
}

void ItemDetailTab::createOverviewTab()
{
    // Create overview tab - display basic information and summary
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *overviewWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(overviewWidget);
    
    // Get item detail data
    QVariantMap detailData = m_item->getDetailData();
    EntityType itemType = m_item->getType();
    
    // Basic information section
    QWidget *basicInfoWidget = new QWidget();
    QVBoxLayout *basicInfoLayout = new QVBoxLayout(basicInfoWidget);
    basicInfoWidget->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
    
    // Title
    QLabel *basicInfoTitle = new QLabel(tr("<h3>Basic Information</h3>"));
    basicInfoTitle->setAlignment(Qt::AlignLeft);
    basicInfoLayout->addWidget(basicInfoTitle);
    
    // MBID
    QString mbid = m_item->getId();
    if (!mbid.isEmpty()) {
        QWidget *mbidItem = UiUtils::createInfoItem(tr("MBID"), mbid, overviewWidget);
        basicInfoLayout->addWidget(mbidItem);
    }
    
    // Display different fields based on entity type
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
    
    // Rating information (if available)
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
    
    // Disambiguation information
    QString disambiguation = m_item->getDisambiguation();
    if (!disambiguation.isEmpty()) {
        QWidget *disambiguationItem = UiUtils::createInfoItem(tr("Disambiguation"), disambiguation, overviewWidget);
        basicInfoLayout->addWidget(disambiguationItem);
    }
    
    layout->addWidget(basicInfoWidget);
    
    // Artist information section (for entities with artist-credit)
    QVariantList artistCredits = detailData.value("artist-credit", QVariantList()).toList();
    if (!artistCredits.isEmpty()) {
        layout->addWidget(createArtistCreditWidget(artistCredits));
    }
    
    // Genre information
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
    // Create aliases tab
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *aliasesWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(aliasesWidget);
    
    // Get aliases data
    QVariantMap detailData = m_item->getDetailData();
    QVariantList aliases = detailData.value("aliases", QVariantList()).toList();
    
    if (aliases.isEmpty()) {
        QLabel *noAliasesLabel = new QLabel(tr("No aliases found for this release group."));
        noAliasesLabel->setStyleSheet("padding: 20px; text-align: center; color: #666; font-style: italic;");
        noAliasesLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(noAliasesLabel);
    } else {
        // Create aliases list
        for (const QVariant &aliasVar : aliases) {
            QVariantMap alias = aliasVar.toMap();
            
            QWidget *aliasItem = new QWidget();
            QVBoxLayout *aliasLayout = new QVBoxLayout(aliasItem);
            aliasItem->setStyleSheet("QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; margin: 5px; }");
            
            // Alias name
            QString aliasName = alias.value("name", "").toString();
            if (!aliasName.isEmpty()) {
                QLabel *nameLabel = new QLabel(QString("<b>%1</b>").arg(aliasName));
                nameLabel->setStyleSheet("font-size: 14px; margin-bottom: 5px;");
                aliasLayout->addWidget(nameLabel);
            }
            
            // Alias type
            QString aliasType = alias.value("type", "").toString();
            if (!aliasType.isEmpty()) {
                QLabel *typeLabel = new QLabel(tr("Type: %1").arg(aliasType));
                typeLabel->setStyleSheet("color: #666; font-size: 12px;");
                aliasLayout->addWidget(typeLabel);
            }
            
            // Sort name
            QString sortName = alias.value("sort-name", "").toString();
            if (!sortName.isEmpty() && sortName != aliasName) {
                QLabel *sortLabel = new QLabel(tr("Sort name: %1").arg(sortName));
                sortLabel->setStyleSheet("color: #666; font-size: 12px;");
                aliasLayout->addWidget(sortLabel);
            }
            
            // Language and region
            QString locale = alias.value("locale", "").toString();
            if (!locale.isEmpty()) {
                QLabel *localeLabel = new QLabel(tr("Locale: %1").arg(locale));
                localeLabel->setStyleSheet("color: #666; font-size: 12px;");
                aliasLayout->addWidget(localeLabel);
            }
            
            // Whether it's a primary alias
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
    // Create tags tab
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *tagsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tagsWidget);
    
    // Get tags and genres data
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
        // Genres section
        if (!genres.isEmpty()) {
            QWidget *genresSection = createGenresSection(genres);
            layout->addWidget(genresSection);
        }
        
        // Tags section
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
    
    // Title
    QLabel *genresTitle = new QLabel(tr("<h3>Genres</h3>"));
    genresTitle->setAlignment(Qt::AlignLeft);
    genresLayout->addWidget(genresTitle);
    
    // Description
    QLabel *genresDesc = new QLabel(tr("Genres are standardized musical categories:"));
    genresDesc->setStyleSheet("font-size: 12px; color: #666; margin-bottom: 10px;");
    genresDesc->setWordWrap(true);
    genresLayout->addWidget(genresDesc);
    
    // Create genres grid
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
    
    // Title
    QLabel *tagsTitle = new QLabel(tr("<h3>User Tags</h3>"));
    tagsTitle->setAlignment(Qt::AlignLeft);
    tagsLayout->addWidget(tagsTitle);
    
    // Description
    QLabel *tagsDesc = new QLabel(tr("Tags are user-generated keywords that describe this entity:"));
    tagsDesc->setStyleSheet("font-size: 12px; color: #666; margin-bottom: 10px;");
    tagsDesc->setWordWrap(true);
    tagsLayout->addWidget(tagsDesc);
    
    // Create tags grid
    QWidget *tagsContainer = new QWidget();
    QGridLayout *tagsGrid = new QGridLayout(tagsContainer);
    tagsGrid->setSpacing(8);
    
    // Sort tags by vote count
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
            
            // Set style based on tag usage frequency
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
    // Create relationships tab main layout
    QHBoxLayout *mainLayout = new QHBoxLayout();
    
    // Left side: relationships content area
    QScrollArea *contentScrollArea = new QScrollArea();
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    
    // Right side: information sidebar
    QWidget *sidebarWidget = createRelationshipsSidebar();
    sidebarWidget->setFixedWidth(300);
    sidebarWidget->setStyleSheet(
        "QWidget { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 8px; }"
        "QLabel { padding: 5px; }"
    );
    
    // Get relationships data
    QVariantMap detailData = m_item->getDetailData();
    QVariantList relations = detailData.value("relationships", QVariantList()).toList();
    
    // Support legacy key names
    if (relations.isEmpty()) {
        relations = detailData.value("relations", QVariantList()).toList();
    }
    
    if (relations.isEmpty()) {
        QLabel *noRelationsLabel = new QLabel(tr("No relationships found for this %1.").arg(m_item->getTypeString().toLower()));
        noRelationsLabel->setStyleSheet("padding: 40px; text-align: center; color: #6c757d; font-style: italic; font-size: 14px;");
        noRelationsLabel->setAlignment(Qt::AlignCenter);
        contentLayout->addWidget(noRelationsLabel);
    } else {
        // Group and sort by relationship type
        QMap<QString, QVariantList> relationsByType;
        for (const QVariant &relationVar : relations) {
            QVariantMap relation = relationVar.toMap();
            QString type = relation.value("type", tr("Unknown")).toString();
            relationsByType[type].append(relationVar);
        }
        
        // Create section for each relationship type
        for (auto it = relationsByType.constBegin(); it != relationsByType.constEnd(); ++it) {
            const QString &relationType = it.key();
            const QVariantList &typeRelations = it.value();
            
            // Relationship type title
            QLabel *typeTitle = new QLabel(QString("<h3 style='color: #495057; margin: 20px 0 10px 0;'>%1:</h3>").arg(relationType));
            typeTitle->setWordWrap(true);
            contentLayout->addWidget(typeTitle);
            
            // Relationship items list
            for (const QVariant &relationVar : typeRelations) {
                QVariantMap relation = relationVar.toMap();
                QWidget *relationItem = createRelationshipItem(relation);
                contentLayout->addWidget(relationItem);
            }
        }
    }
    
    // Set up content area
    contentLayout->addStretch();
    contentScrollArea->setWidget(contentWidget);
    contentScrollArea->setWidgetResizable(true);
    contentScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    contentScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Set up main layout
    mainLayout->addWidget(contentScrollArea, 1);
    mainLayout->addWidget(sidebarWidget, 0);
    
    // Create relationships tab and set layout
    QWidget *relationshipsWidget = new QWidget();
    relationshipsWidget->setLayout(mainLayout);
    
    ui->subTabWidget->addTab(relationshipsWidget, tr("Relationships"));
    
    qCDebug(logUI) << "Created Relationships tab with" << relations.size() << "relationships";
}

QWidget* ItemDetailTab::createRelationshipItem(const QVariantMap &relation)
{
    QWidget *item = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(item);
    layout->setContentsMargins(10, 8, 10, 8);
    
    // Target entity information
    QVariantMap target;
    QString targetName;
    QString targetId;
    
    // Get target entity based on relationship type
    if (relation.contains("artist")) {
        target = relation.value("artist").toMap();
        targetName = target.value("name").toString();
        targetId = target.value("id").toString();
    } else if (relation.contains("release")) {
        target = relation.value("release").toMap();
        targetName = target.value("title").toString();
        targetId = target.value("id").toString();
    } else if (relation.contains("release-group")) {
        target = relation.value("release-group").toMap();
        targetName = target.value("title").toString();
        targetId = target.value("id").toString();
    } else if (relation.contains("recording")) {
        target = relation.value("recording").toMap();
        targetName = target.value("title").toString();
        targetId = target.value("id").toString();
    } else if (relation.contains("work")) {
        target = relation.value("work").toMap();
        targetName = target.value("title").toString();
        targetId = target.value("id").toString();
    } else if (relation.contains("url")) {
        target = relation.value("url").toMap();
        targetName = target.value("resource").toString();
        targetId = target.value("id").toString();
    }
    
    // Create target name label
    if (!targetName.isEmpty()) {
        QLabel *nameLabel = new QLabel(targetName);
        if (!targetId.isEmpty() && !relation.contains("url")) {
            // For non-URL relationships, use clickable style
            nameLabel->setStyleSheet("color: #0066cc; font-weight: bold;");
            nameLabel->setCursor(Qt::PointingHandCursor);
            nameLabel->setToolTip(tr("Click to view details"));
        } else if (relation.contains("url")) {
            // For URL relationships, use link style
            nameLabel->setStyleSheet("color: #0066cc; text-decoration: underline;");
            nameLabel->setCursor(Qt::PointingHandCursor);
            nameLabel->setToolTip(tr("Click to open URL"));
        } else {
            nameLabel->setStyleSheet("font-weight: bold;");
        }
        layout->addWidget(nameLabel);
    }
    
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

QWidget* ItemDetailTab::createRelationshipsSidebar()
{
    QWidget *sidebar = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    QVariantMap detailData = m_item->getDetailData();
    
    // Artist Information section
    if (m_item->getType() == EntityType::Artist) {
        QWidget *artistInfo = new QWidget();
        QVBoxLayout *artistLayout = new QVBoxLayout(artistInfo);
        
        QLabel *title = new QLabel("<h4 style='color: #495057; margin: 0;'>Artist Information</h4>");
        artistLayout->addWidget(title);
        
        // Type
        QString type = detailData.value("type").toString();
        if (!type.isEmpty()) {
            addSidebarItem(artistLayout, tr("Type:"), type);
        }
        
        // Area
        QString area = detailData.value("area").toMap().value("name").toString();
        if (area.isEmpty()) {
            area = detailData.value("country").toString();
        }
        if (!area.isEmpty()) {
            addSidebarItem(artistLayout, tr("Area:"), area);
        }
        
        layout->addWidget(artistInfo);
    }
    
    // Rating section
    if (detailData.contains("rating")) {
        QVariantMap rating = detailData.value("rating").toMap();
        QString ratingValue = rating.value("value").toString();
        if (!ratingValue.isEmpty()) {
            QWidget *ratingWidget = new QWidget();
            QVBoxLayout *ratingLayout = new QVBoxLayout(ratingWidget);
            
            QLabel *ratingTitle = new QLabel("<h4 style='color: #495057; margin: 0;'>Rating</h4>");
            ratingLayout->addWidget(ratingTitle);
            
            addSidebarItem(ratingLayout, tr("Rating:"), QString("%1/5").arg(ratingValue));
            
            layout->addWidget(ratingWidget);
        }
    }
    
    // Tags section
    if (detailData.contains("tags")) {
        QVariantList tags = detailData.value("tags").toList();
        if (!tags.isEmpty()) {
            QWidget *tagsWidget = new QWidget();
            QVBoxLayout *tagsLayout = new QVBoxLayout(tagsWidget);
            
            QLabel *tagsTitle = new QLabel("<h4 style='color: #495057; margin: 0;'>Tags</h4>");
            tagsLayout->addWidget(tagsTitle);
            
            QStringList tagNames;
            for (const QVariant &tag : tags) {
                QVariantMap tagMap = tag.toMap();
                QString tagName = tagMap.value("name").toString();
                if (!tagName.isEmpty()) {
                    tagNames.append(tagName);
                }
            }
            
            if (!tagNames.isEmpty()) {
                QString tagsText = tagNames.join(", ");
                QLabel *tagsLabel = new QLabel(tagsText);
                tagsLabel->setWordWrap(true);
                tagsLabel->setStyleSheet("padding: 5px 0; color: #6c757d;");
                tagsLayout->addWidget(tagsLabel);
            }
            
            layout->addWidget(tagsWidget);
        }
    }
    
    // Genres section
    if (detailData.contains("genres")) {
        QVariantList genres = detailData.value("genres").toList();
        if (!genres.isEmpty()) {
            QWidget *genresWidget = new QWidget();
            QVBoxLayout *genresLayout = new QVBoxLayout(genresWidget);
            
            QLabel *genresTitle = new QLabel("<h4 style='color: #495057; margin: 0;'>Genres</h4>");
            genresLayout->addWidget(genresTitle);
            
            QStringList genreNames;
            for (const QVariant &genre : genres) {
                QVariantMap genreMap = genre.toMap();
                QString genreName = genreMap.value("name").toString();
                if (!genreName.isEmpty()) {
                    genreNames.append(genreName);
                }
            }
            
            if (!genreNames.isEmpty()) {
                QString genresText = genreNames.join(", ");
                QLabel *genresLabel = new QLabel(genresText);
                genresLabel->setWordWrap(true);
                genresLabel->setStyleSheet("padding: 5px 0; color: #6c757d;");
                genresLayout->addWidget(genresLabel);
            }
            
            layout->addWidget(genresWidget);
        }
    }
    
    layout->addStretch();
    return sidebar;
}

void ItemDetailTab::addSidebarItem(QVBoxLayout *layout, const QString &label, const QString &value)
{
    QWidget *item = new QWidget();
    QHBoxLayout *itemLayout = new QHBoxLayout(item);
    itemLayout->setContentsMargins(0, 2, 0, 2);
    
    QLabel *labelWidget = new QLabel(label);
    labelWidget->setStyleSheet("font-weight: bold; color: #495057;");
    labelWidget->setMinimumWidth(60);
    
    QLabel *valueWidget = new QLabel(value);
    valueWidget->setStyleSheet("color: #6c757d;");
    valueWidget->setWordWrap(true);
    
    itemLayout->addWidget(labelWidget);
    itemLayout->addWidget(valueWidget, 1);
    
    layout->addWidget(item);
}

void ItemDetailTab::populateItemInfo()
{
    if (!m_item) return;
    
    // Set basic information
    ui->nameLabel->setText(m_item->getName());
    ui->typeLabel->setText(m_item->getTypeString());
    
    qCDebug(logUI) << "ItemDetailTab::populateItemInfo - Item:" << m_item->getName() << "Type:" << m_item->getTypeString();
    
    // Clear existing items
    clearInfoLayout(ui->infoContentLayout);
    clearInfoLayout(ui->detailsContentLayout);

    // Populate information area
    QMap<QString, QVariant> details = m_item->getDetails();
    populateInfoSection(ui->infoContentLayout, details);

    // Populate detailed information area
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
    
    // Iterate through all sub-tabs and populate data
    for (auto it = m_subTabWidgets.begin(); it != m_subTabWidgets.end(); ++it) {
        const QString &key = it.key();
        EntityListWidget *widget = it.value();
        qCDebug(logUI) << "Processing sub tab key:" << key;
        
        // Automatically find data (prefer no suffix, then check _list suffix)
        QVariant subData = m_item->getDetailProperty(key);
        if (!subData.isValid()) {
            subData = m_item->getDetailProperty(key + "_list");
        }
        if (!subData.isValid()) {
            subData = m_item->getDetailProperty(key + "s"); // Support plural
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
                QVariantList artistList = artistCreditData.toList();
                for (const QVariant &artistData : artistList) {
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
        } else if (key == "recordings" && m_item->getType() == EntityType::Release) {
            // Special handling for recordings in Release - extract from media->tracks->recording
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
            for (const QVariant &itemData : itemList) {
                if (itemData.canConvert<QVariantMap>()) {
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

void ItemDetailTab::clearInfoLayout(QVBoxLayout *layout)
{
    // Clear all items in layout
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void ItemDetailTab::populateInfoSection(QVBoxLayout *layout, const QMap<QString, QVariant> &data)
{
    // Core information field mapping - only display most important basic information
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
        
        // Only display fields defined in fieldNames and non-empty values
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
    
    // Add elastic space
    layout->addStretch();
}

void ItemDetailTab::populateDetailsSection(QVBoxLayout *layout, const QVariantMap &data)
{
    // Detailed information field mapping - only display useful fields
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
        
        // Skip sub-item list data
        if (key == "releases" || key == "release-groups" || key == "recordings" || 
            key == "works" || key == "artists" || key == "labels" || key == "places" ||
            key == "tags" || key == "rating" || key == "aliases") {
            continue;
        }
        
        // Only display useful fields defined in fieldNames
        if (!fieldNames.contains(key)) {
            continue;
        }
        
        // Handle different types of data
        if (value.canConvert<QString>() && !value.toString().isEmpty()) {
            QString displayName = fieldNames.value(key);
            QWidget *infoItem = UiUtils::createInfoItem(displayName, value.toString(), layout->parentWidget());
            layout->addWidget(infoItem);
            hasData = true;
        } else if (value.canConvert<QVariantMap>()) {
            // Handle complex objects
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
    
    // Add elastic space
    layout->addStretch();
}

QString ItemDetailTab::formatComplexValue(const QVariantMap &map)
{
    QStringList parts;
    
    // Handle common complex value fields
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
    
    // Artist type
    QString type = detailData.value("type", "").toString();
    if (!type.isEmpty()) {
        QWidget *typeItem = UiUtils::createInfoItem(tr("Type:"), type);
        layout->addWidget(typeItem);
    }
    
    // Area
    QString area = detailData.value("area", "").toString();
    if (!area.isEmpty()) {
        QWidget *areaItem = UiUtils::createInfoItem(tr("Area:"), area);
        layout->addWidget(areaItem);
    }
    
    // Life span
    QVariantMap lifeSpan = detailData.value("life-span", QVariantMap()).toMap();
    if (!lifeSpan.isEmpty()) {
        QString begin = lifeSpan.value("begin", "").toString();
        QString end = lifeSpan.value("end", "").toString();
        QString lifeSpanText = end.isEmpty() ? (begin.isEmpty() ? tr("Unknown") : begin + " - ") 
                              : begin + " - " + end;
        QWidget *lifeSpanItem = UiUtils::createInfoItem(tr("Life span:"), lifeSpanText);
        layout->addWidget(lifeSpanItem);
    }
    
    // Gender
    QString gender = detailData.value("gender", "").toString();
    if (!gender.isEmpty()) {
        QWidget *genderItem = UiUtils::createInfoItem(tr("Gender:"), gender);
        layout->addWidget(genderItem);
    }
    
    // Country
    QString country = detailData.value("country", "").toString();
    if (!country.isEmpty()) {
        QWidget *countryItem = UiUtils::createInfoItem(tr("Country:"), country);
        layout->addWidget(countryItem);
    }
}

void ItemDetailTab::populateReleaseOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // Release status
    QString status = detailData.value("status", "").toString();
    if (!status.isEmpty()) {
        QWidget *statusItem = UiUtils::createInfoItem(tr("Status:"), status);
        layout->addWidget(statusItem);
    }
    
    // Release date
    QString date = detailData.value("date", "").toString();
    if (!date.isEmpty()) {
        QWidget *dateItem = UiUtils::createInfoItem(tr("Release date:"), date);
        layout->addWidget(dateItem);
    }
    
    // Country/Region
    QString country = detailData.value("country", "").toString();
    if (!country.isEmpty()) {
        QWidget *countryItem = UiUtils::createInfoItem(tr("Country:"), country);
        layout->addWidget(countryItem);
    }
    
    // Barcode
    QString barcode = detailData.value("barcode", "").toString();
    if (!barcode.isEmpty()) {
        QWidget *barcodeItem = UiUtils::createInfoItem(tr("Barcode:"), barcode);
        layout->addWidget(barcodeItem);
    }
    
    // Packaging
    QString packaging = detailData.value("packaging", "").toString();
    if (!packaging.isEmpty()) {
        QWidget *packagingItem = UiUtils::createInfoItem(tr("Packaging:"), packaging);
        layout->addWidget(packagingItem);
    }
    
    // Track count
    int trackCount = detailData.value("track-count", 0).toInt();
    if (trackCount > 0) {
        QWidget *trackCountItem = UiUtils::createInfoItem(tr("Track count:"), QString::number(trackCount));
        layout->addWidget(trackCountItem);
    }
}

void ItemDetailTab::populateRecordingOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // Duration
    int length = detailData.value("length", 0).toInt();
    if (length > 0) {
        QString duration = UiUtils::formatDuration(length / 1000); // Convert to seconds
        QWidget *durationItem = UiUtils::createInfoItem(tr("Duration:"), duration);
        layout->addWidget(durationItem);
    }
    
    // Disambiguation
    QString disambiguation = detailData.value("disambiguation", "").toString();
    if (!disambiguation.isEmpty()) {
        QWidget *disambiguationItem = UiUtils::createInfoItem(tr("Disambiguation:"), disambiguation);
        layout->addWidget(disambiguationItem);
    }
}

void ItemDetailTab::populateReleaseGroupOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // Primary type
    QString primaryType = detailData.value("primary-type", "").toString();
    if (!primaryType.isEmpty()) {
        QWidget *primaryTypeItem = UiUtils::createInfoItem(tr("Primary type:"), primaryType);
        layout->addWidget(primaryTypeItem);
    }
    
    // Secondary types
    QVariantList secondaryTypes = detailData.value("secondary-types", QVariantList()).toList();
    if (!secondaryTypes.isEmpty()) {
        QStringList typeNames;
        for (const QVariant &type : secondaryTypes) {
            typeNames << type.toString();
        }
        QString secondaryTypesText = typeNames.join(", ");
        QWidget *secondaryTypesItem = UiUtils::createInfoItem(tr("Secondary types:"), secondaryTypesText);
        layout->addWidget(secondaryTypesItem);
    }
    
    // First release date
    QString firstReleaseDate = detailData.value("first-release-date", "").toString();
    if (!firstReleaseDate.isEmpty()) {
        QWidget *dateItem = UiUtils::createInfoItem(tr("First release date:"), firstReleaseDate);
        layout->addWidget(dateItem);
    }
    
    // Disambiguation
    QString disambiguation = detailData.value("disambiguation", "").toString();
    if (!disambiguation.isEmpty()) {
        QWidget *disambiguationItem = UiUtils::createInfoItem(tr("Disambiguation:"), disambiguation);
        layout->addWidget(disambiguationItem);
    }
}

void ItemDetailTab::populateLabelOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // Label type
    QString type = detailData.value("type", "").toString();
    if (!type.isEmpty()) {
        QWidget *typeItem = UiUtils::createInfoItem(tr("Type:"), type);
        layout->addWidget(typeItem);
    }
    
    // Label code
    QString labelCode = detailData.value("label-code", "").toString();
    if (!labelCode.isEmpty()) {
        QWidget *labelCodeItem = UiUtils::createInfoItem(tr("Label code:"), QString("LC-%1").arg(labelCode));
        layout->addWidget(labelCodeItem);
    }
    
    // Area
    QString area = detailData.value("area", "").toString();
    if (!area.isEmpty()) {
        QWidget *areaItem = UiUtils::createInfoItem(tr("Area:"), area);
        layout->addWidget(areaItem);
    }
}

void ItemDetailTab::populateWorkOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent)
{
    Q_UNUSED(parent)
    
    // Work type
    QString type = detailData.value("type", "").toString();
    if (!type.isEmpty()) {
        QWidget *typeItem = UiUtils::createInfoItem(tr("Type:"), type);
        layout->addWidget(typeItem);
    }
    
    // Languages
    QVariantList languages = detailData.value("languages", QVariantList()).toList();
    if (!languages.isEmpty()) {
        QStringList languageNames;
        for (const QVariant &language : languages) {
            languageNames << language.toString();
        }
        QString languageText = languageNames.join(", ");
        QWidget *languageItem = UiUtils::createInfoItem(tr("Languages:"), languageText);
        layout->addWidget(languageItem);
    }
    
    // Disambiguation
    QString disambiguation = detailData.value("disambiguation", "").toString();
    if (!disambiguation.isEmpty()) {
        QWidget *disambiguationItem = UiUtils::createInfoItem(tr("Disambiguation:"), disambiguation);
        layout->addWidget(disambiguationItem);
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
