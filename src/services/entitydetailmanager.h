#ifndef ENTITYDETAILMANAGER_H
#define ENTITYDETAILMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QVariantMap>
#include <QTimer>
#include <QSet>
#include <QDateTime>
#include "../core/types.h"
#include "../core/error_types.h"
#include "../models/resultitem.h"

class LibMusicBrainzApi;

/**
 * 实体详细信息管理器
 * 负责加载和管理实体的详细信息，支持批量加载
 * 注意：缓存功能已禁用，所有数据都从服务器实时获取
 */
class EntityDetailManager : public QObject {
    Q_OBJECT

public:
    explicit EntityDetailManager(QObject *parent = nullptr);
    
    // 加载单个实体的详细信息
    void loadEntityDetails(QSharedPointer<ResultItem> item);
    
    // 批量加载多个实体的详细信息
    void loadEntitiesDetails(const QList<QSharedPointer<ResultItem>> &items);
      // 获取已缓存的详细信息（缓存已禁用，总是返回空）
    QVariantMap getCachedDetails(const QString &entityId) const;
    
    // 检查实体是否已加载详细信息（缓存已禁用，总是返回false）
    bool hasDetailedInfo(const QString &entityId) const;
    
    // 清理缓存（缓存已禁用，但保留接口兼容性）
    void clearCache();
    
    // 设置批量加载的延迟时间(毫秒)
    void setBatchDelay(int milliseconds);

signals:
    // 单个实体详细信息加载完成
    void entityDetailsLoaded(const QString &entityId, const QVariantMap &details);
    
    // 批量加载完成
    void batchLoadingCompleted(const QStringList &loadedEntityIds);
    
    // 加载失败
    void detailsLoadingFailed(const QString &entityId, const ErrorInfo &error);
    
    // 批量加载进度
    void batchLoadingProgress(int loaded, int total);

private slots:
    void onApiDetailsReady(const QVariantMap &details, EntityType type);
    void onApiErrorOccurred(const QString &error);
    void processBatchQueue();

private:
    struct EntityRequest {
        QSharedPointer<ResultItem> item;
        QDateTime requestTime;
        int retryCount = 0;
    };
    
    LibMusicBrainzApi *m_api;
    
    // 详细信息缓存
    // 详细信息缓存（已禁用，保留变量以避免编译错误）
    QMap<QString, QVariantMap> m_detailsCache;
    QSet<QString> m_loadingItems; // 正在加载的实体ID
    
    // 批量加载队列
    QList<EntityRequest> m_batchQueue;
    QTimer *m_batchTimer;
    int m_batchDelay = 500; // 默认500ms延迟
    
    // 批量加载状态
    QStringList m_currentBatch;
    int m_batchLoadedCount = 0;
    
    // 加载统计
    struct LoadingStats {
        int totalRequested = 0;
        int totalLoaded = 0;
        int totalFailed = 0;
        QDateTime startTime;
    } m_stats;
    
    // 私有方法
    void startBatchLoading();
    void processNextInBatch();
    bool isEntityInQueue(const QString &entityId) const;
    void addToCache(const QString &entityId, const QVariantMap &details);
    void handleLoadingError(const QString &entityId, const ErrorInfo &error);
    
    // 增强实体信息
    void enrichEntityInfo(QSharedPointer<ResultItem> item, const QVariantMap &details);
};

#endif // ENTITYDETAILMANAGER_H
