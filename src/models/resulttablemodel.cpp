#include "resulttablemodel.h"
#include "../core/types.h"
#include "../api/api_utils.h"
#include <QLocale>
#include <QDate>
#include <QCoreApplication>
#include <algorithm>

ResultTableModel::ResultTableModel(QObject *parent)
    : QAbstractTableModel(parent), m_type(EntityType::Unknown) {
}

void ResultTableModel::setItems(const QList<QSharedPointer<ResultItem>> &items, EntityType type) {
    beginResetModel();
    m_items = items;
    m_type = type;
    
    // 动态检测列
    updateColumnsFromData();
    
    endResetModel();
}

void ResultTableModel::clear() {
    beginResetModel();
    m_items.clear();
    m_type = EntityType::Unknown;
    m_visibleColumns.clear();
    endResetModel();
}

void ResultTableModel::addItem(const QSharedPointer<ResultItem> &item) {
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items.append(item);
    endInsertRows();
}

int ResultTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_items.count();
}

int ResultTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_visibleColumns.count();
}

QVariant ResultTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_items.count() || index.column() >= m_visibleColumns.count()) {
        return QVariant();
    }
    
    const auto &item = m_items[index.row()];
    const QString &fieldKey = m_visibleColumns[index.column()].key;
    
    if (role == Qt::DisplayRole) {
        return getFieldValue(item, fieldKey);
    } else if (role == Qt::ToolTipRole) {
        return m_visibleColumns[index.column()].description;
    }
    
    return QVariant();
}

QVariant ResultTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section < m_visibleColumns.count()) {
            return m_visibleColumns[section].header;
        }
    }
    return QVariant();
}

EntityType ResultTableModel::currentType() const {
    return m_type;
}

QSharedPointer<ResultItem> ResultTableModel::getItem(int row) const {
    if (row >= 0 && row < m_items.count()) {
        return m_items[row];
    }
    return nullptr;
}

QList<ColumnInfo> ResultTableModel::getAvailableColumns(EntityType type) const {
    QList<ColumnInfo> columns;
    
    switch (type) {
        case EntityType::Artist:
            columns << ColumnInfo{"name", QCoreApplication::translate("ResultTableModel", "Name"), QCoreApplication::translate("ResultTableModel", "Artist name")}
                   << ColumnInfo{"sort_name", QCoreApplication::translate("ResultTableModel", "Sort Name"), QCoreApplication::translate("ResultTableModel", "Artist sort name")}
                   << ColumnInfo{"type", QCoreApplication::translate("ResultTableModel", "Type"), QCoreApplication::translate("ResultTableModel", "Artist type")}
                   << ColumnInfo{"country", QCoreApplication::translate("ResultTableModel", "Country"), QCoreApplication::translate("ResultTableModel", "Artist country")}
                   << ColumnInfo{"life_span", QCoreApplication::translate("ResultTableModel", "Life Span"), QCoreApplication::translate("ResultTableModel", "Artist life span")}
                   << ColumnInfo{"gender", QCoreApplication::translate("ResultTableModel", "Gender"), QCoreApplication::translate("ResultTableModel", "Artist gender")}
                   << ColumnInfo{"disambiguation", QCoreApplication::translate("ResultTableModel", "Disambiguation"), QCoreApplication::translate("ResultTableModel", "Artist disambiguation")}
                   << ColumnInfo{"aliases", QCoreApplication::translate("ResultTableModel", "Aliases"), QCoreApplication::translate("ResultTableModel", "Artist aliases")}
                   << ColumnInfo{"release_count", QCoreApplication::translate("ResultTableModel", "Releases"), QCoreApplication::translate("ResultTableModel", "Number of releases")}
                   << ColumnInfo{"recording_count", QCoreApplication::translate("ResultTableModel", "Recordings"), QCoreApplication::translate("ResultTableModel", "Number of recordings")}
                   << ColumnInfo{"work_count", QCoreApplication::translate("ResultTableModel", "Works"), QCoreApplication::translate("ResultTableModel", "Number of works")}
                   << ColumnInfo{"release_group_count", QCoreApplication::translate("ResultTableModel", "Release Groups"), QCoreApplication::translate("ResultTableModel", "Number of release groups")}
                   << ColumnInfo{"tags", QCoreApplication::translate("ResultTableModel", "Tags"), QCoreApplication::translate("ResultTableModel", "Artist tags")}
                   << ColumnInfo{"rating", QCoreApplication::translate("ResultTableModel", "Rating"), QCoreApplication::translate("ResultTableModel", "Artist rating")};
            break;
            
        case EntityType::Release:
            columns << ColumnInfo{"title", QCoreApplication::translate("ResultTableModel", "Title"), QCoreApplication::translate("ResultTableModel", "Release title")}
                   << ColumnInfo{"artist", QCoreApplication::translate("ResultTableModel", "Artist"), QCoreApplication::translate("ResultTableModel", "Release artist")}
                   << ColumnInfo{"date", QCoreApplication::translate("ResultTableModel", "Date"), QCoreApplication::translate("ResultTableModel", "Release date")}
                   << ColumnInfo{"country", QCoreApplication::translate("ResultTableModel", "Country"), QCoreApplication::translate("ResultTableModel", "Release country")}
                   << ColumnInfo{"status", QCoreApplication::translate("ResultTableModel", "Status"), QCoreApplication::translate("ResultTableModel", "Release status")}
                   << ColumnInfo{"packaging", QCoreApplication::translate("ResultTableModel", "Packaging"), QCoreApplication::translate("ResultTableModel", "Release packaging")}
                   << ColumnInfo{"format", QCoreApplication::translate("ResultTableModel", "Format"), QCoreApplication::translate("ResultTableModel", "Release format")}
                   << ColumnInfo{"track_count", QCoreApplication::translate("ResultTableModel", "Tracks"), QCoreApplication::translate("ResultTableModel", "Number of tracks")}
                   << ColumnInfo{"catalog_number", QCoreApplication::translate("ResultTableModel", "Catalog Number"), QCoreApplication::translate("ResultTableModel", "Release catalog number")}
                   << ColumnInfo{"barcode", QCoreApplication::translate("ResultTableModel", "Barcode"), QCoreApplication::translate("ResultTableModel", "Release barcode")}
                   << ColumnInfo{"label", QCoreApplication::translate("ResultTableModel", "Label"), QCoreApplication::translate("ResultTableModel", "Release label")}
                   << ColumnInfo{"disambiguation", QCoreApplication::translate("ResultTableModel", "Disambiguation"), QCoreApplication::translate("ResultTableModel", "Release disambiguation")}
                   << ColumnInfo{"tags", QCoreApplication::translate("ResultTableModel", "Tags"), QCoreApplication::translate("ResultTableModel", "Release tags")}
                   << ColumnInfo{"rating", QCoreApplication::translate("ResultTableModel", "Rating"), QCoreApplication::translate("ResultTableModel", "Release rating")};
            break;
            
        case EntityType::Recording:
            columns << ColumnInfo{"title", QCoreApplication::translate("ResultTableModel", "Title"), QCoreApplication::translate("ResultTableModel", "Recording title")}
                   << ColumnInfo{"artist", QCoreApplication::translate("ResultTableModel", "Artist"), QCoreApplication::translate("ResultTableModel", "Recording artist")}
                   << ColumnInfo{"length", QCoreApplication::translate("ResultTableModel", "Length"), QCoreApplication::translate("ResultTableModel", "Recording length")}
                   << ColumnInfo{"disambiguation", QCoreApplication::translate("ResultTableModel", "Disambiguation"), QCoreApplication::translate("ResultTableModel", "Recording disambiguation")}
                   << ColumnInfo{"isrcs", QCoreApplication::translate("ResultTableModel", "ISRCs"), QCoreApplication::translate("ResultTableModel", "Recording ISRCs")}
                   << ColumnInfo{"tags", QCoreApplication::translate("ResultTableModel", "Tags"), QCoreApplication::translate("ResultTableModel", "Recording tags")}
                   << ColumnInfo{"rating", QCoreApplication::translate("ResultTableModel", "Rating"), QCoreApplication::translate("ResultTableModel", "Recording rating")}
                   << ColumnInfo{"release_count", QCoreApplication::translate("ResultTableModel", "Releases"), QCoreApplication::translate("ResultTableModel", "Number of releases containing this recording")};
            break;
            
        case EntityType::ReleaseGroup:
            columns << ColumnInfo{"title", QCoreApplication::translate("ResultTableModel", "Title"), QCoreApplication::translate("ResultTableModel", "Release group title")}
                   << ColumnInfo{"artist", QCoreApplication::translate("ResultTableModel", "Artist"), QCoreApplication::translate("ResultTableModel", "Release group artist")}
                   << ColumnInfo{"type", QCoreApplication::translate("ResultTableModel", "Type"), QCoreApplication::translate("ResultTableModel", "Release group type")}
                   << ColumnInfo{"first_release_date", QCoreApplication::translate("ResultTableModel", "First Release Date"), QCoreApplication::translate("ResultTableModel", "First release date")}
                   << ColumnInfo{"disambiguation", QCoreApplication::translate("ResultTableModel", "Disambiguation"), QCoreApplication::translate("ResultTableModel", "Release group disambiguation")}
                   << ColumnInfo{"tags", QCoreApplication::translate("ResultTableModel", "Tags"), QCoreApplication::translate("ResultTableModel", "Release group tags")}
                   << ColumnInfo{"rating", QCoreApplication::translate("ResultTableModel", "Rating"), QCoreApplication::translate("ResultTableModel", "Release group rating")}
                   << ColumnInfo{"release_count", QCoreApplication::translate("ResultTableModel", "Releases"), QCoreApplication::translate("ResultTableModel", "Number of releases in this group")};
            break;
            
        case EntityType::Work:
            columns << ColumnInfo{"title", QCoreApplication::translate("ResultTableModel", "Title"), QCoreApplication::translate("ResultTableModel", "Work title")}
                   << ColumnInfo{"type", QCoreApplication::translate("ResultTableModel", "Type"), QCoreApplication::translate("ResultTableModel", "Work type")}
                   << ColumnInfo{"language", QCoreApplication::translate("ResultTableModel", "Language"), QCoreApplication::translate("ResultTableModel", "Work language")}
                   << ColumnInfo{"disambiguation", QCoreApplication::translate("ResultTableModel", "Disambiguation"), QCoreApplication::translate("ResultTableModel", "Work disambiguation")}
                   << ColumnInfo{"tags", QCoreApplication::translate("ResultTableModel", "Tags"), QCoreApplication::translate("ResultTableModel", "Work tags")}
                   << ColumnInfo{"rating", QCoreApplication::translate("ResultTableModel", "Rating"), QCoreApplication::translate("ResultTableModel", "Work rating")}
                   << ColumnInfo{"recording_count", QCoreApplication::translate("ResultTableModel", "Recordings"), QCoreApplication::translate("ResultTableModel", "Number of recordings of this work")};
            break;
            
        case EntityType::Label:
            columns << ColumnInfo{"name", QCoreApplication::translate("ResultTableModel", "Name"), QCoreApplication::translate("ResultTableModel", "Label name")}
                   << ColumnInfo{"sort_name", QCoreApplication::translate("ResultTableModel", "Sort Name"), QCoreApplication::translate("ResultTableModel", "Label sort name")}
                   << ColumnInfo{"type", QCoreApplication::translate("ResultTableModel", "Type"), QCoreApplication::translate("ResultTableModel", "Label type")}
                   << ColumnInfo{"code", QCoreApplication::translate("ResultTableModel", "Code"), QCoreApplication::translate("ResultTableModel", "Label code")}
                   << ColumnInfo{"country", QCoreApplication::translate("ResultTableModel", "Country"), QCoreApplication::translate("ResultTableModel", "Label country")}
                   << ColumnInfo{"life_span", QCoreApplication::translate("ResultTableModel", "Life Span"), QCoreApplication::translate("ResultTableModel", "Label life span")}
                   << ColumnInfo{"disambiguation", QCoreApplication::translate("ResultTableModel", "Disambiguation"), QCoreApplication::translate("ResultTableModel", "Label disambiguation")}
                   << ColumnInfo{"tags", QCoreApplication::translate("ResultTableModel", "Tags"), QCoreApplication::translate("ResultTableModel", "Label tags")}
                   << ColumnInfo{"rating", QCoreApplication::translate("ResultTableModel", "Rating"), QCoreApplication::translate("ResultTableModel", "Label rating")}
                   << ColumnInfo{"release_count", QCoreApplication::translate("ResultTableModel", "Releases"), QCoreApplication::translate("ResultTableModel", "Number of releases on this label")};
            break;
            
        default:
            columns << ColumnInfo{"name", QCoreApplication::translate("ResultTableModel", "Name"), QCoreApplication::translate("ResultTableModel", "Entity name")}
                   << ColumnInfo{"id", QCoreApplication::translate("ResultTableModel", "ID"), QCoreApplication::translate("ResultTableModel", "Entity ID")}
                   << ColumnInfo{"score", QCoreApplication::translate("ResultTableModel", "Score"), QCoreApplication::translate("ResultTableModel", "Search score")};
            break;
    }
    
    return columns;
}

QStringList ResultTableModel::getAvailableColumnNames(EntityType type) const {
    QStringList names;
    const auto columns = getAvailableColumns(type);
    for (const auto &column : columns) {
        names << column.header;
    }
    return names;
}

QStringList ResultTableModel::getVisibleColumns() const {
    QStringList keys;
    for (const auto &column : m_visibleColumns) {
        keys << column.key;
    }
    return keys;
}

void ResultTableModel::setVisibleColumns(const QStringList &columnKeys) {
    beginResetModel();
    
    m_visibleColumns.clear();
    const auto availableColumns = getAvailableColumns(m_type);
    
    // 根据键值找到对应的列信息
    for (const QString &key : columnKeys) {
        auto it = std::find_if(availableColumns.begin(), availableColumns.end(),
                              [&key](const ColumnInfo &info) { return info.key == key; });
        if (it != availableColumns.end()) {
            m_visibleColumns << *it;
        }
    }
    
    // 如果没有找到任何列，使用默认列
    if (m_visibleColumns.isEmpty()) {
        m_visibleColumns = getDefaultColumns(m_type);
    }
    endResetModel();
}

void ResultTableModel::resetToDefaultColumns() {
    setVisibleColumns(QStringList()); // 空列表会触发使用默认列
}

QList<ColumnInfo> ResultTableModel::getDefaultColumns(EntityType type) const {
    QList<ColumnInfo> defaultColumns;
    
    switch (type) {
        case EntityType::Artist:
            defaultColumns << ColumnInfo{"name", QCoreApplication::translate("ResultTableModel", "Name"), QCoreApplication::translate("ResultTableModel", "Artist name")}
                          << ColumnInfo{"sort_name", QCoreApplication::translate("ResultTableModel", "Sort Name"), QCoreApplication::translate("ResultTableModel", "Artist sort name")}
                          << ColumnInfo{"type", QCoreApplication::translate("ResultTableModel", "Type"), QCoreApplication::translate("ResultTableModel", "Artist type")}
                          << ColumnInfo{"country", QCoreApplication::translate("ResultTableModel", "Country"), QCoreApplication::translate("ResultTableModel", "Artist country")}
                          << ColumnInfo{"life_span", QCoreApplication::translate("ResultTableModel", "Life Span"), QCoreApplication::translate("ResultTableModel", "Artist life span")};
            break;
            
        case EntityType::Release:
            defaultColumns << ColumnInfo{"title", QCoreApplication::translate("ResultTableModel", "Title"), QCoreApplication::translate("ResultTableModel", "Release title")}
                          << ColumnInfo{"artist", QCoreApplication::translate("ResultTableModel", "Artist"), QCoreApplication::translate("ResultTableModel", "Release artist")}
                          << ColumnInfo{"date", QCoreApplication::translate("ResultTableModel", "Date"), QCoreApplication::translate("ResultTableModel", "Release date")}
                          << ColumnInfo{"country", QCoreApplication::translate("ResultTableModel", "Country"), QCoreApplication::translate("ResultTableModel", "Release country")}
                          << ColumnInfo{"status", QCoreApplication::translate("ResultTableModel", "Status"), QCoreApplication::translate("ResultTableModel", "Release status")};
            break;
            
        case EntityType::Recording:
            defaultColumns << ColumnInfo{"title", QCoreApplication::translate("ResultTableModel", "Title"), QCoreApplication::translate("ResultTableModel", "Recording title")}
                          << ColumnInfo{"artist", QCoreApplication::translate("ResultTableModel", "Artist"), QCoreApplication::translate("ResultTableModel", "Recording artist")}
                          << ColumnInfo{"length", QCoreApplication::translate("ResultTableModel", "Length"), QCoreApplication::translate("ResultTableModel", "Recording length")}
                          << ColumnInfo{"disambiguation", QCoreApplication::translate("ResultTableModel", "Disambiguation"), QCoreApplication::translate("ResultTableModel", "Recording disambiguation")};
            break;
            
        case EntityType::ReleaseGroup:
            defaultColumns << ColumnInfo{"title", QCoreApplication::translate("ResultTableModel", "Title"), QCoreApplication::translate("ResultTableModel", "Release group title")}
                          << ColumnInfo{"artist", QCoreApplication::translate("ResultTableModel", "Artist"), QCoreApplication::translate("ResultTableModel", "Release group artist")}
                          << ColumnInfo{"type", QCoreApplication::translate("ResultTableModel", "Type"), QCoreApplication::translate("ResultTableModel", "Release group type")}
                          << ColumnInfo{"first_release_date", QCoreApplication::translate("ResultTableModel", "First Release Date"), QCoreApplication::translate("ResultTableModel", "First release date")};
            break;
            
        case EntityType::Work:
            defaultColumns << ColumnInfo{"title", QCoreApplication::translate("ResultTableModel", "Title"), QCoreApplication::translate("ResultTableModel", "Work title")}
                          << ColumnInfo{"type", QCoreApplication::translate("ResultTableModel", "Type"), QCoreApplication::translate("ResultTableModel", "Work type")}
                          << ColumnInfo{"language", QCoreApplication::translate("ResultTableModel", "Language"), QCoreApplication::translate("ResultTableModel", "Work language")};
            break;
            
        case EntityType::Label:
            defaultColumns << ColumnInfo{"name", QCoreApplication::translate("ResultTableModel", "Name"), QCoreApplication::translate("ResultTableModel", "Label name")}
                          << ColumnInfo{"type", QCoreApplication::translate("ResultTableModel", "Type"), QCoreApplication::translate("ResultTableModel", "Label type")}
                          << ColumnInfo{"country", QCoreApplication::translate("ResultTableModel", "Country"), QCoreApplication::translate("ResultTableModel", "Label country")};
            break;
            
        default:
            defaultColumns << ColumnInfo{"name", QCoreApplication::translate("ResultTableModel", "Name"), QCoreApplication::translate("ResultTableModel", "Entity name")}
                          << ColumnInfo{"id", QCoreApplication::translate("ResultTableModel", "ID"), QCoreApplication::translate("ResultTableModel", "Entity ID")};
            break;
    }
    
    return defaultColumns;
}

QVariant ResultTableModel::getFieldValue(const QSharedPointer<ResultItem> &item, const QString &fieldKey) const {
    if (!item) {
        return QVariant();
    }
      // 首先尝试从基本属性获取
    if (fieldKey == "name") {
        return item->getName();
    } else if (fieldKey == "id") {
        return item->getId();
    } else if (fieldKey == "score") {
        // score 不是 ResultItem 的直接属性，尝试从详细数据获取
        return item->getDetailProperty("score");
    } else if (fieldKey == "type") {
        return item->getTypeString();
    } else if (fieldKey == "disambiguation") {
        return item->getDetailProperty("disambiguation");
    } else if (fieldKey == "sort_name") {
        return item->getDetailProperty("sort_name");
    } else if (fieldKey == "country") {
        return item->getDetailProperty("country");
    } else if (fieldKey == "life_span") {
        return item->getDetailProperty("life_span");
    } else if (fieldKey == "title") {
        return item->getDetailProperty("title");
    } else if (fieldKey == "artist") {
        return item->getDetailProperty("artist");
    } else if (fieldKey == "date") {
        return item->getDetailProperty("date");
    } else if (fieldKey == "status") {
        return item->getDetailProperty("status");
    }
    
    // 然后尝试从详细信息获取
    QVariant detailValue = item->getDetailProperty(fieldKey);
    if (detailValue.isValid()) {
        return detailValue;
    }
    
    // 最后尝试从详细数据获取并格式化
    const QVariantMap details = item->getDetailData();
    if (details.contains(fieldKey)) {
        QVariant value = details[fieldKey];
        
        // 格式化特殊字段
        if (fieldKey.endsWith("_count")) {
            return value.toInt();
        } else if (fieldKey == "length") {
            int ms = value.toInt();
            if (ms > 0) {
                int seconds = ms / 1000;
                int minutes = seconds / 60;
                seconds %= 60;
                return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
            }
        } else if (fieldKey == "rating") {
            double rating = value.toDouble();
            if (rating > 0) {
                return QString::number(rating, 'f', 1);
            }
        } else if (fieldKey == "tags" || fieldKey == "aliases" || fieldKey == "isrcs") {
            QStringList list = value.toStringList();
            return list.join(", ");
        }
        
        return value;
    }    
    return QVariant();
}

QList<ColumnInfo> ResultTableModel::detectColumnsFromData() const {
    QList<ColumnInfo> detectedColumns;
    QSet<QString> allFields;
    
    // 收集所有项目中出现的字段
    for (const auto &item : m_items) {
        if (!item) continue;
        
        // 从基本数据收集字段
        allFields.insert("name");
        if (!item->getId().isEmpty()) allFields.insert("id");
        
        // 从详细数据收集字段
        QVariantMap details = item->getDetailData();
        for (auto it = details.begin(); it != details.end(); ++it) {
            if (!it.value().isNull() && !it.value().toString().isEmpty()) {
                allFields.insert(it.key());
            }
        }
    }
    
    // 将字段转换为列信息，优先使用已知的列定义
    QList<ColumnInfo> knownColumns = getAvailableColumns(m_type);
    QSet<QString> usedFields;
    
    // 首先添加已知的列
    for (const auto &knownColumn : knownColumns) {
        if (allFields.contains(knownColumn.key)) {
            detectedColumns << knownColumn;
            usedFields.insert(knownColumn.key);
        }
    }
    
    // 然后添加未知的字段作为新列
    for (const QString &field : allFields) {
        if (!usedFields.contains(field)) {
            QString displayName = field;
            displayName = displayName.replace('_', ' ');
            displayName = displayName.replace('-', ' ');
            // 首字母大写
            if (!displayName.isEmpty()) {
                displayName[0] = displayName[0].toUpper();
            }
            
            detectedColumns << ColumnInfo{
                field, 
                displayName, 
                QCoreApplication::translate("ResultTableModel", "Dynamic field: %1").arg(field)
            };
        }
    }
    
    return detectedColumns;
}

void ResultTableModel::updateColumnsFromData() {
    QList<ColumnInfo> detectedColumns = detectColumnsFromData();
    
    if (!detectedColumns.isEmpty()) {
        m_visibleColumns = detectedColumns;
    } else {
        // 如果没有检测到任何列，使用默认列
        m_visibleColumns = getDefaultColumns(m_type);
    }
}

void ResultTableModel::sort(int column, Qt::SortOrder order) {
    if (column < 0 || column >= m_visibleColumns.count() || m_items.isEmpty()) {
        return;
    }
    
    const QString &fieldKey = m_visibleColumns[column].key;
    
    emit layoutAboutToBeChanged();
    
    // 创建一个包含原始索引的排序对列表
    QList<QPair<QSharedPointer<ResultItem>, int>> sortPairs;
    for (int i = 0; i < m_items.size(); ++i) {
        sortPairs.append(qMakePair(m_items[i], i));
    }
    
    // 执行排序
    std::sort(sortPairs.begin(), sortPairs.end(), [this, &fieldKey, order](const QPair<QSharedPointer<ResultItem>, int> &a, const QPair<QSharedPointer<ResultItem>, int> &b) {
        QVariant valueA = getFieldValue(a.first, fieldKey);
        QVariant valueB = getFieldValue(b.first, fieldKey);
        
        // 处理空值
        if (!valueA.isValid() && !valueB.isValid()) {
            return false;
        }
        if (!valueA.isValid()) {
            return order == Qt::AscendingOrder ? true : false;
        }
        if (!valueB.isValid()) {
            return order == Qt::AscendingOrder ? false : true;
        }
          // 比较值
        bool result = false;
        if (valueA.typeId() == QMetaType::QString && valueB.typeId() == QMetaType::QString) {
            result = valueA.toString().compare(valueB.toString(), Qt::CaseInsensitive) < 0;
        } else if (valueA.canConvert<double>() && valueB.canConvert<double>()) {
            result = valueA.toDouble() < valueB.toDouble();
        } else {
            result = valueA.toString().compare(valueB.toString(), Qt::CaseInsensitive) < 0;
        }
        
        return order == Qt::AscendingOrder ? result : !result;
    });
    
    // 更新项目列表
    for (int i = 0; i < sortPairs.size(); ++i) {
        m_items[i] = sortPairs[i].first;
    }
    
    emit layoutChanged();
}

QString ResultTableModel::generateFriendlyColumnName(const QString &key) const {
    static const QMap<QString, QString> friendlyNames = {
        {"id", QCoreApplication::translate("ResultTableModel", "ID")},
        {"name", QCoreApplication::translate("ResultTableModel", "Name")},
        {"title", QCoreApplication::translate("ResultTableModel", "Title")},
        {"sort_name", QCoreApplication::translate("ResultTableModel", "Sort Name")},
        {"disambiguation", QCoreApplication::translate("ResultTableModel", "Disambiguation")},
        {"score", QCoreApplication::translate("ResultTableModel", "Score")},
        {"type", QCoreApplication::translate("ResultTableModel", "Type")},
        {"country", QCoreApplication::translate("ResultTableModel", "Country")},
        {"date", QCoreApplication::translate("ResultTableModel", "Date")},
        {"first_release_date", QCoreApplication::translate("ResultTableModel", "First Release Date")},
        {"life_span", QCoreApplication::translate("ResultTableModel", "Life Span")},
        {"gender", QCoreApplication::translate("ResultTableModel", "Gender")},
        {"length", QCoreApplication::translate("ResultTableModel", "Length")},
        {"track_count", QCoreApplication::translate("ResultTableModel", "Tracks")},
        {"release_count", QCoreApplication::translate("ResultTableModel", "Releases")},
        {"recording_count", QCoreApplication::translate("ResultTableModel", "Recordings")},
        {"work_count", QCoreApplication::translate("ResultTableModel", "Works")},
        {"release_group_count", QCoreApplication::translate("ResultTableModel", "Release Groups")},
        {"artist", QCoreApplication::translate("ResultTableModel", "Artist")},
        {"status", QCoreApplication::translate("ResultTableModel", "Status")},
        {"packaging", QCoreApplication::translate("ResultTableModel", "Packaging")},
        {"format", QCoreApplication::translate("ResultTableModel", "Format")},
        {"catalog_number", QCoreApplication::translate("ResultTableModel", "Catalog Number")},
        {"barcode", QCoreApplication::translate("ResultTableModel", "Barcode")},
        {"label", QCoreApplication::translate("ResultTableModel", "Label")},
        {"language", QCoreApplication::translate("ResultTableModel", "Language")},
        {"code", QCoreApplication::translate("ResultTableModel", "Code")},
        {"tags", QCoreApplication::translate("ResultTableModel", "Tags")},
        {"rating", QCoreApplication::translate("ResultTableModel", "Rating")},
        {"aliases", QCoreApplication::translate("ResultTableModel", "Aliases")},
        {"isrcs", QCoreApplication::translate("ResultTableModel", "ISRCs")}
    };
    
    if (friendlyNames.contains(key)) {
        return friendlyNames[key];
    }
    
    // 默认转换：将下划线替换为空格，首字母大写
    QString friendly = key;
    friendly.replace('_', ' ');
    friendly.replace('-', ' ');
    if (!friendly.isEmpty()) {
        friendly[0] = friendly[0].toUpper();
    }
    return friendly;
}

QString ResultTableModel::generateColumnDescription(const QString &key, EntityType type) const {
    QString entityName = EntityUtils::entityTypeToString(type);
    return QCoreApplication::translate("ResultTableModel", "%1 %2").arg(entityName, generateFriendlyColumnName(key));
}

int ResultTableModel::getColumnPriority(const QString &key, EntityType type) const {
    // 定义不同实体类型的列优先级
    static const QMap<EntityType, QStringList> priorities = {
        {EntityType::Artist, {"name", "sort_name", "type", "country", "life_span", "gender", "disambiguation", "score"}},
        {EntityType::Release, {"title", "artist", "date", "country", "status", "track_count", "disambiguation", "score"}},
        {EntityType::Recording, {"title", "artist", "length", "disambiguation", "release_count", "score"}},
        {EntityType::ReleaseGroup, {"title", "artist", "type", "first_release_date", "release_count", "disambiguation", "score"}},
        {EntityType::Work, {"title", "type", "language", "disambiguation", "recording_count", "score"}},
        {EntityType::Label, {"name", "sort_name", "type", "country", "code", "life_span", "disambiguation", "score"}}
    };
    
    if (priorities.contains(type)) {
        const QStringList &priorityList = priorities[type];
        int index = priorityList.indexOf(key);
        if (index >= 0) {
            return index;
        }
    }
    
    // 未在优先级列表中的字段放在后面
    return 1000;
}
