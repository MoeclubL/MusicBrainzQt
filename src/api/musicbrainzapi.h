#ifndef MUSICBRAINZAPI_H
#define MUSICBRAINZAPI_H

#include <QObject>
#include <QSharedPointer>
#include <QVariantMap>
#include <QQueue>
#include "../core/types.h"
#include "api_utils.h"

class ResultItem;
class NetworkManager;
class MusicBrainzParser;
class MusicBrainzResponseHandler;
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
    void setUserAgent(const QString &userAgent);    /**
     * @brief 设置认证信息
     * @param username MusicBrainz用户名
     * @param password MusicBrainz密码
     * 
     * 用于需要认证的操作，如管理用户集合等。
     */
    void setAuthentication(const QString &username, const QString &password);
    
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
      /**
     * @brief 查找DiscID对应的发行版
     * @param discId CD的DiscID
     */
    void lookupDiscId(const QString &discId);
    
    /**
     * @brief 通用查询方法
     * @param entity 实体类型字符串
     * @param id 实体ID（可选）
     * @param resource 资源路径（可选）
     * @param includes 包含的关系信息列表
     * @param params 额外查询参数
     */
    void genericQuery(const QString &entity, const QString &id = QString(),
                     const QString &resource = QString(), const QStringList &includes = {},
                     const QVariantMap &params = {});
    
    /**
     * @brief 浏览相关实体
     * @param entity 要浏览的实体类型
     * @param relatedEntity 关联实体类型
     * @param relatedId 关联实体ID
     * @param limit 结果限制（默认25）
     * @param offset 结果偏移（默认0）
     */
    void browse(const QString &entity, const QString &relatedEntity,
               const QString &relatedId, int limit = 25, int offset = 0);
    
    /**
     * @brief 获取用户集合列表
     * 需要认证
     */
    void getUserCollections();
    
    /**
     * @brief 获取集合内容
     * @param collectionId 集合ID
     */
    void getCollectionContents(const QString &collectionId);
    
    /**
     * @brief 添加到集合
     * @param collectionId 集合ID
     * @param releaseIds 要添加的发行版ID列表
     * 需要认证
     */
    void addToCollection(const QString &collectionId, const QStringList &releaseIds);
    
    /**
     * @brief 从集合移除
     * @param collectionId 集合ID
     * @param releaseIds 要移除的发行版ID列表
     * 需要认证
     */
    void removeFromCollection(const QString &collectionId, const QStringList &releaseIds);
    
    // =============================================================================
    // 状态查询方法
    // =============================================================================
    
    /**
     * @brief 获取最后一次请求的HTTP状态码
     */
    int getLastHttpCode() const;
    
    /**
     * @brief 获取最后一次的错误消息
     */
    QString getLastErrorMessage() const;
    
    /**
     * @brief 获取API版本
     */
    QString getVersion() const;

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
    
    // =============================================================================
    // 扩展信号定义
    // =============================================================================
    
    /**
     * @brief DiscID查找结果就绪信号
     * @param releases 匹配的发行版列表
     * @param discId 原始DiscID
     */
    void discIdLookupReady(const QList<QSharedPointer<ResultItem>> &releases, const QString &discId);
    
    /**
     * @brief 通用查询结果就绪信号
     * @param result 查询结果数据
     * @param entity 查询的实体类型
     * @param id 查询的实体ID
     */
    void genericQueryReady(const QVariantMap &result, const QString &entity, const QString &id);
    
    /**
     * @brief 浏览结果就绪信号
     * @param results 浏览结果列表
     * @param entity 浏览的实体类型
     * @param totalCount 总结果数
     * @param offset 当前偏移量
     */
    void browseResultsReady(const QList<QSharedPointer<ResultItem>> &results, 
                           const QString &entity, int totalCount, int offset);
    
    /**
     * @brief 用户集合列表就绪信号
     * @param collections 集合信息列表
     */
    void userCollectionsReady(const QList<QVariantMap> &collections);
    
    /**
     * @brief 集合内容就绪信号
     * @param contents 集合内容列表
     * @param collectionId 集合ID
     */
    void collectionContentsReady(const QList<QSharedPointer<ResultItem>> &contents, 
                                const QString &collectionId);
    
    /**
     * @brief 集合修改完成信号
     * @param success 操作是否成功
     * @param collectionId 集合ID
     * @param operation 操作类型（"add"或"remove"）
     */
    void collectionModified(bool success, const QString &collectionId, const QString &operation);

private slots:
    void onRequestFinished(QNetworkReply *reply, const QString &url);
    void onRequestError(const QString &error, const QString &url);

private:
    // 统一请求处理
    void sendRequestInternal(const QString& url, RequestType type, const QVariantMap& context, 
                             const QString &method = "GET", const QByteArray &data = QByteArray(), 
                             bool authenticated = false);
    void processResponse(QNetworkReply* reply, RequestType type, const QVariantMap& context);

    QString prepareCollectionModification(const QString &collectionId, const QStringList &releaseIds);

    NetworkManager *m_networkManager;
    class MusicBrainzParser *m_parser;
    class MusicBrainzResponseHandler *m_responseHandler;
    QString m_userAgent;
    
    // 认证信息
    QString m_username;
    QString m_password;
    
    // 状态跟踪
    int m_lastHttpCode;
    QString m_lastErrorMessage;
    QString m_version;
};

#endif // MUSICBRAINZAPI_H
