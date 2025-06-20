#include "musicbrainzparser.h"
#include "musicbrainz_utils.h"
#include "../utils/logger.h"
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
        qCWarning(logApi) << "MusicBrainzParser::parseSearchResponse - Invalid JSON data";
        return results;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    
    // 检查API错误
    QString error = checkForErrors(root);
    if (!error.isEmpty()) {
        qCWarning(logApi) << "MusicBrainzParser::parseSearchResponse - API Error:" << error;
        return results;
    }
    
    // 如果没有指定类型，尝试自动检测
    EntityType actualType = expectedType;
    if (actualType == EntityType::Unknown) {
        // 通过检查root对象的键来推断类型
        QStringList keys = root.keys();
        QStringList entityTypes = {"artists", "releases", "recordings", "release-groups", "works", "labels", "areas"};
          for (const QString &entityType : entityTypes) {
            if (keys.contains(entityType)) {
                actualType = MusicBrainzUtils::stringToEntityType(entityType.chopped(1)); // 去掉末尾的's'
                break;
            }
        }
    }
    
    if (actualType == EntityType::Unknown) {
        qCWarning(logApi) << "MusicBrainzParser::parseSearchResponse - Could not determine entity type";
        return results;
    }
    
    QString entityPluralName = MusicBrainzUtils::getEntityPluralName(actualType);
    QJsonArray items = root.value(entityPluralName).toArray();
      qCDebug(logApi) << "MusicBrainzParser::parseSearchResponse - Parsing" << items.size() 
                    << "items of type" << MusicBrainzUtils::entityTypeToString(actualType);
    
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
        qCWarning(logApi) << "MusicBrainzParser::parseDetailsResponse - Invalid JSON data";
        return nullptr;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    
    // 检查API错误
    QString error = checkForErrors(root);
    if (!error.isEmpty()) {
        qCWarning(logApi) << "MusicBrainzParser::parseDetailsResponse - API Error:" << error;
        return nullptr;
    }
    
    // 自动检测实体类型
    EntityType actualType = detectEntityType(root);
    if (expectedType != EntityType::Unknown && actualType != expectedType) {        qCWarning(logApi) << "MusicBrainzParser::parseDetailsResponse - Type mismatch: expected" 
                          << MusicBrainzUtils::entityTypeToString(expectedType) << "got" << MusicBrainzUtils::entityTypeToString(actualType);
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
        qCWarning(logApi) << "MusicBrainzParser::parseEntity - Missing required fields (id or name/title)";
        return nullptr;
    }
    
    auto resultItem = QSharedPointer<ResultItem>::create(id, name, type);
    
    // 解析基本信息
    parseBasicEntityInfo(resultItem, jsonObj);
    
    // 解析类型特定属性
    parseTypeSpecificProperties(resultItem, jsonObj, type);    qCDebug(logApi) << "MusicBrainzParser::parseEntity - Parsed entity:" << name 
                    << "(" << MusicBrainzUtils::entityTypeToString(type) << ") with" 
                    << resultItem->getDetailData().size() << "detail properties";
    
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
            QJsonObject obj = value.toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                map.insert(it.key(), parseJsonValue(it.value()));
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
        qCWarning(logApi) << "MusicBrainzParser::validateJsonData - JSON parse error:" << error.errorString();
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
        item->setDetailProperty("tags_list", tags);
    }
    
    // 解析别名
    if (jsonObj.contains("aliases")) {
        QJsonArray aliasesArray = jsonObj.value("aliases").toArray();
        QVariantList aliases = parseAliases(aliasesArray);
        item->setDetailProperty("aliases_list", aliases);
    }
}

void MusicBrainzParser::parseReleaseProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 解析艺术家信用
    if (jsonObj.contains("artist-credit")) {
        QJsonArray artistCreditArray = jsonObj.value("artist-credit").toArray();
        QVariantList artistCredits = parseArtistCredits(artistCreditArray);
        item->setDetailProperty("artist_credits", artistCredits);
    }
    
    // 解析发行事件
    if (jsonObj.contains("release-events")) {
        QJsonArray releaseEventsArray = jsonObj.value("release-events").toArray();
        QVariantList releaseEvents = parseReleaseEvents(releaseEventsArray);
        item->setDetailProperty("release_events", releaseEvents);
    }
    
    // 解析媒体
    if (jsonObj.contains("media")) {
        QJsonArray mediaArray = jsonObj.value("media").toArray();
        QVariantList media = parseMedia(mediaArray);
        item->setDetailProperty("media_list", media);
    }
    
    // 解析封面艺术档案
    if (jsonObj.contains("cover-art-archive")) {
        QJsonObject coverArtObj = jsonObj.value("cover-art-archive").toObject();
        QVariantMap coverArt = parseCoverArtArchive(coverArtObj);
        item->setDetailProperty("cover_art_archive", coverArt);
    }
    
    // 解析文本表示
    if (jsonObj.contains("text-representation")) {
        QJsonObject textRepObj = jsonObj.value("text-representation").toObject();
        QVariantMap textRep = parseTextRepresentation(textRepObj);
        item->setDetailProperty("text_representation", textRep);
    }
    
    // 解析标签
    if (jsonObj.contains("tags")) {
        QJsonArray tagsArray = jsonObj.value("tags").toArray();
        QVariantList tags = parseTags(tagsArray);
        item->setDetailProperty("tags_list", tags);
    }
}

void MusicBrainzParser::parseRecordingProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 解析艺术家信用
    if (jsonObj.contains("artist-credit")) {
        QJsonArray artistCreditArray = jsonObj.value("artist-credit").toArray();
        QVariantList artistCredits = parseArtistCredits(artistCreditArray);
        item->setDetailProperty("artist_credits", artistCredits);
    }
    
    // 解析发行列表
    if (jsonObj.contains("releases")) {
        QJsonArray releasesArray = jsonObj.value("releases").toArray();
        QVariantList releases;
        for (const QJsonValue &value : releasesArray) {
            if (value.isObject()) {
                QJsonObject releaseObj = value.toObject();
                QVariantMap release;
                release["id"] = getJsonString(releaseObj, "id");
                release["title"] = getJsonString(releaseObj, "title");
                release["status"] = getJsonString(releaseObj, "status");
                release["date"] = getJsonString(releaseObj, "date");
                releases.append(release);
            }
        }
        item->setDetailProperty("releases_list", releases);
    }
    
    // 解析标签
    if (jsonObj.contains("tags")) {
        QJsonArray tagsArray = jsonObj.value("tags").toArray();
        QVariantList tags = parseTags(tagsArray);
        item->setDetailProperty("tags_list", tags);
    }
}

void MusicBrainzParser::parseReleaseGroupProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 解析艺术家信用
    if (jsonObj.contains("artist-credit")) {
        QJsonArray artistCreditArray = jsonObj.value("artist-credit").toArray();
        QVariantList artistCredits = parseArtistCredits(artistCreditArray);
        item->setDetailProperty("artist_credits", artistCredits);
    }
    
    // 解析发行列表
    if (jsonObj.contains("releases")) {
        QJsonArray releasesArray = jsonObj.value("releases").toArray();
        QVariantList releases;
        for (const QJsonValue &value : releasesArray) {
            if (value.isObject()) {
                QJsonObject releaseObj = value.toObject();
                QVariantMap release;
                release["id"] = getJsonString(releaseObj, "id");
                release["title"] = getJsonString(releaseObj, "title");
                release["status"] = getJsonString(releaseObj, "status");
                release["date"] = getJsonString(releaseObj, "date");
                release["country"] = getJsonString(releaseObj, "country");
                releases.append(release);
            }
        }
        item->setDetailProperty("releases_list", releases);
    }
    
    // 解析标签
    if (jsonObj.contains("tags")) {
        QJsonArray tagsArray = jsonObj.value("tags").toArray();
        QVariantList tags = parseTags(tagsArray);
        item->setDetailProperty("tags_list", tags);
    }
}

void MusicBrainzParser::parseWorkProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 解析录音列表
    if (jsonObj.contains("recordings")) {
        QJsonArray recordingsArray = jsonObj.value("recordings").toArray();
        QVariantList recordings;
        for (const QJsonValue &value : recordingsArray) {
            if (value.isObject()) {
                QJsonObject recordingObj = value.toObject();
                QVariantMap recording;
                recording["id"] = getJsonString(recordingObj, "id");
                recording["title"] = getJsonString(recordingObj, "title");
                recording["length"] = getJsonInt(recordingObj, "length");
                recordings.append(recording);
            }
        }
        item->setDetailProperty("recordings_list", recordings);
    }
    
    // 解析标签
    if (jsonObj.contains("tags")) {
        QJsonArray tagsArray = jsonObj.value("tags").toArray();
        QVariantList tags = parseTags(tagsArray);
        item->setDetailProperty("tags_list", tags);
    }
}

void MusicBrainzParser::parseLabelProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
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
    
    // 解析发行列表
    if (jsonObj.contains("releases")) {
        QJsonArray releasesArray = jsonObj.value("releases").toArray();
        QVariantList releases;
        for (const QJsonValue &value : releasesArray) {
            if (value.isObject()) {
                QJsonObject releaseObj = value.toObject();
                QVariantMap release;
                release["id"] = getJsonString(releaseObj, "id");
                release["title"] = getJsonString(releaseObj, "title");
                release["status"] = getJsonString(releaseObj, "status");
                release["date"] = getJsonString(releaseObj, "date");
                releases.append(release);
            }
        }
        item->setDetailProperty("releases_list", releases);
    }
    
    // 解析标签
    if (jsonObj.contains("tags")) {
        QJsonArray tagsArray = jsonObj.value("tags").toArray();
        QVariantList tags = parseTags(tagsArray);
        item->setDetailProperty("tags_list", tags);
    }
}

void MusicBrainzParser::parseAreaProperties(QSharedPointer<ResultItem> &item, const QJsonObject &jsonObj)
{
    // 解析生命周期
    if (jsonObj.contains("life-span")) {
        QJsonObject lifeSpanObj = jsonObj.value("life-span").toObject();
        QVariantMap lifeSpan = parseLifeSpan(lifeSpanObj);
        item->setDetailProperty("life_span", lifeSpan);
    }
    
    // 解析标签
    if (jsonObj.contains("tags")) {
        QJsonArray tagsArray = jsonObj.value("tags").toArray();
        QVariantList tags = parseTags(tagsArray);
        item->setDetailProperty("tags_list", tags);
    }
}

// =============================================================================
// 复杂结构解析方法
// =============================================================================

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

QVariantList MusicBrainzParser::parseRelations(const QJsonArray &relations)
{
    QVariantList result;
    
    for (const QJsonValue &value : relations) {
        if (value.isObject()) {
            QJsonObject relationObj = value.toObject();
            QVariantMap relation;
            
            relation["type"] = getJsonString(relationObj, "type");
            relation["type-id"] = getJsonString(relationObj, "type-id");
            relation["direction"] = getJsonString(relationObj, "direction");
            relation["begin"] = getJsonString(relationObj, "begin");
            relation["end"] = getJsonString(relationObj, "end");
            relation["ended"] = getJsonBool(relationObj, "ended");
            
            // 解析目标实体
            if (relationObj.contains("artist")) {
                relation["target-type"] = "artist";
                relation["target"] = parseJsonValue(relationObj.value("artist"));
            } else if (relationObj.contains("release")) {
                relation["target-type"] = "release";
                relation["target"] = parseJsonValue(relationObj.value("release"));
            } else if (relationObj.contains("recording")) {
                relation["target-type"] = "recording";
                relation["target"] = parseJsonValue(relationObj.value("recording"));
            } else if (relationObj.contains("work")) {
                relation["target-type"] = "work";
                relation["target"] = parseJsonValue(relationObj.value("work"));
            } else if (relationObj.contains("url")) {
                relation["target-type"] = "url";
                relation["target"] = parseJsonValue(relationObj.value("url"));
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
            
            medium["position"] = getJsonInt(mediaObj, "position");
            medium["title"] = getJsonString(mediaObj, "title");
            medium["format"] = getJsonString(mediaObj, "format");
            medium["format-id"] = getJsonString(mediaObj, "format-id");
            medium["track-count"] = getJsonInt(mediaObj, "track-count");
            
            // 解析曲目列表
            if (mediaObj.contains("tracks")) {
                QJsonArray tracksArray = mediaObj.value("tracks").toArray();
                QVariantList tracks;
                
                for (const QJsonValue &trackValue : tracksArray) {
                    if (trackValue.isObject()) {
                        QJsonObject trackObj = trackValue.toObject();
                        QVariantMap track;
                        
                        track["id"] = getJsonString(trackObj, "id");
                        track["position"] = getJsonInt(trackObj, "position");
                        track["title"] = getJsonString(trackObj, "title");
                        track["length"] = getJsonInt(trackObj, "length");
                        track["number"] = getJsonString(trackObj, "number");
                        
                        if (trackObj.contains("artist-credit")) {
                            QJsonArray artistCreditArray = trackObj.value("artist-credit").toArray();
                            track["artist-credit"] = parseArtistCredits(artistCreditArray);
                        }
                        
                        if (trackObj.contains("recording")) {
                            track["recording"] = parseJsonValue(trackObj.value("recording"));
                        }
                        
                        tracks.append(track);
                    }
                }
                
                medium["tracks"] = tracks;
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
            
            tag["count"] = getJsonInt(tagObj, "count");
            tag["name"] = getJsonString(tagObj, "name");
            
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
            
            alias["name"] = getJsonString(aliasObj, "name");
            alias["sort-name"] = getJsonString(aliasObj, "sort-name");
            alias["type"] = getJsonString(aliasObj, "type");
            alias["type-id"] = getJsonString(aliasObj, "type-id");
            alias["locale"] = getJsonString(aliasObj, "locale");
            alias["primary"] = getJsonBool(aliasObj, "primary");
            alias["begin"] = getJsonString(aliasObj, "begin");
            alias["end"] = getJsonString(aliasObj, "end");
            alias["ended"] = getJsonBool(aliasObj, "ended");
            
            result.append(alias);
        }
    }
    
    return result;
}

QVariantMap MusicBrainzParser::parseLifeSpan(const QJsonObject &lifeSpan)
{
    QVariantMap result;
    
    result["begin"] = getJsonString(lifeSpan, "begin");
    result["end"] = getJsonString(lifeSpan, "end");
    result["ended"] = getJsonBool(lifeSpan, "ended");
    
    return result;
}

QVariantMap MusicBrainzParser::parseArea(const QJsonObject &area)
{
    QVariantMap result;
    
    result["id"] = getJsonString(area, "id");
    result["name"] = getJsonString(area, "name");
    result["sort-name"] = getJsonString(area, "sort-name");
    result["type"] = getJsonString(area, "type");
    result["type-id"] = getJsonString(area, "type-id");
    
    // ISO 代码
    if (area.contains("iso-3166-1-codes")) {
        result["iso-3166-1-codes"] = parseJsonValue(area.value("iso-3166-1-codes"));
    }
    
    if (area.contains("iso-3166-2-codes")) {
        result["iso-3166-2-codes"] = parseJsonValue(area.value("iso-3166-2-codes"));
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
            
            event["date"] = getJsonString(eventObj, "date");
            
            if (eventObj.contains("area")) {
                QJsonObject areaObj = eventObj.value("area").toObject();
                event["area"] = parseArea(areaObj);
            }
            
            result.append(event);
        }
    }
    
    return result;
}

QVariantMap MusicBrainzParser::parseCoverArtArchive(const QJsonObject &coverArtArchive)
{
    QVariantMap result;
    
    result["artwork"] = getJsonBool(coverArtArchive, "artwork");
    result["count"] = getJsonInt(coverArtArchive, "count");
    result["front"] = getJsonBool(coverArtArchive, "front");
    result["back"] = getJsonBool(coverArtArchive, "back");
    
    return result;
}

QVariantMap MusicBrainzParser::parseTextRepresentation(const QJsonObject &textRepresentation)
{
    QVariantMap result;
    
    result["language"] = getJsonString(textRepresentation, "language");
    result["script"] = getJsonString(textRepresentation, "script");
    
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
