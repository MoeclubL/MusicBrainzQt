#ifndef API_UTILS_H
#define API_UTILS_H

#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVariantMap>
#include <QSharedPointer>
#include <array>
#include <string_view>
#include "../core/types.h"

class ResultItem;

// =============================================================================
// MusicBrainz实体类型工具（合并自musicbrainz_utils）
// =============================================================================

/**
 * @brief 实体映射结构
 */
struct EntityMapping {
    EntityType type;
    std::string_view singular;
    std::string_view plural;
    std::string_view defaultIncludes;
};

/**
 * @brief 编译时实体类型映射表
 */
constexpr std::array<EntityMapping, 16> ENTITY_MAPPINGS = {{
    {EntityType::Artist, "artist", "artists", "aliases+tags+ratings+genres+recordings+releases+release-groups+works+artist-rels+release-rels+release-group-rels+recording-rels+work-rels+url-rels"},
    {EntityType::Release, "release", "releases", "aliases+tags+ratings+genres+artists+labels+recordings+release-groups+artist-rels+release-rels+release-group-rels+recording-rels+work-rels+url-rels"},
    {EntityType::Recording, "recording", "recordings", "aliases+tags+ratings+genres+artists+releases+release-groups+isrcs+artist-rels+release-rels+release-group-rels+recording-rels+work-rels+url-rels"},
    {EntityType::ReleaseGroup, "release-group", "release-groups", "aliases+tags+ratings+genres+artists+releases+artist-rels+release-rels+release-group-rels+recording-rels+work-rels+url-rels"},
    {EntityType::Label, "label", "labels", "aliases+tags+ratings+genres+releases+artist-rels+release-rels+release-group-rels+recording-rels+work-rels+url-rels"},
    {EntityType::Work, "work", "works", "aliases+tags+ratings+genres+artist-rels+release-rels+release-group-rels+recording-rels+work-rels+url-rels"},
    {EntityType::Area, "area", "areas", "aliases+tags+ratings+genres"},
    {EntityType::Place, "place", "places", "aliases+tags+ratings+genres"},
    {EntityType::Annotation, "annotation", "annotations", "tags+ratings+genres"},
    {EntityType::CDStub, "cdstub", "cdstubs", "tags+ratings+genres"},
    {EntityType::Editor, "editor", "editors", "tags+ratings+genres"},
    {EntityType::Tag, "tag", "tags", "tags+ratings+genres"},
    {EntityType::Instrument, "instrument", "instruments", "aliases+tags+ratings+genres"},
    {EntityType::Series, "series", "series", "aliases+tags+ratings+genres"},
    {EntityType::Event, "event", "events", "aliases+tags+ratings+genres"},
    {EntityType::Documentation, "documentation", "documentation", "tags+ratings+genres"}
}};

/**
 * @brief MusicBrainz实体工具函数
 */
namespace EntityUtils {
    /**
     * @brief 查找实体映射信息
     */
    constexpr const EntityMapping* findEntityMapping(EntityType type) noexcept;
    
    /**
     * @brief 实体类型转字符串
     */
    QString entityTypeToString(EntityType type);
    
    /**
     * @brief 字符串转实体类型
     */
    EntityType stringToEntityType(const QString &typeStr);
    
    /**
     * @brief 获取实体复数名称
     */
    QString getEntityPluralName(EntityType type);
    
    /**
     * @brief 获取默认包含参数
     */
    QString getDefaultIncludes(EntityType type);
}

/**
 * @brief API请求类型枚举
 */
enum class RequestType {
    Search,
    Details, 
    DiscId,
    Generic,
    Browse,
    Collection
};



/**
 * @brief 统一URL构建器
 */
class UrlBuilder {
public:
    static const QString BASE_URL;
    
    /**
     * @brief 创建基础URL
     */
    static QUrl createBaseUrl(const QString& endpoint);
    
    /**
     * @brief 添加标准参数
     */
    static void addStandardParams(QUrlQuery& query, const QStringList& includes = {});
    
    /**
     * @brief 添加分页参数
     */
    static void addPaginationParams(QUrlQuery& query, int limit, int offset);
    
    /**
     * @brief 添加自定义参数
     */
    static void addCustomParams(QUrlQuery& query, const QVariantMap& params);
    
    /**
     * @brief 构建搜索URL
     */
    static QString buildSearchUrl(const QString& query, EntityType type, int limit, int offset);
    
    /**
     * @brief 构建详情URL
     */
    static QString buildDetailsUrl(const QString& mbid, EntityType type);
    
    /**
     * @brief 构建DiscID查找URL
     */
    static QString buildDiscIdUrl(const QString& discId);
    
    /**
     * @brief 构建通用查询URL
     */
    static QString buildGenericUrl(const QString& entity, const QString& id = QString(),
                                  const QString& resource = QString(), 
                                  const QStringList& includes = {},
                                  const QVariantMap& params = {});
    
    /**
     * @brief 构建浏览URL
     */
    static QString buildBrowseUrl(const QString& entity, const QString& relatedEntity,
                                 const QString& relatedId, int limit, int offset);
    
    /**
     * @brief 构建集合URL
     */
    static QString buildCollectionUrl(const QString& collectionId = QString(), 
                                     const QString& action = QString());
};

/**
 * @brief 响应解析器
 */
class ResponseParser {
public:
    /**
     * @brief 解析JSON响应
     */
    static QJsonObject parseJsonResponse(const QByteArray& data, QString* error = nullptr);
    
    /**
     * @brief 提取分页信息
     */
    static QPair<int, int> extractPagination(const QJsonObject& obj);
    
    /**
     * @brief 解析条目列表
     */
    static QList<QSharedPointer<ResultItem>> parseItemList(const QJsonArray& array, EntityType type);
    
    /**
     * @brief 解析集合列表
     */
    static QList<QVariantMap> parseCollectionList(const QJsonObject& obj);
    
    /**
     * @brief 从URL提取DiscID
     */
    static QString extractDiscIdFromUrl(const QString& url);
};

/**
 * @brief 参数验证器
 */
class Validator {
public:
    /**
     * @brief 验证MBID格式
     */
    static bool isValidMbid(const QString& mbid);
    
    /**
     * @brief 验证实体类型
     */
    static bool isValidEntity(const QString& entity);
    
    /**
     * @brief 验证并清理查询字符串
     */
    static QString validateAndCleanQuery(const QString& query);
    
    /**
     * @brief 验证分页参数
     */
    static QPair<int, int> validatePagination(int limit, int offset);
};

#endif // API_UTILS_H
