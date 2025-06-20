#include "network_manager.h"
#include "../utils/logger.h"
#include <QNetworkRequest>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_rateLimitTimer(new QTimer(this))
    , m_rateLimitDelay(500) // 默认速率限制延迟为500毫秒
{
    m_rateLimitTimer->setSingleShot(true);
    connect(m_rateLimitTimer, &QTimer::timeout,
            this, &NetworkManager::onRateLimitTimerTimeout);
}

NetworkManager::~NetworkManager()
{
    // QObject会自动清理子对象
}

QNetworkReply* NetworkManager::sendRequest(const QString &url, const QString &userAgent)
{
    // 检查速率限制
    if (isRateLimited()) {
        // 添加到待处理队列
        m_pendingRequests.enqueue(qMakePair(url, userAgent));
        LOG_SERVICE_DEBUG("Request queued due to rate limiting: %s", url.toUtf8().constData());
        return nullptr;
    }

    QNetworkRequest request = createRequest(url, userAgent);
    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished,
            this, &NetworkManager::onReplyFinished);

    // 启动速率限制定时器
    enforceRateLimit();
    
    LOG_SERVICE_DEBUG("NetworkManager: Request sent - %s", url.toUtf8().constData());
    return reply;
}

void NetworkManager::setRateLimitDelay(int milliseconds)
{
    m_rateLimitDelay = qMax(500, milliseconds); // 最小500ms
    LOG_SERVICE_DEBUG("Rate limit delay set to: %d ms", m_rateLimitDelay);
}

int NetworkManager::getRateLimitDelay() const
{
    return m_rateLimitDelay;
}

bool NetworkManager::isRateLimited() const
{
    return m_rateLimitTimer->isActive();
}

void NetworkManager::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    QString url = reply->request().url().toString();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString error = QString("Network request failed: %1 (HTTP %2)")
                           .arg(reply->errorString())
                           .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
        LOG_SERVICE_ERROR("NetworkManager error: %s", error.toUtf8().constData());
        emit requestError(error, url);
    } else {
        emit requestFinished(reply, url);
    }
}

void NetworkManager::onRateLimitTimerTimeout()
{
    // 处理待发送的请求
    if (!m_pendingRequests.isEmpty()) {
        auto request = m_pendingRequests.dequeue();
        sendRequest(request.first, request.second);
    }
}

void NetworkManager::enforceRateLimit()
{
    m_rateLimitTimer->start(m_rateLimitDelay);
}

QNetworkRequest NetworkManager::createRequest(const QString &url, const QString &userAgent) const
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);
    request.setRawHeader("Accept", "application/json");
    return request;
}
