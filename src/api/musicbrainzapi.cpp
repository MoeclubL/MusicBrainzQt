#include "musicbrainzapi.h"
#include "musicbrainzparser.h"
#include "musicbrainz_utils.h"
#include "network_manager.h"
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
    , m_currentSearchType(EntityType::Unknown)
    , m_currentDetailsType(EntityType::Unknown)
{
    // 连接网络管理器信�?
    connect(m_networkManager, &NetworkManager::requestFinished,
            this, &MusicBrainzApi::onRequestFinished);
    connect(m_networkManager, &NetworkManager::requestError,
            this, &MusicBrainzApi::onRequestError);
            
    LOG_SERVICE_INFO("MusicBrainzApi initialized");
}

MusicBrainzApi::~MusicBrainzApi()
{
    // NetworkManager会自动清理未完成的请�?
}

void MusicBrainzApi::search(const QString &query, EntityType type, int limit, int offset)
{
    if (query.trimmed().isEmpty()) {
        emit errorOccurred("Empty search query");
        return;
    }
    
    QString url = buildSearchUrl(query, type, limit, offset);
    m_currentSearchType = type;
    m_pendingSearchUrls.append(url);
    
    LOG_SERVICE_DEBUG("MusicBrainzApi: Search request - %s", url.toUtf8().constData());
    
    m_networkManager->sendRequest(url, m_userAgent);
}

void MusicBrainzApi::getDetails(const QString &mbid, EntityType type)
{
    if (mbid.isEmpty()) {
        LOG_SERVICE_ERROR("MusicBrainzApi::getDetails - Empty MBID provided");
        emit errorOccurred("Invalid MBID: empty string");
        return;
    }
    
    m_currentDetailsType = type;
    QString url = buildDetailsUrl(mbid, type);
    m_pendingDetailsUrls.append(url);
    
    LOG_SERVICE_INFO("MusicBrainzApi::getDetails - MBID: %s, Type: %d, URL: %s", 
                     mbid.toUtf8().constData(), 
                     static_cast<int>(type),
                     url.toUtf8().constData());
    
    m_networkManager->sendRequest(url, m_userAgent);
}

void MusicBrainzApi::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent;
    LOG_SERVICE_DEBUG("User-Agent set to: %s", "MusicBrainzQt/1.0 ( https://github.com/MoeclubL/MusicBrainzQt )");
}

void MusicBrainzApi::setRateLimitDelay(int milliseconds)
{
    m_networkManager->setRateLimitDelay(milliseconds);
    LOG_SERVICE_DEBUG("Rate limit delay set to: %d ms", milliseconds);
}

QString MusicBrainzApi::buildSearchUrl(const QString &query, EntityType type, int limit, int offset) const
{
    QString baseUrl = "https://musicbrainz.org/ws/2/";
    QString entityStr = MusicBrainzUtils::entityTypeToString(type);
    
    if (entityStr.isEmpty() || entityStr == "unknown") {
        return QString();
    }
    
    QUrl url(baseUrl + entityStr);
    QUrlQuery queryParams;
    queryParams.addQueryItem("query", query);
    queryParams.addQueryItem("limit", QString::number(limit));
    queryParams.addQueryItem("offset", QString::number(offset));
    queryParams.addQueryItem("fmt", "json");
    
    url.setQuery(queryParams);
    return url.toString();
}

QString MusicBrainzApi::buildDetailsUrl(const QString &mbid, EntityType type) const
{
    QString baseUrl = "https://musicbrainz.org/ws/2/";
    QString entityStr = MusicBrainzUtils::entityTypeToString(type);
    
    if (entityStr.isEmpty() || entityStr == "unknown") {
        return QString();
    }
    
    QUrl url(baseUrl + entityStr + "/" + mbid);
    QUrlQuery queryParams;
    queryParams.addQueryItem("fmt", "json");
    queryParams.addQueryItem("inc", MusicBrainzUtils::getDefaultIncludes(type));
    
    url.setQuery(queryParams);    return url.toString();
}

void MusicBrainzApi::onRequestFinished(QNetworkReply *reply, const QString &url)
{
    processReply(reply, url);
}

void MusicBrainzApi::onRequestError(const QString &error, const QString &url)
{
    LOG_SERVICE_ERROR("MusicBrainzApi network error: %s for URL: %s", 
                      error.toUtf8().constData(), url.toUtf8().constData());
    emit errorOccurred(error);
}

void MusicBrainzApi::processReply(QNetworkReply *reply, const QString &url)
{
    QByteArray data = reply->readAll();
    
    // 检查是搜索请求还是详情请求
    bool isSearchRequest = m_pendingSearchUrls.contains(url);
    bool isDetailsRequest = m_pendingDetailsUrls.contains(url);
    
    if (isSearchRequest) {
        m_pendingSearchUrls.removeAll(url);
        
        QList<QSharedPointer<ResultItem>> results = m_parser->parseSearchResponse(data, m_currentSearchType);
        
        // 从响应中提取总数和偏移量
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        
        int totalCount = obj.value("count").toInt();
        int offset = obj.value("offset").toInt();
        
        LOG_SERVICE_DEBUG("MusicBrainzApi: Search completed - %d results, total: %d, offset: %d", 
                          results.size(), totalCount, offset);
        
        emit searchResultsReady(results, totalCount, offset);
    }
    else if (isDetailsRequest) {
        m_pendingDetailsUrls.removeAll(url);
        
        LOG_SERVICE_DEBUG("MusicBrainzApi::processReply - Details response size: %d bytes", data.size());
        
        // 使用通用解析器解析详细信�?
        auto detailedItem = m_parser->parseDetailsResponse(data, m_currentDetailsType);
        
        QVariantMap details;
        if (detailedItem) {
            // 将解析后的ResultItem转换为QVariantMap
            details["id"] = detailedItem->getId();
            details["name"] = detailedItem->getName();
            details["type"] = MusicBrainzUtils::entityTypeToString(detailedItem->getType());
            details["disambiguation"] = detailedItem->getDisambiguation();
            
            // 添加所有详细属�?
            const QVariantMap &detailData = detailedItem->getDetailData();
            for (auto it = detailData.constBegin(); it != detailData.constEnd(); ++it) {
                details[it.key()] = it.value();
            }
        }
        
        LOG_SERVICE_INFO("MusicBrainzApi::processReply - Parsed %d detail fields for type %d", 
                         details.size(), static_cast<int>(m_currentDetailsType));
        
        emit detailsReady(details, m_currentDetailsType);
    }
    
    reply->deleteLater();
}


