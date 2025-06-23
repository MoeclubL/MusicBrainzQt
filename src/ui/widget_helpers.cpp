#include "widget_helpers.h"
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QGuiApplication>
#include <QClipboard>
#include <QDate>
#include <QCoreApplication>
#include <QString>
#include "../core/types.h"
#include "../api/api_utils.h"

namespace WidgetHelpers {

// =============================================================================
// URL和链接相关
// =============================================================================

QString buildMusicBrainzUrl(const QString &entityId, EntityType type)
{
    if (entityId.isEmpty()) {
        return QString();
    }
    
    // 使用 API 层的统一工具类获取实体类型字符串
    QString typeStr = EntityUtils::entityTypeToString(type);
    if (typeStr.isEmpty() || typeStr == "unknown") {
        return QString();
    }
    
    return QString("https://musicbrainz.org/") + typeStr + "/" + entityId;
}

bool openUrlInBrowser(const QString &url)
{
    if (url.isEmpty()) {
        return false;
    }
    return QDesktopServices::openUrl(QUrl(url));
}

void copyToClipboard(const QString &text)
{
    if (!text.isEmpty()) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(text);
    }
}

// =============================================================================
// UI组件辅助
// =============================================================================

QWidget* createInfoItem(const QString &labelText, const QString &valueText, QWidget *parent)
{
    QWidget *widget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 2, 5, 2);
      QLabel *label = new QLabel(labelText + ":", widget);
    label->setProperty("class", "bold-text");
    label->setMinimumWidth(120);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    QLabel *value = new QLabel(valueText, widget);
    value->setWordWrap(true);
    value->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    value->setProperty("class", "secondary-text");
    
    layout->addWidget(label);
    layout->addWidget(value, 1);
    
    return widget;
}

QLabel* createGroupTitle(const QString &title, QWidget *parent)
{
    QLabel *titleLabel = new QLabel(title, parent);
    titleLabel->setObjectName("sectionTitle");
    titleLabel->setProperty("class", "section-title");
    titleLabel->setAlignment(Qt::AlignLeft);
    titleLabel->setMargin(5);
    return titleLabel;
}



// =============================================================================
// 格式化和显示
// =============================================================================

QString formatDuration(int milliseconds)
{
    if (milliseconds <= 0) {
        return QString();
    }
    
    int seconds = milliseconds / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;
    
    return QString::number(minutes) + ":" + QString::number(seconds).rightJustified(2, '0');
}

QString formatDate(const QString &dateString)
{
    if (dateString.isEmpty()) {
        return QString();
    }
    
    // 尝试解析不同的日期格式
    QDate date = QDate::fromString(dateString, Qt::ISODate);
    if (date.isValid()) {
        return date.toString(Qt::TextDate);  // 使用Qt::TextDate代替已弃用的DefaultLocaleLongDate
    }
    
    // 如果是部分日期（如"2023"），直接返回
    return dateString;
}

QString getEntityDisplayName(EntityType type)
{
    // 使用统一的实体类型映射，避免重复的switch语句
    const struct {
        EntityType type;
        const char* displayName;
    } displayNames[] = {
        {EntityType::Artist, QT_TRANSLATE_NOOP("UiUtils", "Artist")},
        {EntityType::Release, QT_TRANSLATE_NOOP("UiUtils", "Release")},
        {EntityType::ReleaseGroup, QT_TRANSLATE_NOOP("UiUtils", "Release Group")},
        {EntityType::Recording, QT_TRANSLATE_NOOP("UiUtils", "Recording")},
        {EntityType::Work, QT_TRANSLATE_NOOP("UiUtils", "Work")},
        {EntityType::Label, QT_TRANSLATE_NOOP("UiUtils", "Label")},
        {EntityType::Area, QT_TRANSLATE_NOOP("UiUtils", "Area")},
        {EntityType::Place, QT_TRANSLATE_NOOP("UiUtils", "Place")},
        {EntityType::Event, QT_TRANSLATE_NOOP("UiUtils", "Event")},
        {EntityType::Instrument, QT_TRANSLATE_NOOP("UiUtils", "Instrument")},
        {EntityType::Series, QT_TRANSLATE_NOOP("UiUtils", "Series")},
    };
    
    for (const auto& mapping : displayNames) {
        if (mapping.type == type) {
            return QCoreApplication::translate("UiUtils", mapping.displayName);
        }
    }
    
    return QCoreApplication::translate("UiUtils", "Unknown");
}

QString getEntityIconPath(EntityType type)
{
    // 使用统一的实体类型到图标映射
    const struct {
        EntityType type;
        const char* iconPath;
    } iconMappings[] = {
        {EntityType::Artist, ":/icons/artist.svg"},
        {EntityType::Release, ":/icons/album.svg"},
        {EntityType::ReleaseGroup, ":/icons/album.svg"},
        {EntityType::Recording, ":/icons/recording.svg"},
        {EntityType::Work, ":/icons/work.svg"},
        {EntityType::Label, ":/icons/label.svg"},
        {EntityType::Area, ":/icons/area.svg"},
    };
    
    for (const auto& mapping : iconMappings) {
        if (mapping.type == type) {
            return QString(mapping.iconPath);
        }
    }
    
    return ":/icons/browser.svg"; // 默认图标
}

}
