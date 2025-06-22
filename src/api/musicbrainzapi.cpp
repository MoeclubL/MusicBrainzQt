#include "musicbrainzapi.h"
#include "musicbrainzparser.h"
#include "network_manager.h"
#include "api_utils.h"
#include "../models/resultitem.h"
#include "../utils/logger.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QUrl>
#include <QDebug>

MusicBrainzApi::MusicBrainzApi(QObject *parent)
    : QObject(parent)
    , m_networkManager(new NetworkManager(this))
    , m_parser(new MusicBrainzParser(this))
    , m_userAgent("MusicBrainzQt/1.0 (https://github.com/MoeclubL/MusicBrainzQt)")
    , m_proxyPort(0)
    , m_lastHttpCode(0)
    , m_version("1.0.0")
{
    // 连接网络管理器信号
    connect(m_networkManager, &NetworkManager::requestFinished,
            this, &MusicBrainzApi::onRequestFinished);
    connect(m_networkManager, &NetworkManager::requestError,
            this, &MusicBrainzApi::onRequestError);
            
    LOG_SERVICE_INFO("MusicBrainzApi initialized with optimized architecture");
}

MusicBrainzApi::~MusicBrainzApi()
{
    // NetworkManager会自动清理未完成的请�?
}

void MusicBrainzApi::search(const QString &query, EntityType type, int limit, int offset)
{
    // 使用Validator进行参数验证
    QString cleanedQuery = Validator::validateAndCleanQuery(query);
    if (cleanedQuery.isEmpty()) {
        emit errorOccurred("Empty search query");
        return;
    }
    
    auto pagination = Validator::validatePagination(limit, offset);
    
    // 使用UrlBuilder构建URL
    QString url = UrlBuilder::buildSearchUrl(cleanedQuery, type, pagination.first, pagination.second);
    if (url.isEmpty()) {
        emit errorOccurred("Invalid entity type for search");
        return;
    }
    
    // 创建请求上下文
    QVariantMap context;
    context["entityType"] = static_cast<int>(type);
    context["originalQuery"] = query;
    context["limit"] = pagination.first;
    context["offset"] = pagination.second;
    
    sendRequest(url, RequestType::Search, context);
}

void MusicBrainzApi::getDetails(const QString &mbid, EntityType type)
{
    // 使用Validator验证MBID
    if (!Validator::isValidMbid(mbid)) {
        LOG_SERVICE_ERROR("MusicBrainzApi::getDetails - Invalid MBID: %s", mbid.toUtf8().constData());
        emit errorOccurred("Invalid MBID format");
        return;
    }
    
    // 使用UrlBuilder构建URL
    QString url = UrlBuilder::buildDetailsUrl(mbid, type);
    if (url.isEmpty()) {
        emit errorOccurred("Invalid entity type for details");
        return;
    }
    
    // 创建请求上下文
    QVariantMap context;
    context["mbid"] = mbid;
    context["entityType"] = static_cast<int>(type);
    
    LOG_SERVICE_INFO("MusicBrainzApi::getDetails - MBID: %s, Type: %d", 
                     mbid.toUtf8().constData(), static_cast<int>(type));
    
    sendRequest(url, RequestType::Details, context);
}

void MusicBrainzApi::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent;
    LOG_SERVICE_DEBUG("User-Agent set to: %s", userAgent.toUtf8().constData());
}

void MusicBrainzApi::setAuthentication(const QString &username, const QString &password)
{
    m_username = username;
    m_password = password;
    LOG_SERVICE_DEBUG("Authentication set for user: %s", username.toUtf8().constData());
}

void MusicBrainzApi::setProxy(const QString &host, int port, 
                             const QString &username, const QString &password)
{
    m_proxyHost = host;
    m_proxyPort = port;
    m_proxyUsername = username;
    m_proxyPassword = password;
    
    // 配置NetworkManager的代理设置
    m_networkManager->setProxy(host, port, username, password);
    LOG_SERVICE_DEBUG("Proxy set to: %s:%d", host.toUtf8().constData(), port);
}

// =============================================================================
// 扩展API方法实现
// =============================================================================

void MusicBrainzApi::lookupDiscId(const QString &discId)
{
    if (discId.isEmpty()) {
        emit errorOccurred("Empty DiscID provided");
        return;
    }
    
    QString url = UrlBuilder::buildDiscIdUrl(discId);
    
    QVariantMap context;
    context["discId"] = discId;
    
    LOG_SERVICE_INFO("MusicBrainzApi::lookupDiscId - DiscID: %s", discId.toUtf8().constData());
    
    sendRequest(url, RequestType::DiscId, context);
}

void MusicBrainzApi::genericQuery(const QString &entity, const QString &id,
                                 const QString &resource, const QStringList &includes,
                                 const QVariantMap &params)
{
    if (!Validator::isValidEntity(entity)) {
        emit errorOccurred("Invalid entity type provided");
        return;
    }
    
    QString url = UrlBuilder::buildGenericUrl(entity, id, resource, includes, params);
    
    QVariantMap context;
    context["entity"] = entity;
    context["id"] = id;
    context["resource"] = resource;
    
    LOG_SERVICE_INFO("MusicBrainzApi::genericQuery - Entity: %s, ID: %s", 
                     entity.toUtf8().constData(), id.toUtf8().constData());
    
    sendRequest(url, RequestType::Generic, context);
}

void MusicBrainzApi::browse(const QString &entity, const QString &relatedEntity,
                           const QString &relatedId, int limit, int offset)
{
    if (!Validator::isValidEntity(entity) || relatedEntity.isEmpty() || relatedId.isEmpty()) {
        emit errorOccurred("Invalid browse parameters");
        return;
    }
    
    auto pagination = Validator::validatePagination(limit, offset);
    QString url = UrlBuilder::buildBrowseUrl(entity, relatedEntity, relatedId, 
                                            pagination.first, pagination.second);
    
    QVariantMap context;
    context["entity"] = entity;
    context["relatedEntity"] = relatedEntity;
    context["relatedId"] = relatedId;
    context["limit"] = pagination.first;
    context["offset"] = pagination.second;
    
    LOG_SERVICE_INFO("MusicBrainzApi::browse - Entity: %s, Related: %s", 
                     entity.toUtf8().constData(), relatedEntity.toUtf8().constData());
    
    sendRequest(url, RequestType::Browse, context);
}

void MusicBrainzApi::getUserCollections()
{
    if (m_username.isEmpty()) {
        emit errorOccurred("Authentication required for collection access");
        return;
    }
    
    QString url = UrlBuilder::buildCollectionUrl();
    
    QVariantMap context;
    context["operation"] = "list";
    
    sendAuthenticatedRequest(url, "GET", QByteArray(), context);
}

void MusicBrainzApi::getCollectionContents(const QString &collectionId)
{
    if (collectionId.isEmpty()) {
        emit errorOccurred("Empty collection ID provided");
        return;
    }
    
    QString url = UrlBuilder::buildCollectionUrl(collectionId, "releases");
    
    QVariantMap context;
    context["collectionId"] = collectionId;
    context["operation"] = "contents";
    
    sendRequest(url, RequestType::Collection, context);
}

void MusicBrainzApi::addToCollection(const QString &collectionId, const QStringList &releaseIds)
{
    if (m_username.isEmpty()) {
        emit errorOccurred("Authentication required for collection modification");
        return;
    }
    
    if (collectionId.isEmpty() || releaseIds.isEmpty()) {
        emit errorOccurred("Invalid collection modification parameters");
        return;
    }
    
    QString url = UrlBuilder::buildCollectionUrl(collectionId, "releases/" + releaseIds.join(";"));
    
    QVariantMap context;
    context["collectionId"] = collectionId;
    context["operation"] = "add";
    context["releaseIds"] = releaseIds;
    
    sendAuthenticatedRequest(url, "PUT", QByteArray(), context);
}

void MusicBrainzApi::removeFromCollection(const QString &collectionId, const QStringList &releaseIds)
{
    if (m_username.isEmpty()) {
        emit errorOccurred("Authentication required for collection modification");
        return;
    }
    
    if (collectionId.isEmpty() || releaseIds.isEmpty()) {
        emit errorOccurred("Invalid collection modification parameters");
        return;
    }
    
    QString url = UrlBuilder::buildCollectionUrl(collectionId, "releases/" + releaseIds.join(";"));
    
    QVariantMap context;
    context["collectionId"] = collectionId;
    context["operation"] = "remove";
    context["releaseIds"] = releaseIds;
    
    sendAuthenticatedRequest(url, "DELETE", QByteArray(), context);
}

// =============================================================================
// 状态查询方法
// =============================================================================

int MusicBrainzApi::getLastHttpCode() const
{
    return m_lastHttpCode;
}

QString MusicBrainzApi::getLastErrorMessage() const
{
    return m_lastErrorMessage;
}

QString MusicBrainzApi::getVersion() const
{
    return m_version;
}

// =============================================================================
// 统一请求处理架构
// =============================================================================

void MusicBrainzApi::sendRequest(const QString& url, RequestType type, const QVariantMap& context)
{
    LOG_SERVICE_DEBUG("MusicBrainzApi: Sending request - %s", url.toUtf8().constData());
    
    // 将请求加入队列
    PendingRequest request(url, type, context);
    m_pendingRequests.enqueue(request);
    
    // 发送网络请求
    m_networkManager->sendRequest(url, m_userAgent);
}

bool MusicBrainzApi::sendAuthenticatedRequest(const QString &url, const QString &method, 
                                             const QByteArray &data, const QVariantMap& context)
{
    if (m_username.isEmpty() || m_password.isEmpty()) {
        emit errorOccurred("Authentication credentials not set");
        return false;
    }
    
    LOG_SERVICE_DEBUG("MusicBrainzApi: Sending authenticated request - %s", url.toUtf8().constData());
    
    // 将请求加入队列
    PendingRequest request(url, RequestType::Collection, context);
    m_pendingRequests.enqueue(request);
    
    // 使用NetworkManager发送认证请求
    return m_networkManager->sendAuthenticatedRequest(url, m_userAgent, m_username, m_password, method, data);
}

void MusicBrainzApi::onRequestFinished(QNetworkReply *reply, const QString &url)
{
    // 查找对应的请求
    for (int i = 0; i < m_pendingRequests.size(); ++i) {
        if (m_pendingRequests[i].url == url) {
            PendingRequest request = m_pendingRequests[i];
            m_pendingRequests.removeAt(i);
            processResponse(reply, request);
            return;
        }
    }
    
    LOG_SERVICE_WARNING("Received response for unknown URL: %s", url.toUtf8().constData());
    reply->deleteLater();
}

void MusicBrainzApi::onRequestError(const QString &error, const QString &url)
{
    LOG_SERVICE_ERROR("MusicBrainzApi network error: %s for URL: %s", 
                      error.toUtf8().constData(), url.toUtf8().constData());
    
    // 从队列中移除失败的请求
    for (int i = 0; i < m_pendingRequests.size(); ++i) {
        if (m_pendingRequests[i].url == url) {
            m_pendingRequests.removeAt(i);
            break;
        }
    }
    
    emit errorOccurred(error);
}

void MusicBrainzApi::processResponse(QNetworkReply* reply, const PendingRequest& request)
{
    QByteArray data = reply->readAll();
    m_lastHttpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    // 检查HTTP错误
    if (reply->error() != QNetworkReply::NoError) {
        m_lastErrorMessage = reply->errorString();
        LOG_SERVICE_ERROR("HTTP Error %d: %s", m_lastHttpCode, m_lastErrorMessage.toUtf8().constData());
        emit errorOccurred(m_lastErrorMessage);
        reply->deleteLater();
        return;
    }
    
    // 根据请求类型分发处理
    switch (request.type) {
        case RequestType::Search:
            handleSearchResponse(data, request.context);
            break;
        case RequestType::Details:
            handleDetailsResponse(data, request.context);
            break;
        case RequestType::DiscId:
            handleDiscIdResponse(data, request.context);
            break;
        case RequestType::Generic:
            handleGenericResponse(data, request.context);
            break;
        case RequestType::Browse:
            handleBrowseResponse(data, request.context);
            break;
        case RequestType::Collection:
            handleCollectionResponse(data, request.context);
            break;
    }
    
    reply->deleteLater();
}

// =============================================================================
// 响应处理分发器
// =============================================================================

void MusicBrainzApi::handleSearchResponse(const QByteArray& data, const QVariantMap& context)
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
    
    LOG_SERVICE_DEBUG("Search completed - %d results, total: %d, offset: %d", 
                      results.size(), pagination.first, pagination.second);
    
    emit searchResultsReady(results, pagination.first, pagination.second);
}

void MusicBrainzApi::handleDetailsResponse(const QByteArray& data, const QVariantMap& context)
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

void MusicBrainzApi::handleDiscIdResponse(const QByteArray& data, const QVariantMap& context)
{
    QString discId = context.value("discId").toString();
    
    QString error;
    QJsonObject obj = ResponseParser::parseJsonResponse(data, &error);
    if (!error.isEmpty()) {
        emit errorOccurred("Failed to parse DiscID response: " + error);
        return;
    }
    
    QList<QSharedPointer<ResultItem>> releases;
    if (obj.contains("disc") && obj["disc"].toObject().contains("release-list")) {        QJsonArray releaseArray = obj["disc"].toObject()["release-list"].toArray();
        for (const QJsonValue &value : releaseArray) {
            auto release = m_parser->parseEntity(value.toObject(), EntityType::Release);
            if (release) {
                releases.append(release);
            }
        }
    }
    
    emit discIdLookupReady(releases, discId);
}

void MusicBrainzApi::handleGenericResponse(const QByteArray& data, const QVariantMap& context)
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

void MusicBrainzApi::handleBrowseResponse(const QByteArray& data, const QVariantMap& context)
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

void MusicBrainzApi::handleCollectionResponse(const QByteArray& data, const QVariantMap& context)
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
            QJsonArray releaseArray = obj["release-list"].toArray();            for (const QJsonValue &value : releaseArray) {
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
        bool success = (m_lastHttpCode >= 200 && m_lastHttpCode < 300);
        emit collectionModified(success, collectionId, operation);
    }
}

