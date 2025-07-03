#ifndef MUSICBRAINZ_RESPONSE_HANDLER_H
#define MUSICBRAINZ_RESPONSE_HANDLER_H

#include <QObject>
#include <QVariantMap>
#include <QList>
#include <QSharedPointer>
#include "../core/types.h"

class ResultItem;
class MusicBrainzParser;
class QNetworkReply;

/**
 * @class MusicBrainzResponseHandler
 * @brief MusicBrainz API响应处理器
 * 
 * 负责处理从MusicBrainz API收到的各种响应，包括：
 * - 搜索结果解析
 * - 详情信息解析
 * - DiscID查找结果
 * - 通用查询响应
 * - 浏览结果
 * - 集合操作响应
 */
class MusicBrainzResponseHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parser MusicBrainz解析器指针
     * @param parent 父对象
     */
    explicit MusicBrainzResponseHandler(MusicBrainzParser *parser, QObject *parent = nullptr);

    /**
     * @brief 处理搜索响应
     * @param data API返回的原始数据
     * @param context 请求上下文信息，包含entityType等
     */
    void handleSearchResponse(const QByteArray& data, const QVariantMap& context);
    
    /**
     * @brief 处理详情响应
     * @param data API返回的原始数据
     * @param context 请求上下文信息，包含entityType等
     */
    void handleDetailsResponse(const QByteArray& data, const QVariantMap& context);
    
    /**
     * @brief 处理DiscID查找响应
     * @param data API返回的原始数据
     * @param context 请求上下文信息，包含discId等
     */
    void handleDiscIdResponse(const QByteArray& data, const QVariantMap& context);
    
    /**
     * @brief 处理通用查询响应
     * @param data API返回的原始数据
     * @param context 请求上下文信息，包含entity、id等
     */
    void handleGenericResponse(const QByteArray& data, const QVariantMap& context);
    
    /**
     * @brief 处理浏览响应
     * @param data API返回的原始数据
     * @param context 请求上下文信息，包含entity等
     */
    void handleBrowseResponse(const QByteArray& data, const QVariantMap& context);
    
    /**
     * @brief 处理集合操作响应
     * @param data API返回的原始数据
     * @param context 请求上下文信息，包含operation、collectionId等
     */
    void handleCollectionResponse(const QByteArray& data, const QVariantMap& context);

signals:
    /**
     * @brief 搜索结果准备就绪信号
     * @param results 搜索结果列表
     * @param totalCount 总结果数
     * @param offset 偏移量
     */
    void searchResultsReady(const QList<QSharedPointer<ResultItem>>& results, int totalCount, int offset);
    
    /**
     * @brief 详情信息准备就绪信号
     * @param details 详情信息映射
     * @param entityType 实体类型
     */
    void detailsReady(const QVariantMap& details, EntityType entityType);
    
    /**
     * @brief DiscID查找结果准备就绪信号
     * @param releases 发行版列表
     * @param discId 光盘ID
     */
    void discIdLookupReady(const QList<QSharedPointer<ResultItem>>& releases, const QString& discId);
    
    /**
     * @brief 通用查询结果准备就绪信号
     * @param result 查询结果
     * @param entity 实体类型字符串
     * @param id 实体ID
     */
    void genericQueryReady(const QVariantMap& result, const QString& entity, const QString& id);
    
    /**
     * @brief 浏览结果准备就绪信号
     * @param results 浏览结果列表
     * @param entity 实体类型字符串
     * @param totalCount 总结果数
     * @param offset 偏移量
     */
    void browseResultsReady(const QList<QSharedPointer<ResultItem>>& results, const QString& entity, int totalCount, int offset);
    
    /**
     * @brief 用户集合列表准备就绪信号
     * @param collections 集合列表
     */
    void userCollectionsReady(const QList<QVariantMap>& collections);
    
    /**
     * @brief 集合内容准备就绪信号
     * @param contents 集合内容
     * @param collectionId 集合ID
     */
    void collectionContentsReady(const QList<QSharedPointer<ResultItem>>& contents, const QString& collectionId);
    
    /**
     * @brief 集合修改完成信号
     * @param success 操作是否成功
     * @param collectionId 集合ID
     * @param operation 操作类型（add/remove）
     */
    void collectionModified(bool success, const QString& collectionId, const QString& operation);
    
    /**
     * @brief 错误发生信号
     * @param errorMessage 错误消息
     */
    void errorOccurred(const QString& errorMessage);

private:
    /**
     * @brief MusicBrainz解析器指针
     */
    MusicBrainzParser *m_parser;
};

#endif // MUSICBRAINZ_RESPONSE_HANDLER_H
