#include "network_manager.h"
#include "../core/error_types.h"
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    // 移除速率限制 - 不再创建定时器
}

NetworkManager::~NetworkManager()
{
    // QObject会自动清理子对象
}

QNetworkReply* NetworkManager::sendRequest(const QString &url, const QString &userAgent)
{
    QNetworkRequest request = createRequest(url, userAgent);
    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished,
            this, &NetworkManager::onReplyFinished);
    
    qDebug() << "NetworkManager: Request sent -" << url;
    return reply;
}

QNetworkReply* NetworkManager::sendAuthenticatedRequest(const QString &url, const QString &userAgent,
                                            const QString &username, const QString &password,
                                            const QString &method, const QByteArray &data)
{
    QNetworkRequest request = createAuthenticatedRequest(url, userAgent, username, password);
    QNetworkReply *reply = nullptr;
    
    if (method == "GET") {
        reply = m_networkManager->get(request);
    } else if (method == "POST") {
        reply = m_networkManager->post(request, data);
    } else if (method == "PUT") {
        reply = m_networkManager->put(request, data);
    } else if (method == "DELETE") {
        reply = m_networkManager->deleteResource(request);
    } else {
        qCritical() << "Unsupported HTTP method:" << method;
        return nullptr;
    }
    
    if (reply) {
        connect(reply, &QNetworkReply::finished,
                this, &NetworkManager::onReplyFinished);
        qDebug() << "NetworkManager: Authenticated" << method << "request sent -" << url;
    }
    
    return reply;
}

void NetworkManager::setProxy(const QString &host, int port, 
                             const QString &username, const QString &password)
{
    if (host.isEmpty()) {
        m_networkManager->setProxy(QNetworkProxy::NoProxy);
        return;
    }
    
    QNetworkProxy proxy(QNetworkProxy::HttpProxy, host, port);
    if (!username.isEmpty()) {
        proxy.setUser(username);
        proxy.setPassword(password);
    }
    
    m_networkManager->setProxy(proxy);
    qDebug() << "Proxy configured:" << host << ":" << port;
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
        qCritical() << "NetworkManager error:" << error;
        emit requestError(error, url);
    } else {
        emit requestFinished(reply, url);
    }
}

QNetworkRequest NetworkManager::createRequest(const QString &url, const QString &userAgent) const
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);
    request.setRawHeader("Accept", "application/json");
    return request;
}

QNetworkRequest NetworkManager::createAuthenticatedRequest(const QString &url, const QString &userAgent,
                                                          const QString &username, const QString &password) const
{
    QNetworkRequest request = createRequest(url, userAgent);
    
    // 添加HTTP基础认证
    QString credentials = username + ":" + password;
    QByteArray encodedCredentials = credentials.toUtf8().toBase64();
    request.setRawHeader("Authorization", "Basic " + encodedCredentials);
    
    return request;
}
