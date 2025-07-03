#include "entitydetailmanager.h"
#include "../api/musicbrainzapi.h"
#include "../core/error_types.h"
#include <QDebug>

EntityDetailManager::EntityDetailManager(QObject *parent)
    : QObject(parent)
    , m_api(new MusicBrainzApi(this))
    , m_batchTimer(new QTimer(this))
{
    // 配置批量处理定时器
    m_batchTimer->setSingleShot(true);
    connect(m_batchTimer, &QTimer::timeout, this, &EntityDetailManager::processBatchQueue);
    
    // 连接API信号
    connect(m_api, &MusicBrainzApi::detailsReady,
            this, &EntityDetailManager::onApiDetailsReady);
    connect(m_api, &MusicBrainzApi::errorOccurred,
            this, &EntityDetailManager::onApiErrorOccurred);
            
    qDebug() << "EntityDetailManager initialized with batch delay:" << m_batchDelay << "ms";
}

void EntityDetailManager::loadEntityDetails(QSharedPointer<ResultItem> item) {
    if (!item) {
        qWarning() << "Attempted to load details for null item";
        return;
    }
    
    const QString entityId = item->getId();
      // 缓存已禁用 - 始终从服务器获取最新数据
    // if (hasDetailedInfo(entityId)) {
    //     // 实体详情已缓存，直接返回
    //     emit entityDetailsLoaded(entityId, getCachedDetails(entityId));
    //     return;
    // }
    
    // 检查是否正在加载
    if (m_loadingItems.contains(entityId)) {
        qDebug() << "Entity details already loading:" << entityId;
        return;
    }
      // 添加到批量队列
    EntityRequest request;
    request.item = item;
    request.requestTime = QDateTime::currentDateTime();
    
    if (!isEntityInQueue(entityId)) {
        m_batchQueue.append(request);
        qDebug() << "Added entity to batch queue:" << entityId;
    }
    
    // 启动或重启批量处理定时器
    m_batchTimer->start(m_batchDelay);
}

void EntityDetailManager::loadEntitiesDetails(const QList<QSharedPointer<ResultItem>> &items) {
    qDebug() << "Requested batch loading for" << items.size() << "entities";
    
    for (const auto &item : items) {
        loadEntityDetails(item);
    }
}

QVariantMap EntityDetailManager::getCachedDetails(const QString &entityId) const {
    // 缓存已禁用，返回空映射
    Q_UNUSED(entityId)
    return QVariantMap();
}

bool EntityDetailManager::hasDetailedInfo(const QString &entityId) const {
    // 缓存已禁用，总是返回false以强制从服务器获取数据
    Q_UNUSED(entityId)
    return false;
}

void EntityDetailManager::clearCache() {
    // 缓存已禁用，但保留清理操作以确保内存清理
    qDebug() << "Clearing entity details cache (cache disabled)";
    m_detailsCache.clear(); // 仍然清理以防有遗留数据
    m_loadingItems.clear();
    m_batchQueue.clear();
    m_currentBatch.clear();
    m_batchLoadedCount = 0;
    
    if (m_batchTimer->isActive()) {
        m_batchTimer->stop();
    }
}

void EntityDetailManager::setBatchDelay(int milliseconds) {
    m_batchDelay = qMax(100, milliseconds); // 最小100ms
    qDebug() << "Batch delay set to:" << m_batchDelay << "ms";
}

void EntityDetailManager::processBatchQueue() {
    if (m_batchQueue.isEmpty()) {
        return;
    }
    
    qDebug() << "Processing batch queue with" << m_batchQueue.size() << "items";
    
    // 启动批量加载
    startBatchLoading();
}

void EntityDetailManager::startBatchLoading() {
    if (m_batchQueue.isEmpty()) {
        return;
    }
    
    // 准备当前批次
    m_currentBatch.clear();
    m_batchLoadedCount = 0;
    m_stats.startTime = QDateTime::currentDateTime();
      // 收集需要加载的实体ID
    for (const auto &request : m_batchQueue) {
        const QString entityId = request.item->getId();
        // 缓存已禁用，直接加载所有实体
        if (!m_loadingItems.contains(entityId)) {
            m_currentBatch.append(entityId);
            m_loadingItems.insert(entityId);
        }
    }
    
    m_stats.totalRequested = m_currentBatch.size();
    
    if (m_currentBatch.isEmpty()) {
        qDebug() << "No entities need loading in current batch";
        emit batchLoadingCompleted(QStringList());
        m_batchQueue.clear();
        return;
    }
    
    qDebug() << "Starting batch loading for" << m_currentBatch.size() << "entities";
    
    // 开始加载第一个实体
    processNextInBatch();
}

void EntityDetailManager::processNextInBatch() {
    if (m_batchLoadedCount >= m_currentBatch.size()) {
        // 批量加载完成
        qDebug() << "Batch loading completed:" << m_stats.totalLoaded 
                 << "/" << m_stats.totalRequested << "entities loaded";
        
        emit batchLoadingCompleted(m_currentBatch);
        emit batchLoadingProgress(m_stats.totalLoaded, m_stats.totalRequested);
        
        // 清理
        m_batchQueue.clear();
        m_currentBatch.clear();
        return;
    }
    
    // 找到下一个需要加载的实体
    const QString entityId = m_currentBatch[m_batchLoadedCount];
    
    // 在队列中找到对应的请求
    EntityRequest *request = nullptr;
    for (auto &req : m_batchQueue) {
        if (req.item->getId() == entityId) {
            request = &req;
            break;
        }
    }
    
    if (!request) {
        qWarning() << "Could not find request for entity:" << entityId;
        m_batchLoadedCount++;
        processNextInBatch();
        return;
    }
    
    // 发送API请求
    qDebug() << "Loading details for entity:" << entityId 
             << "(type:" << static_cast<int>(request->item->getType()) << ")";
    
    m_api->getDetails(entityId, request->item->getType());
}

void EntityDetailManager::onApiDetailsReady(const QVariantMap &details, EntityType type) {
    Q_UNUSED(type) // 参数在当前实现中暂未使用
    
    // 从详细信息中提取实体ID
    QString entityId = details.value("id").toString();
    
    if (entityId.isEmpty()) {
        qWarning() << "Received details without entity ID";
        return;
    }
    
    qDebug() << "Received details for entity:" << entityId 
             << "keys:" << details.keys().join(", ");
    
    // 打印详细信息内容以便调试
    for (auto it = details.constBegin(); it != details.constEnd(); ++it) {
        qDebug() << "Detail key:" << it.key() << "=" << it.value().toString();
    }
      // 缓存已禁用，不再保存到缓存
    // addToCache(entityId, details);
    
    // 移除加载状态
    m_loadingItems.remove(entityId);
    
    // 增强实体信息
    for (const auto &request : m_batchQueue) {
        if (request.item->getId() == entityId) {
            enrichEntityInfo(request.item, details);
            break;
        }
    }
    
    // 发送信号
    emit entityDetailsLoaded(entityId, details);
    
    // 更新统计
    m_stats.totalLoaded++;
    m_batchLoadedCount++;
    
    // 发送进度信号
    emit batchLoadingProgress(m_stats.totalLoaded, m_stats.totalRequested);
    
    // 处理下一个实体
    QTimer::singleShot(100, this, &EntityDetailManager::processNextInBatch); // 100ms间隔避免API限流
}

void EntityDetailManager::onApiErrorOccurred(const QString &error) {
    qCritical() << "API error occurred:" << error;
    
    // 更新统计
    m_stats.totalFailed++;
    m_batchLoadedCount++;
    
    // 创建错误信息
    ErrorInfo errorInfo(ErrorCode::ApiServerError, error);
    
    // 如果是批量加载中的错误，继续处理下一个
    if (m_batchLoadedCount < m_currentBatch.size()) {
        const QString failedEntityId = m_currentBatch[m_batchLoadedCount - 1];
        m_loadingItems.remove(failedEntityId);
        emit detailsLoadingFailed(failedEntityId, errorInfo);
        
        // 继续处理下一个实体
        QTimer::singleShot(200, this, &EntityDetailManager::processNextInBatch); // 稍长间隔以避免连续错误
    }
}

bool EntityDetailManager::isEntityInQueue(const QString &entityId) const {
    for (const auto &request : m_batchQueue) {
        if (request.item->getId() == entityId) {
            return true;
        }
    }
    return false;
}

void EntityDetailManager::addToCache(const QString &entityId, const QVariantMap &details) {
    // 缓存已禁用，不再保存数据到缓存
    Q_UNUSED(entityId)
    Q_UNUSED(details)
    // m_detailsCache[entityId] = details;
    // 缓存已禁用，不保存实体详情
}

void EntityDetailManager::enrichEntityInfo(QSharedPointer<ResultItem> item, const QVariantMap &details) {
    if (!item) {
        return;
    }
    
    // 根据实体类型增强信息
    switch (item->getType()) {
        case EntityType::Artist: {
            // 添加艺术家特定信息
            if (details.contains("life-span")) {
                QVariantMap lifeSpan = details["life-span"].toMap();
                QString begin = lifeSpan["begin"].toString();
                QString end = lifeSpan["end"].toString();
                if (!begin.isEmpty()) {
                    item->setDetailProperty("birth_date", begin);
                }
                if (!end.isEmpty()) {
                    item->setDetailProperty("death_date", end);
                }
            }
            
            if (details.contains("area")) {
                QVariantMap area = details["area"].toMap();
                item->setDetailProperty("origin", area["name"].toString());
                if (area.contains("iso-3166-1-codes")) {
                    QVariantList codes = area["iso-3166-1-codes"].toList();
                    if (!codes.isEmpty()) {
                        item->setDetailProperty("country", codes.first().toString());
                    }
                }
            }
            
            if (details.contains("type")) {
                item->setDetailProperty("artist_type", details["type"].toString());
            }
            
            if (details.contains("gender")) {
                item->setDetailProperty("gender", details["gender"].toString());
            }
            
            // 处理发行数量
            if (details.contains("releaseCount")) {
                item->setDetailProperty("release_count", details["releaseCount"].toInt());
            }
              // 处理录音数量和录音列表
            if (details.contains("recordingCount")) {
                item->setDetailProperty("recording_count", details["recordingCount"].toInt());
            }
            
            // 处理录音列表
            if (details.contains("recordings")) {
                QVariant recordingsData = details["recordings"];
                QStringList recordingNames;
                
                if (recordingsData.canConvert<QVariantList>()) {
                    // 如果是对象列表
                    QVariantList recordingsList = recordingsData.toList();
                    for (const auto &recording : recordingsList) {
                        if (recording.canConvert<QVariantMap>()) {
                            QVariantMap recordingMap = recording.toMap();
                            QString title = recordingMap.value("title", recordingMap.value("name")).toString();
                            if (!title.isEmpty()) {
                                recordingNames.append(title);
                            }
                        } else {
                            // 简单字符串
                            recordingNames.append(recording.toString());
                        }
                    }
                } else if (recordingsData.canConvert<QStringList>()) {
                    // 如果直接是字符串列表
                    recordingNames = recordingsData.toStringList();
                } else {
                    // 如果是逗号分隔的字符串
                    QString recordingsStr = recordingsData.toString();
                    if (!recordingsStr.isEmpty()) {
                        recordingNames = recordingsStr.split(", ");
                    }
                }
                
                if (!recordingNames.isEmpty()) {
                    item->setDetailProperty("recordings", recordingNames);
                    item->setDetailProperty("recording_names", recordingNames.join(", "));
                }
            }
            
            // 处理作品列表
            if (details.contains("works")) {
                QVariant worksData = details["works"];
                QStringList workNames;
                
                if (worksData.canConvert<QVariantList>()) {
                    // 如果是对象列表
                    QVariantList worksList = worksData.toList();
                    for (const auto &work : worksList) {
                        if (work.canConvert<QVariantMap>()) {
                            QVariantMap workMap = work.toMap();
                            QString title = workMap.value("title", workMap.value("name")).toString();
                            if (!title.isEmpty()) {
                                workNames.append(title);
                            }
                        } else {
                            // 简单字符串
                            workNames.append(work.toString());
                        }
                    }
                } else if (worksData.canConvert<QStringList>()) {
                    // 如果直接是字符串列表
                    workNames = worksData.toStringList();
                } else {
                    // 如果是逗号分隔的字符串
                    QString worksStr = worksData.toString();
                    if (!worksStr.isEmpty()) {
                        workNames = worksStr.split(", ");
                    }
                }
                
                if (!workNames.isEmpty()) {
                    item->setDetailProperty("works", workNames);
                    item->setDetailProperty("work_names", workNames.join(", "));
                }
            }
            
            // 处理发行列表
            if (details.contains("releases")) {
                QVariant releasesData = details["releases"];
                QStringList releaseNames;
                
                if (releasesData.canConvert<QVariantList>()) {
                    // 如果是对象列表
                    QVariantList releasesList = releasesData.toList();
                    for (const auto &release : releasesList) {
                        if (release.canConvert<QVariantMap>()) {
                            QVariantMap releaseMap = release.toMap();
                            QString title = releaseMap.value("title", releaseMap.value("name")).toString();
                            if (!title.isEmpty()) {
                                releaseNames.append(title);
                            }
                        } else {
                            // 简单字符串
                            releaseNames.append(release.toString());
                        }
                    }
                } else if (releasesData.canConvert<QStringList>()) {
                    // 如果直接是字符串列表
                    releaseNames = releasesData.toStringList();
                } else {
                    // 如果是逗号分隔的字符串
                    QString releasesStr = releasesData.toString();
                    if (!releasesStr.isEmpty()) {
                        releaseNames = releasesStr.split(", ");
                    }
                }
                
                if (!releaseNames.isEmpty()) {
                    item->setDetailProperty("releases", releaseNames);
                    item->setDetailProperty("release_names", releaseNames.join(", "));
                }
            }
            
            // 处理别名
            if (details.contains("aliases")) {
                QVariantList aliases = details["aliases"].toList();
                QStringList aliasNames;
                for (const auto &alias : aliases) {
                    QVariantMap aliasMap = alias.toMap();
                    QString name = aliasMap["name"].toString();
                    if (!name.isEmpty()) {
                        aliasNames.append(name);
                    }
                }
                if (!aliasNames.isEmpty()) {
                    item->setDetailProperty("aliases", aliasNames);
                }
            }
            
            break;
        }
        
        case EntityType::Release: {            // 添加发行特定信息
            if (details.contains("date")) {
                item->setDetailProperty("release_date", details["date"].toString());
            }
            
            if (details.contains("country")) {
                item->setDetailProperty("country", details["country"].toString());
            }
            
            if (details.contains("status")) {
                item->setDetailProperty("status", details["status"].toString());
            }
            
            if (details.contains("track-count")) {
                item->setDetailProperty("track_count", details["track-count"].toInt());
            }
            break;
        }
        
        case EntityType::Recording: {
            // 添加录音特定信息
            if (details.contains("length")) {
                item->setDetailProperty("duration", details["length"].toInt());
            }
            
            if (details.contains("disambiguation")) {
                item->setDetailProperty("disambiguation", details["disambiguation"].toString());
            }
            break;
        }
          default:
            break;
    }
    
    // 通用属性
    if (details.contains("disambiguation")) {
        item->setDetailProperty("disambiguation", details["disambiguation"].toString());
    }
    
    if (details.contains("tags")) {
        QVariantList tags = details["tags"].toList();
        QStringList tagNames;
        for (const auto &tag : tags) {
            QVariantMap tagMap = tag.toMap();
            tagNames.append(tagMap["name"].toString());
        }
        item->setDetailProperty("tags", tagNames);
    }
    
    // 保存所有原始详细数据，确保不丢失任何信息
    QVariantMap currentDetailData = item->getDetailData();
    for (auto it = details.constBegin(); it != details.constEnd(); ++it) {
        QString key = it.key();
        QVariant value = it.value();
        
        // 如果键还没有被处理过，就直接保存
        if (!currentDetailData.contains(key)) {
            item->setDetailProperty(key, value);
        }
    }
    
    qDebug() << "Enriched entity info for:" << item->getId() 
             << "total detail fields:" << item->getDetailData().size();
}
