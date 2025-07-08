#include "musicbrainzparser.h"
#include "api_utils.h"
#include <QDebug>
#include <QRegularExpression>
#include <QDate>

MusicBrainzParser::MusicBrainzParser(QObject *parent)
    : QObject(parent)
{
}

// =============================================================================
// 主要解析方法
// =============================================================================

QList<QSharedPointer<ResultItem>> MusicBrainzParser::parseSearchResponse(const QByteArray &data, EntityType expectedType)
{
    QList<QSharedPointer<ResultItem>> results;
    
    if (!validateJsonData(data)) {
        qWarning() << "MusicBrainzParser::parseSearchResponse - Invalid JSON data";
        return results;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    
    // 检查API错误
    QString error = checkForErrors(root);
    if (!error.isEmpty()) {
        qWarning() << "MusicBrainzParser::parseSearchResponse - API Error:" << error;
        return results;
    }
    
    // 如果没有指定类型，尝试自动检测
    EntityType actualType = expectedType;
    if (actualType == EntityType::Unknown) {        // 通过检查root对象的键来推断类型
        QStringList keys = root.keys();
        QStringList entityTypes;
        entityTypes << "artists" << "releases" << "recordings" << "release-groups" << "works" << "labels" << "areas";
          for (const QString &entityType : entityTypes) {
            if (keys.contains(entityType)) {
                actualType = EntityUtils::stringToEntityType(entityType.chopped(1)); // 去掉末尾的's'
                break;
            }
        }
    }
    
    if (actualType == EntityType::Unknown) {
        qWarning() << "MusicBrainzParser::parseSearchResponse - Could not determine entity type";
        return results;
    }
    
    QString entityPluralName = EntityUtils::getEntityPluralName(actualType);
    QJsonArray items = root.value(entityPluralName).toArray();
      qDebug() << "MusicBrainzParser::parseSearchResponse - Parsing" << items.size() 
               << "items of type" << EntityUtils::entityTypeToString(actualType);
    
    for (const QJsonValue &value : items) {
        if (value.isObject()) {
            QJsonObject itemObj = value.toObject();
            auto resultItem = parseEntity(itemObj, actualType);
            if (resultItem) {
                results.append(resultItem);
            }
        }
    }
    
    return results;
}

QSharedPointer<ResultItem> MusicBrainzParser::parseDetailsResponse(const QByteArray &data, EntityType expectedType)
{
    if (!validateJsonData(data)) {
        qWarning() << "MusicBrainzParser::parseDetailsResponse - Invalid JSON data";
        return nullptr;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    
    // 检查API错误
    QString error = checkForErrors(root);
    if (!error.isEmpty()) {
        qWarning() << "MusicBrainzParser::parseDetailsResponse - API Error:" << error;
        return nullptr;
    }
    
    // 自动检测实体类型
    EntityType actualType = detectEntityType(root);
    if (expectedType != EntityType::Unknown && actualType != expectedType) {        
        qWarning() << "MusicBrainzParser::parseDetailsResponse - Type mismatch: expected" 
                   << EntityUtils::entityTypeToString(expectedType) << "got" << EntityUtils::entityTypeToString(actualType);
    }
    
    return parseEntity(root, actualType);
}

QSharedPointer<ResultItem> MusicBrainzParser::parseEntity(const QJsonObject &jsonObj, EntityType type)
{
    if (jsonObj.isEmpty()) {
        return nullptr;
    }
    
    // 自动检测类型（如果未指定）
    if (type == EntityType::Unknown) {
        type = detectEntityType(jsonObj);
    }
    
    // 获取基本信息
    QString id = getJsonString(jsonObj, "id");
    QString name = getJsonString(jsonObj, "name");
    
    // 对于某些实体类型，name字段可能叫title
    if (name.isEmpty()) {
        name = getJsonString(jsonObj, "title");
    }
    
    if (id.isEmpty() || name.isEmpty()) {
        qWarning() << "MusicBrainzParser::parseEntity - Missing required fields (id or name/title)";
        return nullptr;
    }
    
    auto resultItem = QSharedPointer<ResultItem>::create(id, name, type);
    
    // 解析基本信息
    parseBasicEntityInfo(resultItem, jsonObj);
    
    // 解析类型特定属性
    parseTypeSpecificProperties(resultItem, jsonObj, type);    
    qDebug() << "MusicBrainzParser::parseEntity - Parsed entity:" << name 
             << "(" << EntityUtils::entityTypeToString(type) << ") with" 
             << resultItem->getDetailData().count() << "detail properties";
    
    return resultItem;
}

// =============================================================================
// 工具方法
// =============================================================================

EntityType MusicBrainzParser::detectEntityType(const QJsonObject &jsonObj)
{
    // 通过检查特定字段来判断实体类型
    QStringList keys = jsonObj.keys();
    
    // Artist 特征字段
    if (keys.contains("type") && keys.contains("sort-name") && 
        (keys.contains("gender") || keys.contains("life-span"))) {
        return EntityType::Artist;
    }
    
    // Release 特征字段
    if (keys.contains("status") && keys.contains("barcode") && 
        (keys.contains("release-events") || keys.contains("media"))) {
        return EntityType::Release;
    }
    
    // Recording 特征字段
    if (keys.contains("length") && keys.contains("artist-credit")) {
        return EntityType::Recording;
    }
    
    // ReleaseGroup 特征字段
    if (keys.contains("primary-type") || keys.contains("first-release-date")) {
        return EntityType::ReleaseGroup;
    }
    
    // Work 特征字段
    if (keys.contains("work-type") || keys.contains("language")) {
        return EntityType::Work;
    }
    
    // Label 特征字段
    if (keys.contains("label-code") || keys.contains("type")) {
        return EntityType::Label;
    }
    
    // Area 特征字段
    if (keys.contains("iso-3166-1-codes") || keys.contains("iso-3166-2-codes")) {
        return EntityType::Area;
    }
    
    // 如果无法明确判断，使用默认逻辑
    if (keys.contains("title")) {
        if (keys.contains("artist-credit")) {
            return EntityType::Recording;
        } else {
            return EntityType::Work;
        }
    }
    
    return EntityType::Unknown;
}

QVariant MusicBrainzParser::parseJsonValue(const QJsonValue &value)
{
    switch (value.type()) {
        case QJsonValue::Bool:
            return value.toBool();
        case QJsonValue::Double:
            return value.toDouble();
        case QJsonValue::String:
            return value.toString();
        case QJsonValue::Array: {
            QVariantList list;
            QJsonArray array = value.toArray();
            for (const QJsonValue &item : array) {
                list.append(parseJsonValue(item));
            }
            return list;
        }
        case QJsonValue::Object: {
            QVariantMap map;
            QJsonObject obj = value.toObject();            for (auto it = obj.begin(); it != obj.end(); ++it) {
                map[it.key()] = parseJsonValue(it.value());
            }
            return map;
        }
        case QJsonValue::Null:
        case QJsonValue::Undefined:
        default:
            return QVariant();
    }
}

bool MusicBrainzParser::validateJsonData(const QByteArray &data)
{
    if (data.isEmpty()) {
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "MusicBrainzParser::validateJsonData - JSON parse error:" << error.errorString();
        return false;
    }
    
    return doc.isObject();
}

QString MusicBrainzParser::checkForErrors(const QJsonObject &jsonObj)
{
    if (jsonObj.contains("error")) {
        return jsonObj.value("error").toString();
    }
    
    if (jsonObj.contains("errors")) {
        QJsonArray errors = jsonObj.value("errors").toArray();
        QStringList errorList;
        for (const QJsonValue &error : errors) {
            errorList.append(error.toString());
        }
        return errorList.join("; ");
    }
    
    return QString();
}

// =============================================================================
// 内部解析方法
// =============================================================================

void MusicBrainzParser::parseBasicEntityInfo(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 通用字段
    item->setDisambiguation(getJsonString(jsonObj, "disambiguation"));
    
    // 评分
    if (jsonObj.contains("score")) {
        item->setScore(getJsonInt(jsonObj, "score"));
    }
    
    // 递归解析所有字段为detail properties
    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
        const QString &key = it.key();
        const QJsonValue &value = it.value();
        
        // 跳过已经处理的基本字段
        if (key == "id" || key == "name" || key == "title" || key == "disambiguation" || key == "score") {
            continue;
        }
        
        // 转换JSON值为QVariant
        QVariant variantValue = parseJsonValue(value);
        item->setDetailProperty(key, variantValue);
    }
}

void MusicBrainzParser::parseTypeSpecificProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj, EntityType type)
{
    switch (type) {
        case EntityType::Artist:
            parseArtistProperties(item, jsonObj);
            break;
        case EntityType::Release:
            parseReleaseProperties(item, jsonObj);
            break;
        case EntityType::Recording:
            parseRecordingProperties(item, jsonObj);
            break;
        case EntityType::ReleaseGroup:
            parseReleaseGroupProperties(item, jsonObj);
            break;
        case EntityType::Work:
            parseWorkProperties(item, jsonObj);
            break;
        case EntityType::Label:
            parseLabelProperties(item, jsonObj);
            break;
        case EntityType::Area:
            parseAreaProperties(item, jsonObj);
            break;
        default:
            break;
    }
}

void MusicBrainzParser::parseArtistProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 解析生命周期
    if (jsonObj.contains("life-span")) {
        QJsonObject lifeSpanObj = jsonObj.value("life-span").toObject();
        QVariantMap lifeSpan = parseLifeSpan(lifeSpanObj);
        item->setDetailProperty("life_span", lifeSpan);
    }
    
    // 解析地区
    if (jsonObj.contains("area")) {
        QJsonObject areaObj = jsonObj.value("area").toObject();
        QVariantMap area = parseArea(areaObj);
        item->setDetailProperty("area_info", area);
    }
    
    // 解析开始地区
    if (jsonObj.contains("begin-area")) {
        QJsonObject beginAreaObj = jsonObj.value("begin-area").toObject();
        QVariantMap beginArea = parseArea(beginAreaObj);
        item->setDetailProperty("begin_area", beginArea);
    }
    
    // 解析结束地区
    if (jsonObj.contains("end-area")) {
        QJsonObject endAreaObj = jsonObj.value("end-area").toObject();
        QVariantMap endArea = parseArea(endAreaObj);
        item->setDetailProperty("end_area", endArea);
    }
    
    // 解析标签
    if (jsonObj.contains("tags")) {
        QJsonArray tagsArray = jsonObj.value("tags").toArray();
        QVariantList tags = parseTags(tagsArray);
        item->setDetailProperty("tags", tags); // 使用统一键名
    }
    
    // 解析别名
    if (jsonObj.contains("aliases")) {
        QJsonArray aliasesArray = jsonObj.value("aliases").toArray();
        QVariantList aliases = parseAliases(aliasesArray);
        item->setDetailProperty("aliases", aliases); // 使用统一键名
    }
      // 使用通用解析函数处理各种子实体列表
    parseCommonSubEntities(item, jsonObj);
    
    // 解析关系
    if (jsonObj.contains("relations")) {
        QJsonArray relationsArray = jsonObj.value("relations").toArray();
        qDebug() << "Artist relations found:" << relationsArray.size() << "for" << item->getName();
        QVariantList relationships = parseRelationships(relationsArray);
        item->setDetailProperty("relationships", relationships);
        qDebug() << "Artist relationships stored:" << relationships.count();
    } else {
        qDebug() << "No relations found for artist:" << item->getName();
    }
}

void MusicBrainzParser::parseReleaseProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 解析艺术家信息
    parseArtistCreditProperty(item, jsonObj);
    
    // 解析发行事件
    if (jsonObj.contains("release-events")) {
        QJsonArray releaseEventsArray = jsonObj.value("release-events").toArray();
        QVariantList releaseEvents = parseReleaseEvents(releaseEventsArray);
        item->setDetailProperty("release-events", releaseEvents);
    }
    
    // 解析媒体
    if (jsonObj.contains("media")) {
        QJsonArray mediaArray = jsonObj.value("media").toArray();
        QVariantList media = parseMedia(mediaArray);
        item->setDetailProperty("media", media);
    }
    
    // 解析封面艺术档案
    if (jsonObj.contains("cover-art-archive")) {
        QJsonObject coverArtObj = jsonObj.value("cover-art-archive").toObject();
        QVariantMap coverArt = parseCoverArtArchive(coverArtObj);
        item->setDetailProperty("cover-art-archive", coverArt);
    }
    
    // 解析文本表示
    if (jsonObj.contains("text-representation")) {
        QJsonObject textRepObj = jsonObj.value("text-representation").toObject();
        QVariantMap textRep = parseTextRepresentation(textRepObj);
        item->setDetailProperty("text-representation", textRep);
    }
    
    // 使用通用函数解析标签、别名和关系
    parseCommonProperties(item, jsonObj);
}

void MusicBrainzParser::parseRecordingProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 解析艺术家信息
    parseArtistCreditProperty(item, jsonObj);
    
    // 使用通用函数解析子实体和通用属性
    parseCommonSubEntities(item, jsonObj);
    parseCommonProperties(item, jsonObj);
}

void MusicBrainzParser::parseReleaseGroupProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 解析艺术家信息
    parseArtistCreditProperty(item, jsonObj);
    
    // 使用通用函数解析子实体和通用属性
    parseCommonSubEntities(item, jsonObj);
    parseCommonProperties(item, jsonObj);
}

void MusicBrainzParser::parseWorkProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 使用通用函数解析子实体和通用属性
    parseCommonSubEntities(item, jsonObj);
    parseCommonProperties(item, jsonObj);
}

void MusicBrainzParser::parseLabelProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 使用通用函数解析子实体和通用属性
    parseCommonSubEntities(item, jsonObj);
    parseCommonProperties(item, jsonObj);
}

void MusicBrainzParser::parseAreaProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 使用通用函数解析通用属性
    parseCommonProperties(item, jsonObj);
}

QVariantList MusicBrainzParser::parseArtistCredits(const QJsonArray &artistCredits)
{
    QVariantList result;
    
    for (const QJsonValue &value : artistCredits) {
        if (value.isObject()) {
            QJsonObject creditObj = value.toObject();
            QVariantMap credit;
            
            credit["name"] = getJsonString(creditObj, "name");
            credit["joinphrase"] = getJsonString(creditObj, "joinphrase");
            
            if (creditObj.contains("artist")) {
                QJsonObject artistObj = creditObj.value("artist").toObject();
                QVariantMap artist;
                artist["id"] = getJsonString(artistObj, "id");
                artist["name"] = getJsonString(artistObj, "name");
                artist["sort-name"] = getJsonString(artistObj, "sort-name");
                artist["type"] = getJsonString(artistObj, "type");
                credit["artist"] = artist;
            }
            
            result.append(credit);
        }
    }
    
    return result;
}

QVariantList MusicBrainzParser::parseRelationships(const QJsonArray &relations)
{
    QVariantList result;
    
    for (const QJsonValue &value : relations) {
        if (value.isObject()) {
            QJsonObject relationObj = value.toObject();
            QVariantMap relation;
            
            // 基本关系信息
            relation.insert("type", getJsonString(relationObj, "type"));
            relation.insert("type-id", getJsonString(relationObj, "type-id"));
            relation.insert("direction", getJsonString(relationObj, "direction"));
            relation.insert("begin", getJsonString(relationObj, "begin"));
            relation.insert("end", getJsonString(relationObj, "end"));
            relation.insert("ended", getJsonBool(relationObj, "ended"));
            
            // 解析目标实体
            if (relationObj.contains("artist")) {
                QJsonObject targetObj = relationObj.value("artist").toObject();
                QVariantMap target;
                target.insert("id", getJsonString(targetObj, "id"));
                target.insert("name", getJsonString(targetObj, "name"));
                target.insert("sort-name", getJsonString(targetObj, "sort-name"));
                target.insert("disambiguation", getJsonString(targetObj, "disambiguation"));
                relation.insert("artist", target);
            }
            else if (relationObj.contains("release")) {
                QJsonObject targetObj = relationObj.value("release").toObject();
                QVariantMap target;
                target.insert("id", getJsonString(targetObj, "id"));
                target.insert("title", getJsonString(targetObj, "title"));
                target.insert("status", getJsonString(targetObj, "status"));
                target.insert("disambiguation", getJsonString(targetObj, "disambiguation"));
                relation.insert("release", target);
            }
            else if (relationObj.contains("release-group")) {
                QJsonObject targetObj = relationObj.value("release-group").toObject();
                QVariantMap target;
                target.insert("id", getJsonString(targetObj, "id"));
                target.insert("title", getJsonString(targetObj, "title"));
                target.insert("primary-type", getJsonString(targetObj, "primary-type"));
                target.insert("disambiguation", getJsonString(targetObj, "disambiguation"));
                relation.insert("release-group", target);
            }
            else if (relationObj.contains("recording")) {
                QJsonObject targetObj = relationObj.value("recording").toObject();
                QVariantMap target;
                target.insert("id", getJsonString(targetObj, "id"));
                target.insert("title", getJsonString(targetObj, "title"));
                target.insert("length", getJsonInt(targetObj, "length"));
                target.insert("disambiguation", getJsonString(targetObj, "disambiguation"));
                relation.insert("recording", target);
            }
            else if (relationObj.contains("work")) {
                QJsonObject targetObj = relationObj.value("work").toObject();
                QVariantMap target;
                target.insert("id", getJsonString(targetObj, "id"));
                target.insert("title", getJsonString(targetObj, "title"));
                target.insert("type", getJsonString(targetObj, "type"));
                target.insert("disambiguation", getJsonString(targetObj, "disambiguation"));
                relation.insert("work", target);
            }
            else if (relationObj.contains("url")) {
                QJsonObject targetObj = relationObj.value("url").toObject();
                QVariantMap target;
                target.insert("id", getJsonString(targetObj, "id"));
                target.insert("resource", getJsonString(targetObj, "resource"));
                relation.insert("url", target);
            }
            
            // 解析属性
            if (relationObj.contains("attributes")) {
                QJsonArray attributesArray = relationObj.value("attributes").toArray();
                QVariantList attributes;
                for (const QJsonValue &attrValue : attributesArray) {
                    if (attrValue.isString()) {
                        attributes.append(attrValue.toString());
                    }
                }
                relation.insert("attributes", attributes);
            }
            
            result.append(relation);
        }
    }
    
    return result;
}

QVariantList MusicBrainzParser::parseMedia(const QJsonArray &media)
{
    QVariantList result;
    
    for (const QJsonValue &value : media) {
        if (value.isObject()) {
            QJsonObject mediaObj = value.toObject();
            QVariantMap medium;
            
            medium.insert("position", getJsonInt(mediaObj, "position"));
            medium.insert("title", getJsonString(mediaObj, "title"));
            medium.insert("format", getJsonString(mediaObj, "format"));
            medium.insert("format-id", getJsonString(mediaObj, "format-id"));
            medium.insert("track-count", getJsonInt(mediaObj, "track-count"));
            
            // 解析曲目列表
            if (mediaObj.contains("tracks")) {
                QJsonArray tracksArray = mediaObj.value("tracks").toArray();
                QVariantList tracks;
                
                for (const QJsonValue &trackValue : tracksArray) {
                    if (trackValue.isObject()) {
                        QJsonObject trackObj = trackValue.toObject();
                        QVariantMap track;
                        
                        track.insert("id", getJsonString(trackObj, "id"));
                        track.insert("position", getJsonInt(trackObj, "position"));
                        track.insert("title", getJsonString(trackObj, "title"));
                        track.insert("length", getJsonInt(trackObj, "length"));
                        track.insert("number", getJsonString(trackObj, "number"));
                        
                        if (trackObj.contains("artist-credit")) {
                            QJsonArray artistCreditArray = trackObj.value("artist-credit").toArray();
                            track.insert("artist-credit", parseArtistCredits(artistCreditArray));
                        }
                        
                        if (trackObj.contains("recording")) {
                            track.insert("recording", parseJsonValue(trackObj.value("recording")));
                        }
                        
                        tracks.append(track);
                    }
                }
                
                medium.insert("tracks", tracks);
            }
            
            result.append(medium);
        }
    }
    
    return result;
}

QVariantList MusicBrainzParser::parseTags(const QJsonArray &tags)
{
    QVariantList result;
    
    for (const QJsonValue &value : tags) {
        if (value.isObject()) {
            QJsonObject tagObj = value.toObject();
            QVariantMap tag;
            
            tag.insert("count", getJsonInt(tagObj, "count"));
            tag.insert("name", getJsonString(tagObj, "name"));
            
            result.append(tag);
        }
    }
    
    return result;
}

QVariantList MusicBrainzParser::parseAliases(const QJsonArray &aliases)
{
    QVariantList result;
    
    for (const QJsonValue &value : aliases) {
        if (value.isObject()) {
            QJsonObject aliasObj = value.toObject();
            QVariantMap alias;
            
            alias.insert("name", getJsonString(aliasObj, "name"));
            alias.insert("sort-name", getJsonString(aliasObj, "sort-name"));
            alias.insert("type", getJsonString(aliasObj, "type"));
            alias.insert("type-id", getJsonString(aliasObj, "type-id"));
            alias.insert("locale", getJsonString(aliasObj, "locale"));
            alias.insert("primary", getJsonBool(aliasObj, "primary"));
            alias.insert("begin", getJsonString(aliasObj, "begin"));
            alias.insert("end", getJsonString(aliasObj, "end"));
            alias.insert("ended", getJsonBool(aliasObj, "ended"));
            
            result.append(alias);
        }
    }
    
    return result;
}

QVariantMap MusicBrainzParser::parseLifeSpan(const QJsonObject &lifeSpan)
{
    QVariantMap result;
    
    result.insert("begin", getJsonString(lifeSpan, "begin"));
    result.insert("end", getJsonString(lifeSpan, "end"));
    result.insert("ended", getJsonBool(lifeSpan, "ended"));
    
    return result;
}

QVariantMap MusicBrainzParser::parseArea(const QJsonObject &area)
{
    QVariantMap result;
    
    result.insert("id", getJsonString(area, "id"));
    result.insert("name", getJsonString(area, "name"));
    result.insert("sort-name", getJsonString(area, "sort-name"));
    result.insert("type", getJsonString(area, "type"));
    result.insert("type-id", getJsonString(area, "type-id"));
    
    // ISO 代码
    if (area.contains("iso-3166-1-codes")) {
        result.insert("iso-3166-1-codes", parseJsonValue(area.value("iso-3166-1-codes")));
    }
    
    if (area.contains("iso-3166-2-codes")) {
        result.insert("iso-3166-2-codes", parseJsonValue(area.value("iso-3166-2-codes")));
    }
    
    return result;
}

QVariantList MusicBrainzParser::parseReleaseEvents(const QJsonArray &releaseEvents)
{
    QVariantList result;
    
    for (const QJsonValue &value : releaseEvents) {
        if (value.isObject()) {
            QJsonObject eventObj = value.toObject();
            QVariantMap event;
            
            event.insert("date", getJsonString(eventObj, "date"));
            
            if (eventObj.contains("area")) {
                QJsonObject areaObj = eventObj.value("area").toObject();
                event.insert("area", parseArea(areaObj));
            }
            
            result.append(event);
        }
    }
    
    return result;
}

QVariantMap MusicBrainzParser::parseCoverArtArchive(const QJsonObject &coverArtArchive)
{
    QVariantMap result;
    
    result.insert("artwork", getJsonBool(coverArtArchive, "artwork"));
    result.insert("count", getJsonInt(coverArtArchive, "count"));
    result.insert("front", getJsonBool(coverArtArchive, "front"));
    result.insert("back", getJsonBool(coverArtArchive, "back"));
    
    return result;
}

QVariantMap MusicBrainzParser::parseTextRepresentation(const QJsonObject &textRepresentation)
{
    QVariantMap result;
    
    result.insert("language", getJsonString(textRepresentation, "language"));
    result.insert("script", getJsonString(textRepresentation, "script"));
    
    return result;
}

// =============================================================================
// 工具方法
// =============================================================================

QString MusicBrainzParser::getJsonString(const QJsonObject &obj, const QString &key, const QString &defaultValue)
{
    if (obj.contains(key) && obj.value(key).isString()) {
        return obj.value(key).toString();
    }
    return defaultValue;
}

int MusicBrainzParser::getJsonInt(const QJsonObject &obj, const QString &key, int defaultValue)
{
    if (obj.contains(key)) {
        QJsonValue value = obj.value(key);
        if (value.isDouble()) {
            return static_cast<int>(value.toDouble());
        }
        if (value.isString()) {
            bool ok;
            int intValue = value.toString().toInt(&ok);
            if (ok) {
                return intValue;
            }
        }
    }
    return defaultValue;
}

bool MusicBrainzParser::getJsonBool(const QJsonObject &obj, const QString &key, bool defaultValue)
{
    if (obj.contains(key) && obj.value(key).isBool()) {
        return obj.value(key).toBool();
    }
    return defaultValue;
}

QJsonArray MusicBrainzParser::getJsonArray(const QJsonObject &obj, const QString &key)
{
    if (obj.contains(key) && obj.value(key).isArray()) {
        return obj.value(key).toArray();
    }
    return QJsonArray();
}

QJsonObject MusicBrainzParser::getJsonObject(const QJsonObject &obj, const QString &key)
{
    if (obj.contains(key) && obj.value(key).isObject()) {
        return obj.value(key).toObject();
    }
    return QJsonObject();
}

void MusicBrainzParser::parseCommonSubEntities(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj) {
    // 定义需要解析的子实体映射
    static const QMap<QString, EntityType> subEntityMappings = {
        {"recordings", EntityType::Recording},
        {"releases", EntityType::Release},
        {"release-groups", EntityType::ReleaseGroup},
        {"works", EntityType::Work}
    };
    
    // 遍历并解析所有存在的子实体列表
    for (auto it = subEntityMappings.begin(); it != subEntityMappings.end(); ++it) {
        const QString &arrayKey = it.key();
        const EntityType &entityType = it.value();
        
        if (jsonObj.contains(arrayKey)) {
            QVariantList subEntities = parseSubEntityList(jsonObj, arrayKey, entityType);
            item->setDetailProperty(arrayKey, subEntities);
        }
    }
}

void MusicBrainzParser::parseCommonProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj) {
    // 解析标签
    if (jsonObj.contains("tags")) {
        QJsonArray tagsArray = jsonObj.value("tags").toArray();
        QVariantList tags = parseTags(tagsArray);
        item->setDetailProperty("tags", tags);
    }
    
    // 解析别名
    if (jsonObj.contains("aliases")) {
        QJsonArray aliasesArray = jsonObj.value("aliases").toArray();
        QVariantList aliases = parseAliases(aliasesArray);
        item->setDetailProperty("aliases", aliases);
    }
    
    // 解析关系
    if (jsonObj.contains("relations")) {
        QJsonArray relationsArray = jsonObj.value("relations").toArray();
        QVariantList relationships = parseRelationships(relationsArray);
        item->setDetailProperty("relationships", relationships);
    }
}

void MusicBrainzParser::parseArtistCreditProperty(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj) {    if (jsonObj.contains("artist-credit")) {
        QJsonArray artistCreditArray = jsonObj.value("artist-credit").toArray();
        QVariantList artistCredits = parseArtistCredits(artistCreditArray);
        item->setDetailProperty("artist-credits", artistCredits);
    }
}

QVariantList MusicBrainzParser::parseSubEntityList(const QJsonObject &jsonObj, const QString &arrayKey, EntityType entityType)
{
    QVariantList result;
    
    if (!jsonObj.contains(arrayKey)) {
        return result;
    }
    
    QJsonArray entityArray = jsonObj.value(arrayKey).toArray();
    
    for (const QJsonValue &value : entityArray) {
        if (value.isObject()) {
            QJsonObject entityObj = value.toObject();
            
            // 创建简化的实体信息，只包含基本字段
            QVariantMap entityInfo;
            
            // 基本字段
            entityInfo.insert("id", getJsonString(entityObj, "id"));
            entityInfo.insert("name", getJsonString(entityObj, "name"));
            
            // 对于某些实体类型，name字段可能叫title
            if (entityInfo.value("name").toString().isEmpty()) {
                entityInfo.insert("name", getJsonString(entityObj, "title"));
            }
            
            entityInfo.insert("disambiguation", getJsonString(entityObj, "disambiguation"));
            
            // 根据实体类型添加特定字段
            switch (entityType) {
            case EntityType::Recording:
                entityInfo.insert("length", getJsonInt(entityObj, "length"));
                break;
            case EntityType::Release:
                entityInfo.insert("date", getJsonString(entityObj, "date"));
                entityInfo.insert("status", getJsonString(entityObj, "status"));
                entityInfo.insert("track-count", getJsonInt(entityObj, "track-count"));
                break;
            case EntityType::ReleaseGroup:
                entityInfo.insert("primary-type", getJsonString(entityObj, "primary-type"));
                entityInfo.insert("first-release-date", getJsonString(entityObj, "first-release-date"));
                break;
            case EntityType::Work:
                entityInfo.insert("type", getJsonString(entityObj, "type"));
                break;
            case EntityType::Artist:
                entityInfo.insert("type", getJsonString(entityObj, "type"));
                entityInfo.insert("sort-name", getJsonString(entityObj, "sort-name"));
                break;
            default:
                break;
            }
            
            result.append(entityInfo);
        }
    }
    
    return result;
}
