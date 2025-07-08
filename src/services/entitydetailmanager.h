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

class MusicBrainzApi;

/**
 * @class EntityDetailManager
 * @brief 实体详细信息管理器 - 优化的批量数据加载服务
 * 
 * EntityDetailManager负责从MusicBrainz API获取实体的详细信息，
 * 设计用于高效处理大量数据请求并提供良好的用户体验。
 * 
 * **核心特性：**
 * - **批量加载优化**: 自动将多个请求合并处理，减少API调用次数
 * - **智能延迟**: 可配置的批量处理延迟，平衡响应性和效率
 * - **实时数据**: 当前版本禁用缓存，确保数据始终是最新的
 * - **进度跟踪**: 提供详细的加载进度和统计信息
 * - **错误恢复**: 内置重试机制和错误处理
 * 
 * **使用场景：**
 * ```cpp
 * EntityDetailManager *manager = new EntityDetailManager(this);
 * connect(manager, &EntityDetailManager::entityDetailsLoaded,
 *         this, &MainWindow::onEntityDetailsLoaded);
 * 
 * // 单个实体加载
 * manager->loadEntityDetails(resultItem);
 * 
 * // 批量加载
 * QList<QSharedPointer<ResultItem>> items = getSelectedItems();
 * manager->loadEntitiesDetails(items);
 * ```
 * 
 * **性能说明：**
 * - 默认批处理延迟: 500ms
 * - 支持并发请求处理
 * - 内存使用优化（缓存已禁用）
 * 
 * @note 缓存功能在当前版本中已禁用，以确保数据实时性。
 *       相关接口保留以维持API兼容性。
 * 
 * @author MusicBrainzQt Team
 * @see MusicBrainzApi, ResultItem
 */
class EntityDetailManager : public QObject {    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象，用于Qt对象树管理
     * 
     * 初始化批处理定时器和API连接。
     */
    explicit EntityDetailManager(QObject *parent = nullptr);
    
    // =============================================================================
    // 核心加载接口
    // =============================================================================
    
    /**
     * @brief 加载单个实体的详细信息
     * @param item 要加载详情的结果项
     * 
     * 将单个实体添加到批处理队列中。如果队列中已存在相同实体，
     * 则不会重复添加。实际的API请求会在批处理延迟后执行。
     * 
     * @see setBatchDelay(), loadEntitiesDetails()
     */
    void loadEntityDetails(QSharedPointer<ResultItem> item);
    
    /**
     * @brief 批量加载多个实体的详细信息
     * @param items 要加载详情的结果项列表
     * 
     * 高效的批量加载接口，将多个实体添加到处理队列中。
     * 重复的实体会自动过滤，确保每个实体只处理一次。
     * 
     * **性能特点：**
     * - 自动去重处理
     * - 批量请求优化
     * - 进度跟踪支持
     */
    void loadEntitiesDetails(const QList<QSharedPointer<ResultItem>> &items);
    
    
    
    // =============================================================================
    // 配置接口
    // =============================================================================
    
    /**
     * @brief 设置批量加载的延迟时间
     * @param milliseconds 延迟时间（毫秒）
     * 
     * 配置批处理的延迟时间。较短的延迟提供更快的响应，
     * 但可能增加API调用次数；较长的延迟提高批处理效率，
     * 但可能影响用户体验。
     * 
     * **推荐值：**
     * - 快速响应: 200-300ms
     * - 平衡模式: 500ms（默认）
     * - 高效批处理: 1000-2000ms
     */
    void setBatchDelay(int milliseconds);

signals:
    /**
     * @brief 单个实体详细信息加载完成信号
     * @param entityId 实体ID
     * @param details 详细信息数据
     * 
     * 当单个实体的详细信息成功从API加载后发出。
     * details包含从MusicBrainz API获取的完整实体信息。
     */
    void entityDetailsLoaded(const QString &entityId, const QVariantMap &details);
    
    /**
     * @brief 批量加载完成信号
     * @param loadedEntityIds 成功加载的实体ID列表
     * 
     * 当一个批次的所有实体都完成加载时发出。
     * 不包含加载失败的实体ID。
     */
    void batchLoadingCompleted(const QStringList &loadedEntityIds);
    
    /**
     * @brief 加载失败信号
     * @param entityId 失败的实体ID
     * @param error 错误信息
     * 
     * 当实体详细信息加载失败时发出，包含具体的错误信息。
     */
    void detailsLoadingFailed(const QString &entityId, const ErrorInfo &error);
    
    /**
     * @brief 批量加载进度信号
     * @param loaded 已加载的实体数量
     * @param total 总实体数量
     * 
     * 在批量加载过程中定期发出，用于更新进度显示。
     */
    void batchLoadingProgress(int loaded, int total);

private slots:
    /**
     * @brief 处理API返回的详细信息
     * @param details 详细信息数据
     * @param type 实体类型
     */
    void onApiDetailsReady(const QVariantMap &details, EntityType type);
    
    /**
     * @brief 处理API错误
     * @param error 错误信息
     */
    void onApiErrorOccurred(const QString &error);
    
    /**
     * @brief 处理批量队列
     * 
     * 定时器触发时执行，开始处理待加载的实体队列。
     */
    void processBatchQueue();

private:
    /**
     * @struct EntityRequest
     * @brief 实体请求结构
     * 
     * 存储单个实体的加载请求信息，包含请求时间和重试计数。
     */
    struct EntityRequest {
        QSharedPointer<ResultItem> item;    ///< 结果项指针
        QDateTime requestTime;              ///< 请求时间戳
        int retryCount = 0;                 ///< 重试次数
    };
    
    // =============================================================================
    // 成员变量
    // =============================================================================
    
    MusicBrainzApi *m_api;                           ///< MusicBrainz API接口
    
    // 缓存相关（已禁用但保留结构）
    
    QSet<QString> m_loadingItems;                       ///< 正在加载的实体ID集合
    
    // 批量加载队列管理
    QList<EntityRequest> m_batchQueue;                  ///< 批量加载队列
    QTimer *m_batchTimer;                               ///< 批量处理定时器
    int m_batchDelay = 500;                             ///< 批量处理延迟时间（毫秒）
    
    // 批量加载状态跟踪
    QStringList m_currentBatch;                         ///< 当前批次的实体ID列表
    int m_batchLoadedCount = 0;                         ///< 当前批次已加载数量
    
    /**
     * @struct LoadingStats
     * @brief 加载统计信息
     * 
     * 跟踪整体的加载性能和状态。
     */
    struct LoadingStats {
        int totalRequested = 0;                         ///< 总请求数量
        int totalLoaded = 0;                            ///< 总加载成功数量
        int totalFailed = 0;                            ///< 总失败数量
        QDateTime startTime;                            ///< 开始时间
    } m_stats;                                          ///< 统计信息实例
    
    // =============================================================================
    // 私有方法
    // =============================================================================
    
    /**
     * @brief 开始批量加载处理
     * 
     * 初始化批量加载过程，设置状态和统计信息。
     */
    void startBatchLoading();
    
    /**
     * @brief 处理批次中的下一个实体
     * 
     * 顺序处理批次队列中的实体，发送API请求。
     */
    void processNextInBatch();
    
    /**
     * @brief 检查实体是否在队列中
     * @param entityId 实体ID
     * @return true 如果实体已在处理队列中
     */
    bool isEntityInQueue(const QString &entityId) const;
    
    
    
    /**
     * @brief 增强实体信息
     * @param item 结果项
     * @param details 从API获取的详细信息
     * 
     * 将API返回的详细信息整合到ResultItem对象中。
     */
    void enrichEntityInfo(QSharedPointer<ResultItem> item, const QVariantMap &details);
};

#endif // ENTITYDETAILMANAGER_H
