#ifndef WIDGET_HELPERS_H
#define WIDGET_HELPERS_H

#include <QString>
#include "../core/types.h"

class QWidget;
class QLabel;
class QVBoxLayout;

/**
 * @namespace WidgetHelpers
 * @brief UI层通用工具函数
 * 
 * 专为UI层提供的工具函数，包括：
 * - URL构建和链接处理
 * - UI组件辅助功能
 * - 格式化和显示助手
 * 
 * 注意：此工具类仅供UI层使用，不涉及API数据处理
 */
namespace WidgetHelpers {

// =============================================================================
// URL和链接相关
// =============================================================================

/**
 * @brief 构建MusicBrainz网站URL（UI层专用）
 * @param entityId 实体ID（MBID）
 * @param type 实体类型
 * @return 完整的MusicBrainz网站URL
 *
 * 此函数专为UI层在浏览器中打开"功能设计"
 * 与API层的实体转换逻辑完全分离
 */
QString buildMusicBrainzUrl(const QString &entityId, EntityType type);

/**
 * @brief 在系统默认浏览器中打开URL
 * @param url 要打开的URL
 * @return 是否成功打开
 */
bool openUrlInBrowser(const QString &url);

/**
 * @brief 复制文本到系统剪贴板
 * @param text 要复制的文本
 */
void copyToClipboard(const QString &text);

// =============================================================================
// UI组件辅助
// =============================================================================

/**
 * @brief 创建信息标签组件
 * @param labelText 标签文本
 * @param valueText 值文本
 * @param parent 父组件
 * @return 创建的标签组件
 */
QWidget* createInfoItem(const QString &labelText, const QString &valueText, QWidget *parent = nullptr);

/**
 * @brief 创建分组标题标签
 * @param title 标题文本
 * @param parent 父组件
 * @return 创建的标题标签
 */
QLabel* createGroupTitle(const QString &title, QWidget *parent = nullptr);



// =============================================================================
// 格式化和显示
// =============================================================================

/**
 * @brief 格式化时长显示
 * @param milliseconds 毫秒
 * @return 格式化的时长字符串
 */
QString formatDuration(int milliseconds);

/**
 * @brief 格式化日期显示
 * @param dateString 日期字符串
 * @return 格式化的日期显示
 */
QString formatDate(const QString &dateString);

/**
 * @brief 获取实体类型的显示名称
 * @param type 实体类型
 * @return 本地化的显示名称
 */
QString getEntityDisplayName(EntityType type);

/**
 * @brief 获取实体类型的图标资源路径
 * @param type 实体类型
 * @return 图标资源路径
 */
QString getEntityIconPath(EntityType type);

}

#endif // WIDGET_HELPERS_H
