#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QQueue>
#include <QPair>

/**
 * @class NetworkManager
 * @brief 专用的网络请求管理器
 * 
 * 从MusicBrainzApi中分离出来的网络层，专门负责：
 * - HTTP请求的发送和接收
 * - 速率限制管理
 * - 请求队列管理
 * - 网络错误处理
 */
class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    /**
     * @brief 发送GET请求
     * @param url 请求URL
     * @param userAgent User-Agent字符串
     * @return 网络请求对象
     */
    QNetworkReply* sendRequest(const QString &url, const QString &userAgent);

    /**
     * @brief 设置速率限制延迟
     * @param milliseconds 延迟毫秒数
     */
    void setRateLimitDelay(int milliseconds);

    /**
     * @brief 获取当前速率限制延迟
     */
    int getRateLimitDelay() const;

    /**
     * @brief 检查是否在速率限制期内
     */
    bool isRateLimited() const;

signals:
    /**
     * @brief 请求完成信号
     * @param reply 网络回复对象
     * @param url 原始请求URL
     */
    void requestFinished(QNetworkReply *reply, const QString &url);

    /**
     * @brief 请求错误信号
     * @param error 错误信息
     * @param url 原始请求URL
     */
    void requestError(const QString &error, const QString &url);

private slots:
    void onReplyFinished();
    void onRateLimitTimerTimeout();

private:
    QNetworkAccessManager *m_networkManager;
    QTimer *m_rateLimitTimer;
    int m_rateLimitDelay;
    QQueue<QPair<QString, QString>> m_pendingRequests; // URL, User-Agent

    void enforceRateLimit();
    QNetworkRequest createRequest(const QString &url, const QString &userAgent) const;
};

#endif // NETWORK_MANAGER_H
