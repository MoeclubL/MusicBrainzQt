#ifndef MUSICBRAINZPARSER_H
#define MUSICBRAINZPARSER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QVariantMap>
#include <QVariantList>
#include <QSharedPointer>
#include <QList>
#include <QStringList>
#include "../models/resultitem.h"
#include "../core/types.h"

/**
 * @class MusicBrainzParser
 * @brief 通用的MusicBrainz数据解析器
 * 
 * 这个类提供了一个统一的解析接口，能够自动识别和解析MusicBrainz API返回的各种数据结构。
 * 支持搜索结果、详细信息查询和各种include参数的返回数据。
 * 
 * 特性：
 * - 自动实体类型识别
 * - 递归解析嵌套数据结构
 * - 统一的错误处理
 * - 支持所有MusicBrainz实体类型
 * - 自动处理artist-credits、recordings、releases等复杂结构
 */
class MusicBrainzParser : public QObject
{
    Q_OBJECT

public:
    explicit MusicBrainzParser(QObject *parent = nullptr);

    // =============================================================================
    // 主要解析方法
    // =============================================================================
    
    /**
     * @brief 解析搜索响应
     * @param data JSON响应数据
     * @param expectedType 期望的实体类型（可选，用于验证）
     * @return 解析后的ResultItem列表
     */
    QList<QSharedPointer<ResultItem>> parseSearchResponse(const QByteArray &data, EntityType expectedType = EntityType::Unknown);
    
    /**
     * @brief 解析详细信息响应
     * @param data JSON响应数据
     * @param expectedType 期望的实体类型（可选，用于验证）
     * @return 解析后的ResultItem
     */
    QSharedPointer<ResultItem> parseDetailsResponse(const QByteArray &data, EntityType expectedType = EntityType::Unknown);
    
    /**
     * @brief 解析任意JSON对象为实体
     * @param jsonObj JSON对象
     * @param type 实体类型（如果已知）
     * @return 解析后的ResultItem
     */
    QSharedPointer<ResultItem> parseEntity(const QJsonObject &jsonObj, EntityType type = EntityType::Unknown);

    // =============================================================================
    // 工具方法
    // =============================================================================
    
    /**
     * @brief 自动检测实体类型
     * @param jsonObj JSON对象
     * @return 检测到的实体类型
     */
    static EntityType detectEntityType(const QJsonObject &jsonObj);      /**
     * @brief 递归解析JSON值为QVariant
     * @param value JSON值
     * @return 解析后的QVariant
     */
    static QVariant parseJsonValue(const QJsonValue &value);

    // =============================================================================
    // 验证方法
    // =============================================================================
    
    /**
     * @brief 验证JSON数据的有效性
     * @param data JSON数据
     * @return 是否有效
     */
    static bool validateJsonData(const QByteArray &data);
    
    /**
     * @brief 检查解析错误
     * @param jsonObj JSON对象
     * @return 错误信息，如果没有错误则返回空字符串
     */
    static QString checkForErrors(const QJsonObject &jsonObj);

private:
    // =============================================================================
    // 内部解析方法
    // =============================================================================
    
    /**
     * @brief 解析基本实体信息
     * @param item ResultItem实例
     * @param jsonObj JSON对象
     */
    void parseBasicEntityInfo(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj);
    
    /**
     * @brief 解析特定类型的属性
     * @param item ResultItem实例
     * @param jsonObj JSON对象
     * @param type 实体类型
     */
    void parseTypeSpecificProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj, EntityType type);
    
    /**
     * @brief 解析Artist特定属性
     */
    void parseArtistProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj);
    
    /**
     * @brief 解析Release特定属性
     */
    void parseReleaseProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj);
    
    /**
     * @brief 解析Recording特定属性
     */
    void parseRecordingProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj);
    
    /**
     * @brief 解析ReleaseGroup特定属性
     */
    void parseReleaseGroupProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj);
    
    /**
     * @brief 解析Work特定属性
     */
    void parseWorkProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj);
    
    /**
     * @brief 解析Label特定属性
     */
    void parseLabelProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj);
    
    /**
     * @brief 解析Area特定属性
     */
    void parseAreaProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj);
    
    // =============================================================================
    // 复杂结构解析方法
    // =============================================================================
    
    /**
     * @brief 解析artist-credits数组
     * @param artistCredits JSON数组
     * @return 解析后的艺术家信息列表
     */
    QVariantList parseArtistCredits(const QJsonArray &artistCredits);
    
    /**
     * @brief 解析关系数据
     * @param relations JSON数组
     * @return 解析后的关系信息列表
     */
    QVariantList parseRelations(const QJsonArray &relations);
    
    /**
     * @brief 解析媒体信息
     * @param media JSON数组
     * @return 解析后的媒体信息列表
     */
    QVariantList parseMedia(const QJsonArray &media);
    
    /**
     * @brief 解析标签信息
     * @param tags JSON数组
     * @return 解析后的标签信息列表
     */
    QVariantList parseTags(const QJsonArray &tags);
    
    /**
     * @brief 解析别名信息
     * @param aliases JSON数组
     * @return 解析后的别名信息列表
     */
    QVariantList parseAliases(const QJsonArray &aliases);
    
    /**
     * @brief 解析生命周期信息
     * @param lifeSpan JSON对象
     * @return 解析后的生命周期信息
     */
    QVariantMap parseLifeSpan(const QJsonObject &lifeSpan);
    
    /**
     * @brief 解析地区信息
     * @param area JSON对象
     * @return 解析后的地区信息
     */
    QVariantMap parseArea(const QJsonObject &area);
    
    /**
     * @brief 解析发行事件
     * @param releaseEvents JSON数组
     * @return 解析后的发行事件列表
     */
    QVariantList parseReleaseEvents(const QJsonArray &releaseEvents);
    
    /**
     * @brief 解析封面艺术档案信息
     * @param coverArtArchive JSON对象
     * @return 解析后的封面艺术信息
     */
    QVariantMap parseCoverArtArchive(const QJsonObject &coverArtArchive);
    
    /**
     * @brief 解析文本表示信息
     * @param textRepresentation JSON对象
     * @return 解析后的文本表示信息
     */
    QVariantMap parseTextRepresentation(const QJsonObject &textRepresentation);

    // =============================================================================
    // 工具方法
    // =============================================================================
    
    /**
     * @brief 安全获取JSON字符串值
     * @param obj JSON对象
     * @param key 键名
     * @param defaultValue 默认值
     * @return 字符串值
     */
    static QString getJsonString(const QJsonObject &obj, const QString &key, const QString &defaultValue = QString());
    
    /**
     * @brief 安全获取JSON整数值
     * @param obj JSON对象
     * @param key 键名
     * @param defaultValue 默认值
     * @return 整数值
     */
    static int getJsonInt(const QJsonObject &obj, const QString &key, int defaultValue = 0);
    
    /**
     * @brief 安全获取JSON布尔值
     * @param obj JSON对象
     * @param key 键名
     * @param defaultValue 默认值
     * @return 布尔值
     */
    static bool getJsonBool(const QJsonObject &obj, const QString &key, bool defaultValue = false);
    
    /**
     * @brief 安全获取JSON数组
     * @param obj JSON对象
     * @param key 键名
     * @return JSON数组
     */
    static QJsonArray getJsonArray(const QJsonObject &obj, const QString &key);
    
    /**
     * @brief 安全获取JSON对象
     * @param obj JSON对象
     * @param key 键名
     * @return JSON对象
     */
    static QJsonObject getJsonObject(const QJsonObject &obj, const QString &key);
};

#endif // MUSICBRAINZPARSER_H
