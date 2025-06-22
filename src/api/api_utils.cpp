#include "api_utils.h"
#include "musicbrainzparser.h"
#include "../models/resultitem.h"
#include <QJsonDocument>
#include <QRegularExpression>
#include <algorithm>

// UrlBuilder 实现
const QString UrlBuilder::BASE_URL = "https://musicbrainz.org/ws/2/";

QUrl UrlBuilder::createBaseUrl(const QString& endpoint)
{
    return QUrl(BASE_URL + endpoint);
}

void UrlBuilder::addStandardParams(QUrlQuery& query, const QStringList& includes)
{
    query.addQueryItem("fmt", "json");
    if (!includes.isEmpty()) {
        query.addQueryItem("inc", includes.join(" "));
    }
}

void UrlBuilder::addPaginationParams(QUrlQuery& query, int limit, int offset)
{
    if (limit > 0) {
        query.addQueryItem("limit", QString::number(limit));
    }
    if (offset > 0) {
        query.addQueryItem("offset", QString::number(offset));
    }
}

void UrlBuilder::addCustomParams(QUrlQuery& query, const QVariantMap& params)
{
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.addQueryItem(it.key(), it.value().toString());
    }
}

QString UrlBuilder::buildSearchUrl(const QString& query, EntityType type, int limit, int offset)
{
    QString entityStr = EntityUtils::entityTypeToString(type);
    if (entityStr.isEmpty() || entityStr == "unknown") {
        return QString();
    }
    
    QUrl url = createBaseUrl(entityStr);
    QUrlQuery queryParams;
    
    queryParams.addQueryItem("query", query);
    addPaginationParams(queryParams, limit, offset);
    addStandardParams(queryParams);
    
    url.setQuery(queryParams);
    return url.toString();
}

QString UrlBuilder::buildDetailsUrl(const QString& mbid, EntityType type)
{
    QString entityStr = EntityUtils::entityTypeToString(type);
    if (entityStr.isEmpty() || entityStr == "unknown") {
        return QString();
    }
    
    QUrl url = createBaseUrl(entityStr + "/" + mbid);
    QUrlQuery queryParams;
    
    QStringList includes = {EntityUtils::getDefaultIncludes(type)};
    addStandardParams(queryParams, includes);
    
    url.setQuery(queryParams);
    return url.toString();
}

QString UrlBuilder::buildDiscIdUrl(const QString& discId)
{
    QUrl url = createBaseUrl("discid/" + discId);
    QUrlQuery queryParams;
    
    QStringList includes = {"artists", "recordings", "labels", "release-groups", "artist-credits"};
    addStandardParams(queryParams, includes);
    
    url.setQuery(queryParams);
    return url.toString();
}

QString UrlBuilder::buildGenericUrl(const QString& entity, const QString& id,
                                   const QString& resource, const QStringList& includes,
                                   const QVariantMap& params)
{
    QString urlPath = entity;
    
    if (!id.isEmpty()) {
        urlPath += "/" + id;
    }
    
    if (!resource.isEmpty()) {
        urlPath += "/" + resource;
    }
    
    QUrl url = createBaseUrl(urlPath);
    QUrlQuery queryParams;
    
    addStandardParams(queryParams, includes);
    addCustomParams(queryParams, params);
    
    url.setQuery(queryParams);
    return url.toString();
}

QString UrlBuilder::buildBrowseUrl(const QString& entity, const QString& relatedEntity,
                                  const QString& relatedId, int limit, int offset)
{
    QUrl url = createBaseUrl(entity);
    QUrlQuery queryParams;
    
    addStandardParams(queryParams);
    queryParams.addQueryItem(relatedEntity, relatedId);
    addPaginationParams(queryParams, limit, offset);
    
    url.setQuery(queryParams);
    return url.toString();
}

QString UrlBuilder::buildCollectionUrl(const QString& collectionId, const QString& action)
{
    QString urlPath = "collection";
    
    if (!collectionId.isEmpty()) {
        urlPath += "/" + collectionId;
        
        if (!action.isEmpty()) {
            urlPath += "/" + action;
        }
    }
    
    QUrl url = createBaseUrl(urlPath);
    QUrlQuery queryParams;
    addStandardParams(queryParams);
    
    url.setQuery(queryParams);
    return url.toString();
}

// ResponseParser 实现
QJsonObject ResponseParser::parseJsonResponse(const QByteArray& data, QString* error)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        if (error) {
            *error = parseError.errorString();
        }
        return QJsonObject();
    }
    
    return doc.object();
}

QPair<int, int> ResponseParser::extractPagination(const QJsonObject& obj)
{
    int count = obj.value("count").toInt(0);
    int offset = obj.value("offset").toInt(0);
    return qMakePair(count, offset);
}

QList<QSharedPointer<ResultItem>> ResponseParser::parseItemList(const QJsonArray& array, EntityType type)
{
    QList<QSharedPointer<ResultItem>> items;
    
    // 创建临时的MusicBrainzParser实例来处理解析
    MusicBrainzParser parser;
    
    for (const QJsonValue& value : array) {
        auto item = parser.parseEntity(value.toObject(), type);
        if (item) {
            items.append(item);
        }
    }
    
    return items;
}

QList<QVariantMap> ResponseParser::parseCollectionList(const QJsonObject& obj)
{
    QList<QVariantMap> collections;
    
    if (obj.contains("collection-list")) {
        QJsonArray array = obj["collection-list"].toArray();
        for (const QJsonValue& value : array) {
            collections.append(value.toObject().toVariantMap());
        }
    }
    
    return collections;
}

QString ResponseParser::extractDiscIdFromUrl(const QString& url)
{
    // 从URL中提取DiscID: .../discid/XXXXX?...
    QStringList parts = url.split('/');
    for (int i = 0; i < parts.size() - 1; ++i) {
        if (parts[i] == "discid" && i + 1 < parts.size()) {
            QString discId = parts[i + 1];
            // 移除查询参数
            return discId.section('?', 0, 0);
        }
    }
    return QString();
}

// Validator 实现
bool Validator::isValidMbid(const QString& mbid)
{
    if (mbid.isEmpty()) {
        return false;
    }
    
    // MBID应该是UUID格式: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    QRegularExpression uuidRegex("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");
    return uuidRegex.match(mbid).hasMatch();
}

bool Validator::isValidEntity(const QString& entity)
{
    if (entity.isEmpty()) {
        return false;
    }
    
    // 检查是否是已知的实体类型
    EntityType type = EntityUtils::stringToEntityType(entity);
    return type != EntityType::Unknown;
}

QString Validator::validateAndCleanQuery(const QString& query)
{
    QString cleaned = query.trimmed();
    
    // 移除潜在的危险字符，但保留Lucene查询语法
    // 这里可以添加更多的清理逻辑
    
    return cleaned;
}

QPair<int, int> Validator::validatePagination(int limit, int offset)
{
    // MusicBrainz API限制
    int validLimit = qBound(1, limit, 100);  // 最小1，最大100
    int validOffset = qMax(0, offset);        // 不能为负数
    
    return qMakePair(validLimit, validOffset);
}

// =============================================================================
// EntityUtils 实现（合并自musicbrainz_utils）
// =============================================================================

namespace EntityUtils {

constexpr const EntityMapping* findEntityMapping(EntityType type) noexcept {
    for (const auto& mapping : ENTITY_MAPPINGS) {
        if (mapping.type == type) {
            return &mapping;
        }
    }
    return nullptr;
}

QString entityTypeToString(EntityType type)
{
    if (const auto* mapping = findEntityMapping(type)) {
        return QString::fromLatin1(mapping->singular.data(), mapping->singular.size());
    }
    return "unknown";
}

EntityType stringToEntityType(const QString &typeStr)
{
    const auto typeView = typeStr.toLatin1();
    
    auto it = std::find_if(ENTITY_MAPPINGS.begin(), ENTITY_MAPPINGS.end(),
        [&typeView](const EntityMapping& mapping) {
            return mapping.singular == typeView.data();
        });
    
    return (it != ENTITY_MAPPINGS.end()) ? it->type : EntityType::Unknown;
}

QString getEntityPluralName(EntityType type)
{
    if (const auto* mapping = findEntityMapping(type)) {
        return QString::fromLatin1(mapping->plural.data(), mapping->plural.size());
    }
    return "unknown";
}

QString getDefaultIncludes(EntityType type)
{
    if (const auto* mapping = findEntityMapping(type)) {
        return QString::fromLatin1(mapping->defaultIncludes.data(), mapping->defaultIncludes.size());
    }
    return "tags+ratings+genres";
}

}
