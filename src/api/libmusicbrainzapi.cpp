#include "libmusicbrainzapi.h"
#include "../models/resultitem.h"
#include "../utils/logger.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QUrl>
#include <QDebug>

LibMusicBrainzApi::LibMusicBrainzApi(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_userAgent("MusicBrainzQt/1.0 (https://github.com/example/musicbrainzqt)")
    , m_rateLimitTimer(new QTimer(this))
    , m_rateLimitDelay(1000) // 1秒延迟，符合MusicBrainz API要求
    , m_currentSearchReply(nullptr)
    , m_currentDetailsReply(nullptr)
    , m_currentSearchType(EntityType::Unknown)
    , m_currentDetailsType(EntityType::Unknown)
{
    // 配置速率限制定时器
    m_rateLimitTimer->setSingleShot(true);
    connect(m_rateLimitTimer, &QTimer::timeout,
            this, &LibMusicBrainzApi::onRateLimitTimerTimeout);
            
    LOG_SERVICE_INFO("LibMusicBrainzApi initialized with rate limit: %d ms", m_rateLimitDelay);
}

LibMusicBrainzApi::~LibMusicBrainzApi()
{
    if (m_currentSearchReply) {
        m_currentSearchReply->abort();
    }
    if (m_currentDetailsReply) {
        m_currentDetailsReply->abort();
    }
}

void LibMusicBrainzApi::search(const QString &query, EntityType type, int limit, int offset)
{
    if (query.trimmed().isEmpty()) {
        emit errorOccurred("Empty search query");
        return;
    }
    
    QString url = buildSearchUrl(query, type, limit, offset);
    m_currentSearchType = type;
    
    LOG_SERVICE_DEBUG("LibMusicBrainzApi: Search request - %s", url.toUtf8().constData());
    
    sendRequest(url, false);
}

void LibMusicBrainzApi::getDetails(const QString &mbid, EntityType type)
{
    if (mbid.isEmpty()) {
        LOG_SERVICE_ERROR("LibMusicBrainzApi::getDetails - Empty MBID provided");
        emit errorOccurred("Invalid MBID: empty string");
        return;
    }
    
    m_currentDetailsType = type;
    QString url = buildDetailsUrl(mbid, type);
    
    LOG_SERVICE_INFO("LibMusicBrainzApi::getDetails - MBID: %s, Type: %d, URL: %s", 
                     mbid.toUtf8().constData(), 
                     static_cast<int>(type),
                     url.toUtf8().constData());
    
    sendRequest(url, true);
}

void LibMusicBrainzApi::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent;
    LOG_SERVICE_DEBUG("User-Agent set to: %s", "MusicBrainzQt/1.0 ( https://github.com/MoeclubL/MusicBrainzQt )");
}

void LibMusicBrainzApi::setRateLimitDelay(int milliseconds)
{
    m_rateLimitDelay = qMax(500, milliseconds); // 最小500ms
    LOG_SERVICE_DEBUG("Rate limit delay set to: %d ms", m_rateLimitDelay);
}

QString LibMusicBrainzApi::buildSearchUrl(const QString &query, EntityType type, int limit, int offset) const
{
    QString baseUrl = "https://musicbrainz.org/ws/2/";
    QString entityStr = entityTypeToString(type);
    
    if (entityStr.isEmpty()) {
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

QString LibMusicBrainzApi::buildDetailsUrl(const QString &mbid, EntityType type) const
{
    QString baseUrl = "https://musicbrainz.org/ws/2/";
    QString entityStr = entityTypeToString(type);
    
    if (entityStr.isEmpty()) {
        return QString();
    }
    
    QUrl url(baseUrl + entityStr + "/" + mbid);
    QUrlQuery queryParams;
    queryParams.addQueryItem("fmt", "json");
      // 根据实体类型添加相关子查询
    switch (type) {
        case EntityType::Artist:
            queryParams.addQueryItem("inc", "aliases+tags+ratings+genres+recordings+releases+release-groups+works");
            break;        case EntityType::Release:
            queryParams.addQueryItem("inc", "aliases+tags+ratings+genres+artists+labels+recordings+release-groups");
            break;
        case EntityType::Recording:
            queryParams.addQueryItem("inc", "aliases+tags+ratings+genres+artists+releases+release-groups");
            break;
        case EntityType::ReleaseGroup:
            queryParams.addQueryItem("inc", "aliases+tags+ratings+genres+artists+releases");
            break;
        default:
            queryParams.addQueryItem("inc", "tags+ratings+genres");
            break;
    }
    
    url.setQuery(queryParams);
    return url.toString();
}

QString LibMusicBrainzApi::entityTypeToString(EntityType type) const
{
    switch (type) {
        case EntityType::Artist: return "artist";
        case EntityType::Release: return "release";
        case EntityType::Recording: return "recording";
        case EntityType::ReleaseGroup: return "release-group";
        case EntityType::Label: return "label";
        case EntityType::Work: return "work";
        case EntityType::Area: return "area";
        case EntityType::Place: return "place";
        case EntityType::Annotation: return "annotation";
        case EntityType::CDStub: return "cdstub";
        case EntityType::Editor: return "editor";
        case EntityType::Tag: return "tag";
        case EntityType::Instrument: return "instrument";
        case EntityType::Series: return "series";
        case EntityType::Event: return "event";
        case EntityType::Documentation: return "documentation";
        default: return QString();
    }
}

void LibMusicBrainzApi::sendRequest(const QString &url, bool isDetailsRequest)
{
    // 检查速率限制
    if (m_rateLimitTimer->isActive()) {
        // 添加到待处理队列
        m_pendingRequests.append(qMakePair(url, isDetailsRequest));
        LOG_SERVICE_DEBUG("Request queued due to rate limiting: %s", url.toUtf8().constData());
        return;
    }
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    request.setRawHeader("Accept", "application/json");
    
    QNetworkReply *reply = m_networkManager->get(request);
    
    if (isDetailsRequest) {
        if (m_currentDetailsReply) {
            m_currentDetailsReply->abort();
        }
        m_currentDetailsReply = reply;
        connect(reply, &QNetworkReply::finished,
                this, &LibMusicBrainzApi::onDetailsReplyFinished);
    } else {
        if (m_currentSearchReply) {
            m_currentSearchReply->abort();
        }
        m_currentSearchReply = reply;
        connect(reply, &QNetworkReply::finished,
                this, &LibMusicBrainzApi::onSearchReplyFinished);
    }
    
    // 启动速率限制定时器
    enforceRateLimit();
}

void LibMusicBrainzApi::enforceRateLimit()
{
    m_rateLimitTimer->start(m_rateLimitDelay);
}

void LibMusicBrainzApi::onRateLimitTimerTimeout()
{
    // 处理待发送的请求
    if (!m_pendingRequests.isEmpty()) {
        auto request = m_pendingRequests.takeFirst();
        sendRequest(request.first, request.second);
    }
}

void LibMusicBrainzApi::onSearchReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || reply != m_currentSearchReply) {
        return;
    }
    
    m_currentSearchReply = nullptr;
    
    if (reply->error() != QNetworkReply::NoError) {
        QString error = QString("Search request failed: %1").arg(reply->errorString());
        LOG_SERVICE_ERROR("LibMusicBrainzApi search error: %s", error.toUtf8().constData());
        emit errorOccurred(error);
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    QList<QSharedPointer<ResultItem>> results = parseSearchResponse(data, m_currentSearchType);
    
    // 从响应中提取总数和偏移量
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    int totalCount = obj.value("count").toInt();
    int offset = obj.value("offset").toInt();
    
    LOG_SERVICE_DEBUG("LibMusicBrainzApi: Search completed - %d results, total: %d, offset: %d", 
                      results.size(), totalCount, offset);
    
    emit searchResultsReady(results, totalCount, offset);
    reply->deleteLater();
}

void LibMusicBrainzApi::onDetailsReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || reply != m_currentDetailsReply) {
        LOG_SERVICE_WARNING("LibMusicBrainzApi::onDetailsReplyFinished - Invalid or unexpected reply");
        return;
    }
    
    m_currentDetailsReply = nullptr;
    
    LOG_SERVICE_INFO("LibMusicBrainzApi::onDetailsReplyFinished - HTTP Status: %d, Error: %d", 
                     reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                     static_cast<int>(reply->error()));
    
    if (reply->error() != QNetworkReply::NoError) {
        QString error = QString("Details request failed: %1 (HTTP %2)")
                           .arg(reply->errorString())
                           .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
        LOG_SERVICE_ERROR("LibMusicBrainzApi details error: %s", error.toUtf8().constData());
        emit errorOccurred(error);
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    LOG_SERVICE_DEBUG("LibMusicBrainzApi::onDetailsReplyFinished - Response size: %d bytes", data.size());
    
    QVariantMap details = parseDetailsResponse(data, m_currentDetailsType);
    
    LOG_SERVICE_INFO("LibMusicBrainzApi::onDetailsReplyFinished - Parsed %d detail fields for type %d", 
                     details.size(), static_cast<int>(m_currentDetailsType));
    
    emit detailsReady(details, m_currentDetailsType);
    reply->deleteLater();
}

QList<QSharedPointer<ResultItem>> LibMusicBrainzApi::parseSearchResponse(const QByteArray &data, EntityType type) const
{
    QList<QSharedPointer<ResultItem>> results;
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return results;
    }
    
    QJsonObject root = doc.object();
    QString entityPlural = entityTypeToString(type) + "s";
    
    QJsonArray items = root.value(entityPlural).toArray();
      for (const auto &value : items) {
        QJsonObject item = value.toObject();
        
        QString id = item.value("id").toString();
        QString name = item.value("name").toString();
        auto resultItem = QSharedPointer<ResultItem>::create(id, name, type);
        
        // 通用字段
        resultItem->setDisambiguation(item.value("disambiguation").toString());
        
        // 根据类型设置特定字段
        switch (type) {
            case EntityType::Artist: {
                resultItem->setDetailProperty("sort-name", item.value("sort-name").toString());
                resultItem->setDetailProperty("type", item.value("type").toString());
                
                QJsonObject lifeSpan = item.value("life-span").toObject();
                if (!lifeSpan.isEmpty()) {
                    resultItem->setDetailProperty("begin", lifeSpan.value("begin").toString());
                    resultItem->setDetailProperty("end", lifeSpan.value("end").toString());
                }
                
                QJsonObject area = item.value("area").toObject();
                if (!area.isEmpty()) {
                    resultItem->setDetailProperty("area", area.value("name").toString());
                }
                break;
            }
            case EntityType::Release: {
                resultItem->setDetailProperty("status", item.value("status").toString());
                resultItem->setDetailProperty("date", item.value("date").toString());
                resultItem->setDetailProperty("country", item.value("country").toString());
                resultItem->setDetailProperty("barcode", item.value("barcode").toString());
                break;
            }
            case EntityType::Recording: {
                resultItem->setDetailProperty("length", item.value("length").toInt());
                break;
            }
            default:
                break;
        }
        
        // 评分
        if (item.contains("score")) {
            resultItem->setScore(item.value("score").toInt());
        }
        
        results.append(resultItem);
    }
    
    return results;
}

QVariantMap LibMusicBrainzApi::parseDetailsResponse(const QByteArray &data, EntityType type) const
{
    QVariantMap details;
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return details;
    }
    
    QJsonObject root = doc.object();
    
    // 递归转换JSON对象为QVariantMap
    std::function<QVariant(const QJsonValue&)> jsonToVariant = [&](const QJsonValue &value) -> QVariant {
        if (value.isObject()) {
            QVariantMap map;
            QJsonObject obj = value.toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                map[it.key()] = jsonToVariant(it.value());
            }
            return map;
        } else if (value.isArray()) {
            QVariantList list;
            QJsonArray arr = value.toArray();
            for (const auto &item : arr) {
                list.append(jsonToVariant(item));
            }
            return list;
        } else {
            return value.toVariant();
        }
    };
    
    for (auto it = root.begin(); it != root.end(); ++it) {
        details[it.key()] = jsonToVariant(it.value());
    }
    
    return details;
}
