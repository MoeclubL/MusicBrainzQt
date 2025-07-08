#include "resultitem.h"
#include <QStyle>
#include <QApplication>
#include <QRandomGenerator>

ResultItem::ResultItem(const QString &id, const QString &name, EntityType type)
    : m_id(id), m_name(name), m_type(type), m_score(0)
{
}

ResultItem::~ResultItem()
{
}

QString ResultItem::getId() const
{
    return m_id;
}

QString ResultItem::getName() const
{
    return m_name;
}

EntityType ResultItem::getType() const
{
    return m_type;
}

QIcon ResultItem::getIcon() const
{
    QStyle *style = QApplication::style();
      switch(m_type) {
        case EntityType::Artist:
            return QIcon(":/icons/artist.svg");
        case EntityType::Release:
            return QIcon(":/icons/album.svg");
        case EntityType::Recording:
            return QIcon(":/icons/recording.svg");
        case EntityType::Label:
            return QIcon(":/icons/label.svg");
        case EntityType::Work:
            return QIcon(":/icons/work.svg");
        case EntityType::Area:
            return QIcon(":/icons/area.svg");
        default:
            return style->standardIcon(QStyle::SP_FileIcon);
    }
}

QString ResultItem::getTypeString() const
{    switch(m_type) {
        case EntityType::Artist:
            return "Artist";
        case EntityType::Release:
            return "Release";
        case EntityType::ReleaseGroup:
            return "Release Group";
        case EntityType::Recording:
            return "Recording";
        case EntityType::Work:
            return "Work";
        case EntityType::Label:
            return "Label";
        case EntityType::Area:
            return "Area";
        case EntityType::Place:
            return "Place";
        case EntityType::Annotation:
            return "Annotation";
        case EntityType::CDStub:
            return "CD Stub";
        case EntityType::Editor:
            return "Editor";
        case EntityType::Tag:
            return "Tag";
        case EntityType::Instrument:
            return "Instrument";
        case EntityType::Series:
            return "Series";
        case EntityType::Event:
            return "Event";
        case EntityType::Documentation:
            return "Documentation";
        default:
            return "Unknown";
    }
}

QString ResultItem::getDisplayName() const
{
    return m_name;
}

QMap<QString, QVariant> ResultItem::getDetails() const
{
    QMap<QString, QVariant> details;
    details["ID"] = m_id;
    details["Name"] = m_name;
    details["Type"] = getTypeString();
    
    // 合并详细数据
    for (auto it = m_detailData.constBegin(); it != m_detailData.constEnd(); ++it) {
        QString key = it.key();
        QVariant value = it.value();
        
        // 对一些关键字段进行友好化处理
        if (key == "artistNames") {
            details["Artist"] = value;
        } else if (key == "country") {
            details["Country"] = value;
        } else if (key == "date") {
            details["Date"] = value;
        } else if (key == "firstReleaseDate") {
            details["Date"] = value; // ReleaseGroup 使用 first-release-date
        } else if (key == "length") {
            details["Length"] = value;        } else if (key == "type" && m_type != EntityType::Unknown) {
            // 避免覆盖基本类型信息，使用更具体的名称
            details["Subtype"] = value;
        } else if (key == "artist_type") {
            details["Subtype"] = value;
        } else if (key == "area") {
            details["Area"] = value;
        } else if (key == "origin") {
            details["Area"] = value;
        } else if (key == "disambiguation") {
            details["Disambiguation"] = value;
        } else if (key == "begin") {
            details["Begin"] = value;
        } else if (key == "birth_date") {
            details["Begin"] = value;
        } else if (key == "end") {
            details["End"] = value;
        } else if (key == "death_date") {
            details["End"] = value;
        } else if (key == "status") {
            details["Status"] = value;
        } else if (key == "barcode") {
            details["Barcode"] = value;
        } else if (key == "labels") {
            details["Labels"] = value;
        } else if (key == "gender") {
            details["Gender"] = value;
        } else if (key == "release_count") {
            details["release_count"] = value;
        } else if (key == "recording_count") {
            details["recording_count"] = value;
        } else if (key == "release_names") {
            details["release_names"] = value;
        } else if (key == "releases") {
            // 如果 releases 是列表，转换为字符串
            if (value.canConvert<QStringList>()) {
                details["release_names"] = value.toStringList().join(", ");
            } else {
                details["releases"] = value;
            }
        } else if (key == "aliases") {
            // 如果 aliases 是列表，转换为字符串
            if (value.canConvert<QStringList>()) {
                details["aliases"] = value.toStringList().join(", ");
            } else {
                details["aliases"] = value;
            }
        } else {
            // 其他字段直接使用原始名称
            details[key] = value;
        }
    }
    
    return details;
}

bool ResultItem::hasChildren() const
{
    // 默认情况下，艺术家和发行有子实体
    return (m_type == EntityType::Artist || m_type == EntityType::Release);
}

void ResultItem::setDetailData(const QVariantMap &detailData)
{
    m_detailData = detailData;
}

QVariantMap ResultItem::getDetailData() const
{
    return m_detailData;
}

void ResultItem::setDetailProperty(const QString &key, const QVariant &value)
{
    m_detailData[key] = value;
}

QVariant ResultItem::getDetailProperty(const QString &key) const
{
    return m_detailData.value(key);
}



void ResultItem::setDisambiguation(const QString &disambiguation)
{
    m_disambiguation = disambiguation;
}

QString ResultItem::getDisambiguation() const
{
    return m_disambiguation;
}

void ResultItem::setScore(int score)
{
    m_score = score;
}

int ResultItem::getScore() const
{
    return m_score;
}
