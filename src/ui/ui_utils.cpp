#include "ui_utils.h"
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include <QGuiApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QTime>
#include <QDate>
#include <QCoreApplication>

namespace UiUtils {

// =============================================================================
// URL和链接相关
// =============================================================================

QString buildMusicBrainzUrl(const QString &entityId, EntityType type)
{
    if (entityId.isEmpty()) {
        return QString();
    }
    
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
        case EntityType::Area:
            typeStr = "area";
            break;
        case EntityType::Place:
            typeStr = "place";
            break;
        case EntityType::Event:
            typeStr = "event";
            break;
        case EntityType::Instrument:
            typeStr = "instrument";
            break;
        case EntityType::Series:
            typeStr = "series";
            break;
        default:
            return QString();
    }
    
    return QString("https://musicbrainz.org/%1/%2").arg(typeStr, entityId);
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
    label->setStyleSheet("font-weight: bold; color: #333;");
    label->setMinimumWidth(120);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    QLabel *value = new QLabel(valueText, widget);
    value->setWordWrap(true);
    value->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    value->setStyleSheet("color: #555;");
    
    layout->addWidget(label);
    layout->addWidget(value, 1);
    
    widget->setStyleSheet(getInfoItemStyleSheet());
    return widget;
}

QLabel* createGroupTitle(const QString &title, QWidget *parent)
{
    QLabel *titleLabel = new QLabel(title, parent);
    titleLabel->setStyleSheet(getTitleLabelStyleSheet());
    titleLabel->setAlignment(Qt::AlignLeft);
    titleLabel->setMargin(5);
    return titleLabel;
}

void clearLayout(QVBoxLayout *layout)
{
    if (!layout) return;
    
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
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
    
    return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
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
    switch (type) {
        case EntityType::Artist:
            return QCoreApplication::translate("UiUtils", "Artist");
        case EntityType::Release:
            return QCoreApplication::translate("UiUtils", "Release");
        case EntityType::ReleaseGroup:
            return QCoreApplication::translate("UiUtils", "Release Group");
        case EntityType::Recording:
            return QCoreApplication::translate("UiUtils", "Recording");
        case EntityType::Work:
            return QCoreApplication::translate("UiUtils", "Work");
        case EntityType::Label:
            return QCoreApplication::translate("UiUtils", "Label");
        case EntityType::Area:
            return QCoreApplication::translate("UiUtils", "Area");
        case EntityType::Place:
            return QCoreApplication::translate("UiUtils", "Place");
        case EntityType::Event:
            return QCoreApplication::translate("UiUtils", "Event");
        case EntityType::Instrument:
            return QCoreApplication::translate("UiUtils", "Instrument");
        case EntityType::Series:
            return QCoreApplication::translate("UiUtils", "Series");
        default:
            return QCoreApplication::translate("UiUtils", "Unknown");
    }
}

QString getEntityIconPath(EntityType type)
{
    switch (type) {
        case EntityType::Artist:
            return ":/icons/artist.svg";
        case EntityType::Release:
            return ":/icons/album.svg";
        case EntityType::ReleaseGroup:
            return ":/icons/album.svg";
        case EntityType::Recording:
            return ":/icons/recording.svg";
        case EntityType::Work:
            return ":/icons/work.svg";
        case EntityType::Label:
            return ":/icons/label.svg";
        case EntityType::Area:
            return ":/icons/area.svg";
        default:
            return ":/icons/browser.svg";
    }
}

// =============================================================================
// 样式和主题
// =============================================================================

QString getInfoItemStyleSheet()
{
    return "QWidget { "
           "background-color: transparent; "
           "border-bottom: 1px solid #e0e0e0; "
           "padding: 2px 0; "
           "}";
}

QString getGroupContainerStyleSheet()
{
    return "QWidget { "
           "background-color: #f8f9fa; "
           "border: 1px solid #dee2e6; "
           "border-radius: 5px; "
           "padding: 10px; "
           "margin: 5px; "
           "}";
}

QString getTitleLabelStyleSheet()
{
    return "QLabel { "
           "font-size: 14px; "
           "font-weight: bold; "
           "color: #2c3e50; "
           "background-color: #ecf0f1; "
           "padding: 8px 12px; "
           "border-radius: 4px; "
           "border-left: 4px solid #3498db; "
           "}";
}

}
