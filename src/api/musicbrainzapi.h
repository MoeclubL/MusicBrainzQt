#ifndef MUSICBRAINZAPI_H
#define MUSICBRAINZAPI_H

#include <QObject>
#include <QSharedPointer>
#include <QVariantMap>
#include "../core/types.h"

class ResultItem;
class NetworkManager;
class QNetworkReply;

/**
 * @brief MusicBrainz Web Service API接口实现
 * 
 * MusicBrainzApi是MusicBrainzQt与MusicBrainz Web Service API的主要接口。
 * 它封装了所有API调用，处理HTTP请求、响应解析、错误处理和速率限制。
 * 
 * 核心功能：
 * - 实体搜索：支持艺术家、专辑、录音、作品等所有实体类型的搜索
 * - 详细信息获取：获取实体的完整信息，包括关系、别名、标签等
 * - 速率限制：严格遵守MusicBrainz的1请求/秒限制
 * - 错误处理：提供详细的错误信息和恢复机制
 * - 异步操作：所有操作都是非阻塞的，通过信号返回结果
 * 
 * API规范遵循：
 * - MusicBrainz Web Service v2
 * - JSON响应格式
 * - 标准HTTP状态码
 * - 适当的User-Agent标识，例如：MusicBrainzQt/1.0 ( https://github.com/MoeclubL/MusicBrainzQt )
 * 
 * 设计模式：
 * - 外观模式：为复杂的API提供简单接口
 * - 异步模式：使用信号槽处理异步响应
 * - 单例特征：通常每个应用实例只需一个API对象
 * 
 * @see https://musicbrainz.org/doc/MusicBrainz_API MusicBrainz API文档
 * @see ResultItem 实体数据容器
 */
class MusicBrainzApi : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MusicBrainzApi(QObject *parent = nullptr);
      /**
     * @brief 析构函数
     */
    ~MusicBrainzApi();

    // =============================================================================
    // 核心API方法
    // =============================================================================
    
    /**
     * @brief 搜索MusicBrainz实体
     * @param query 搜索查询字符串
     * @param type 实体类型（艺术家、专辑等）
     * @param limit 返回结果数量限制（默认25）
     * @param offset 结果偏移量，用于分页（默认0）
     * 
     * 异步方法，结果通过searchResultsReady信号返回。
     * 支持Lucene查询语法进行复杂搜索。
     */
    void search(const QString &query, EntityType type, int limit = 25, int offset = 0);
    
    /**
     * @brief 获取实体详细信息
     * @param mbid MusicBrainz ID（UUID格式）
     * @param type 实体类型
     * 
     * 异步方法，获取包含关系、别名、标签等的完整实体信息。
     * 结果通过detailsReady信号返回。
     */
    void getDetails(const QString &mbid, EntityType type);
    
    // =============================================================================
    // 配置方法
    // =============================================================================
    
    /**
     * @brief 设置User-Agent字符串
     * @param userAgent 用户代理字符串
     * 
     * MusicBrainz要求所有客户端提供适当的User-Agent标识。
     * 格式：应用名/版本 (联系信息)
     */
    void setUserAgent(const QString &userAgent);
    
    /**
     * @brief 设置速率限制延迟
     * @param milliseconds 请求间延迟毫秒数
     * 
     * MusicBrainz要求客户端遵守1请求/秒的速率限制。
     * 默认值为1000毫秒，不建议设置更小的值。
     */
    void setRateLimitDelay(int milliseconds);

signals:
    // =============================================================================
    // 信号定义
    // 所有API操作都是异步的，结果通过信号返回
    // =============================================================================
    
    /**
     * @brief 搜索结果就绪信号
     * @param results 搜索结果列表
     * @param totalCount 总结果数（用于分页）
     * @param offset 当前结果偏移量
     */
    void searchResultsReady(const QList<QSharedPointer<ResultItem>> &results, 
                           int totalCount, int offset);
    
    /**
     * @brief 详细信息就绪信号
     * @param details 实体详细信息（原始JSON数据）
     * @param type 实体类型
     */
    void detailsReady(const QVariantMap &details, EntityType type);
    
    /**
     * @brief 错误发生信号
     * @param error 错误描述信息
     */
    void errorOccurred(const QString &error);

private slots:
    void onRequestFinished(QNetworkReply *reply, const QString &url);
    void onRequestError(const QString &error, const QString &url);

private:    // 构建API URL
    QString buildSearchUrl(const QString &query, EntityType type, int limit, int offset) const;
    QString buildDetailsUrl(const QString &mbid, EntityType type) const;
    
    // 请求处理
    void processReply(QNetworkReply *reply, const QString &url);

    NetworkManager *m_networkManager;
    class MusicBrainzParser *m_parser;
    QString m_userAgent;
    
    // 当前请求追踪
    QStringList m_pendingSearchUrls;
    QStringList m_pendingDetailsUrls;
    EntityType m_currentSearchType;
    EntityType m_currentDetailsType;
};

#endif // MUSICBRAINZAPI_H
