/**
 * @file musicbrainz_response_handler.cpp
 * @brief MusicBrainz API响应处理器实现
 * 
 * 该文件实现了MusicBrainzResponseHandler类，负责处理来自MusicBrainz API的各种响应。
 * 主要功能包括：
 * - 解析搜索结果响应
 * - 解析实体详情响应
 * - 处理DiscID查找结果
 * - 处理通用API查询响应
 * - 处理浏览功能响应
 * - 处理用户集合相关操作响应
 * 
 * @author MusicBrainzQt Team
 * @date 2024
 */

#include "musicbrainz_response_handler.h"
#include "musicbrainzparser.h"
#include "api_utils.h"
#include "../models/resultitem.h"
#include "../core/error_types.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MusicBrainzResponseHandler::MusicBrainzResponseHandler(MusicBrainzParser *parser, QObject *parent)
    : QObject(parent)
    , m_parser(parser)
{
}

void MusicBrainzResponseHandler::handleSearchResponse(const QByteArray& data, const QVariantMap& context)
{
    EntityType entityType = static_cast<EntityType>(context.value("entityType").toInt());
    
    QString error;
    QJsonObject obj = ResponseParser::parseJsonResponse(data, &error);
    if (!error.isEmpty()) {
        emit errorOccurred("Failed to parse search response: " + error);
        return;
    }
    
    QList<QSharedPointer<ResultItem>> results = m_parser->parseSearchResponse(data, entityType);
    auto pagination = ResponseParser::extractPagination(obj);
    
    qDebug() << "Search completed -" << results.size() << "results, total:" << pagination.first << "offset:" << pagination.second;
    
    emit searchResultsReady(results, pagination.first, pagination.second);
}

void MusicBrainzResponseHandler::handleDetailsResponse(const QByteArray& data, const QVariantMap& context)
{
    EntityType entityType = static_cast<EntityType>(context.value("entityType").toInt());
    
    auto detailedItem = m_parser->parseDetailsResponse(data, entityType);
    
    QVariantMap details;
    if (detailedItem) {
        details["id"] = detailedItem->getId();
        details["name"] = detailedItem->getName();
        details["type"] = EntityUtils::entityTypeToString(detailedItem->getType());
        details["disambiguation"] = detailedItem->getDisambiguation();
        
        const QVariantMap &detailData = detailedItem->getDetailData();
        for (auto it = detailData.constBegin(); it != detailData.constEnd(); ++it) {
            details[it.key()] = it.value();
        }
    }
    
    emit detailsReady(details, entityType);
}

void MusicBrainzResponseHandler::handleDiscIdResponse(const QByteArray& data, const QVariantMap& context)
{
    QString discId = context.value("discId").toString();
    
    QString error;
    QJsonObject obj = ResponseParser::parseJsonResponse(data, &error);
    if (!error.isEmpty()) {
        emit errorOccurred("Failed to parse DiscID response: " + error);
        return;
    }
    
    QList<QSharedPointer<ResultItem>> releases;
    if (obj.contains("disc") && obj["disc"].toObject().contains("release-list")) {        
        QJsonArray releaseArray = obj["disc"].toObject()["release-list"].toArray();
        for (const QJsonValue &value : releaseArray) {
            auto release = m_parser->parseEntity(value.toObject(), EntityType::Release);
            if (release) {
                releases.append(release);
            }
        }
    }
    
    emit discIdLookupReady(releases, discId);
}

void MusicBrainzResponseHandler::handleGenericResponse(const QByteArray& data, const QVariantMap& context)
{
    QString entity = context.value("entity").toString();
    QString id = context.value("id").toString();
    
    QString error;
    QJsonObject obj = ResponseParser::parseJsonResponse(data, &error);
    if (!error.isEmpty()) {
        emit errorOccurred("Failed to parse generic response: " + error);
        return;
    }
    
    QVariantMap result = obj.toVariantMap();
    emit genericQueryReady(result, entity, id);
}

void MusicBrainzResponseHandler::handleBrowseResponse(const QByteArray& data, const QVariantMap& context)
{
    QString entity = context.value("entity").toString();
    
    QString error;
    QJsonObject obj = ResponseParser::parseJsonResponse(data, &error);
    if (!error.isEmpty()) {
        emit errorOccurred("Failed to parse browse response: " + error);
        return;
    }
    
    EntityType entityType = EntityUtils::stringToEntityType(entity);
    QList<QSharedPointer<ResultItem>> results = m_parser->parseSearchResponse(data, entityType);
    auto pagination = ResponseParser::extractPagination(obj);
    
    emit browseResultsReady(results, entity, pagination.first, pagination.second);
}

void MusicBrainzResponseHandler::handleCollectionResponse(const QByteArray& data, const QVariantMap& context)
{
    QString operation = context.value("operation").toString();
    QString collectionId = context.value("collectionId").toString();
    
    QString error;
    QJsonObject obj = ResponseParser::parseJsonResponse(data, &error);
    if (!error.isEmpty()) {
        emit errorOccurred("Failed to parse collection response: " + error);
        return;
    }
    
    if (operation == "list") {
        // 用户集合列表
        QList<QVariantMap> collections = ResponseParser::parseCollectionList(obj);
        emit userCollectionsReady(collections);
    }
    else if (operation == "contents") {
        // 集合内容
        QList<QSharedPointer<ResultItem>> contents;
        if (obj.contains("release-list")) {
            QJsonArray releaseArray = obj["release-list"].toArray();            
            for (const QJsonValue &value : releaseArray) {
                auto release = m_parser->parseEntity(value.toObject(), EntityType::Release);
                if (release) {
                    contents.append(release);
                }
            }
        }
        emit collectionContentsReady(contents, collectionId);
    }
    else if (operation == "add" || operation == "remove") {
        // 集合修改操作
        int httpStatusCode = context.value("httpStatusCode", 0).toInt();
        bool success = (httpStatusCode >= 200 && httpStatusCode < 300);
        emit collectionModified(success, collectionId, operation);
    }
}
