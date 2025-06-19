#ifndef LIBMUSICBRAINZAPI_H
#define LIBMUSICBRAINZAPI_H

#include <QObject>
#include <QSharedPointer>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "../core/types.h"

class ResultItem;

/**
 * libmusicbrainz API 适配器
 * 使用官方 libmusicbrainz C++ 库的接口
 * 目前先使用HTTP API实现，后续可选择使用libmusicbrainz库
 */
class LibMusicBrainzApi : public QObject
{
    Q_OBJECT

public:
    explicit LibMusicBrainzApi(QObject *parent = nullptr);
    ~LibMusicBrainzApi();

    // 搜索接口
    void search(const QString &query, EntityType type, int limit = 25, int offset = 0);
    
    // 详细信息获取
    void getDetails(const QString &mbid, EntityType type);
    
    // 设置User-Agent
    void setUserAgent(const QString &userAgent);
    
    // 设置API速率限制延迟
    void setRateLimitDelay(int milliseconds);

signals:
    // 搜索结果信号
    void searchResultsReady(const QList<QSharedPointer<ResultItem>> &results, 
                           int totalCount, int offset);
    
    // 详细信息结果信号
    void detailsReady(const QVariantMap &details, EntityType type);
    
    // 错误信号
    void errorOccurred(const QString &error);

private slots:
    void onSearchReplyFinished();
    void onDetailsReplyFinished();
    void onRateLimitTimerTimeout();

private:
    // 构建API URL
    QString buildSearchUrl(const QString &query, EntityType type, int limit, int offset) const;
    QString buildDetailsUrl(const QString &mbid, EntityType type) const;
    
    // 解析响应
    QList<QSharedPointer<ResultItem>> parseSearchResponse(const QByteArray &data, EntityType type) const;
    QVariantMap parseDetailsResponse(const QByteArray &data, EntityType type) const;
    
    // 实体类型到字符串的转换
    QString entityTypeToString(EntityType type) const;
    
    // 发送HTTP请求（带速率限制）
    void sendRequest(const QString &url, bool isDetailsRequest = false);
    
    // 速率限制相关
    void enforceRateLimit();
    
    QNetworkAccessManager *m_networkManager;
    QString m_userAgent;
    
    // 速率限制
    QTimer *m_rateLimitTimer;
    int m_rateLimitDelay;  // 毫秒
    QList<QPair<QString, bool>> m_pendingRequests; // URL, isDetailsRequest
    
    // 当前请求追踪
    QNetworkReply *m_currentSearchReply;
    QNetworkReply *m_currentDetailsReply;
    EntityType m_currentSearchType;
    EntityType m_currentDetailsType;
};

#endif // LIBMUSICBRAINZAPI_H
