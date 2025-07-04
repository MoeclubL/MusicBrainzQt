#include "musicbrainzapi.h"
#include "musicbrainzparser.h"
#include "musicbrainz_response_handler.h"
#include "network_manager.h"
#include "api_utils.h"
#include "../models/resultitem.h"
#include "../core/error_types.h"
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
    , m_responseHandler(new MusicBrainzResponseHandler(m_parser, this))
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
    
    // 连接响应处理器信号
    connect(m_responseHandler, &MusicBrainzResponseHandler::searchResultsReady,
            this, &MusicBrainzApi::searchResultsReady);
    connect(m_responseHandler, &MusicBrainzResponseHandler::detailsReady,
            this, &MusicBrainzApi::detailsReady);
    connect(m_responseHandler, &MusicBrainzResponseHandler::discIdLookupReady,
            this, &MusicBrainzApi::discIdLookupReady);
    connect(m_responseHandler, &MusicBrainzResponseHandler::genericQueryReady,
            this, &MusicBrainzApi::genericQueryReady);
    connect(m_responseHandler, &MusicBrainzResponseHandler::browseResultsReady,
            this, &MusicBrainzApi::browseResultsReady);
    connect(m_responseHandler, &MusicBrainzResponseHandler::userCollectionsReady,
            this, &MusicBrainzApi::userCollectionsReady);
    connect(m_responseHandler, &MusicBrainzResponseHandler::collectionContentsReady,
            this, &MusicBrainzApi::collectionContentsReady);
    connect(m_responseHandler, &MusicBrainzResponseHandler::collectionModified,
            this, &MusicBrainzApi::collectionModified);
    connect(m_responseHandler, &MusicBrainzResponseHandler::errorOccurred,
            this, &MusicBrainzApi::errorOccurred);
            
    qDebug() << "MusicBrainzApi initialized with optimized architecture";
}

MusicBrainzApi::~MusicBrainzApi()
{
    // NetworkManager会自动清理未完成的请求
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
        qCritical() << "MusicBrainzApi::getDetails - Invalid MBID:" << mbid;
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
    
    qDebug() << "MusicBrainzApi::getDetails - MBID:" << mbid << "Type:" << static_cast<int>(type);
    
    sendRequest(url, RequestType::Details, context);
}

void MusicBrainzApi::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent;
    qDebug() << "User-Agent set to:" << userAgent;
}

void MusicBrainzApi::setAuthentication(const QString &username, const QString &password)
{
    m_username = username;
    m_password = password;
    qDebug() << "Authentication set for user:" << username;
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
    qDebug() << "Proxy set to:" << host << ":" << port;
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
    
    qDebug() << "MusicBrainzApi::lookupDiscId - DiscID:" << discId;
    
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
    
    qDebug() << "MusicBrainzApi::genericQuery - Entity:" << entity << "ID:" << id;
    
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
    
    qDebug() << "MusicBrainzApi::browse - Entity:" << entity << "Related:" << relatedEntity;
    
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
    qDebug() << "MusicBrainzApi: Sending request -" << url;
    
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
    
    qDebug() << "MusicBrainzApi: Sending authenticated request -" << url;
    
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
    
    qWarning() << "Received response for unknown URL:" << url;
    reply->deleteLater();
}

void MusicBrainzApi::onRequestError(const QString &error, const QString &url)
{
    qCritical() << "MusicBrainzApi network error:" << error << "for URL:" << url;
    
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
        qCritical() << "HTTP Error" << m_lastHttpCode << ":" << m_lastErrorMessage;
        emit errorOccurred(m_lastErrorMessage);
        reply->deleteLater();
        return;
    }
    
    // 根据请求类型分发给响应处理器
    switch (request.type) {
        case RequestType::Search:
            m_responseHandler->handleSearchResponse(data, request.context);
            break;
        case RequestType::Details:
            m_responseHandler->handleDetailsResponse(data, request.context);
            break;
        case RequestType::DiscId:
            m_responseHandler->handleDiscIdResponse(data, request.context);
            break;
        case RequestType::Generic:
            m_responseHandler->handleGenericResponse(data, request.context);
            break;
        case RequestType::Browse:
            m_responseHandler->handleBrowseResponse(data, request.context);
            break;
        case RequestType::Collection:
            // 对于集合操作，需要传递HTTP状态码
            QVariantMap contextWithStatus = request.context;
            contextWithStatus["httpStatusCode"] = m_lastHttpCode;
            m_responseHandler->handleCollectionResponse(data, contextWithStatus);
            break;
    }
    
    reply->deleteLater();
}
