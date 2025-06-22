#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
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
    ~NetworkManager();    /**
     * @brief 发送GET请求
     * @param url 请求URL
     * @param userAgent User-Agent字符串
     * @return 网络请求对象
     */
    QNetworkReply* sendRequest(const QString &url, const QString &userAgent);
    
    /**
     * @brief 发送认证请求
     * @param url 请求URL
     * @param userAgent User-Agent字符串
     * @param username 用户名
     * @param password 密码
     * @param method HTTP方法（GET, POST, PUT, DELETE）
     * @param data 请求数据
     * @return 是否成功发送
     */
    bool sendAuthenticatedRequest(const QString &url, const QString &userAgent,
                                 const QString &username, const QString &password,
                                 const QString &method = "GET", const QByteArray &data = QByteArray());

    /**
     * @brief 设置代理服务器
     * @param host 代理主机地址
     * @param port 代理端口
     * @param username 代理用户名（可选）
     * @param password 代理密码（可选）
     */
    void setProxy(const QString &host, int port, 
                  const QString &username = QString(), 
                  const QString &password = QString());

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

private:
    QNetworkAccessManager *m_networkManager;
    
    QNetworkRequest createRequest(const QString &url, const QString &userAgent) const;
    QNetworkRequest createAuthenticatedRequest(const QString &url, const QString &userAgent,
                                              const QString &username, const QString &password) const;
};

#endif // NETWORK_MANAGER_H
