#ifndef RESULTITEM_H
#define RESULTITEM_H

#include <QString>
#include <QVariant>
#include <QIcon>
#include <QMap>
#include "../core/types.h"

/**
 * @class ResultItem
 * @brief MusicBrainz搜索结果项的基础数据模型
 * 
 * ResultItem是所有MusicBrainz实体的基础数据容器，提供了统一的
 * 数据访问接口和属性管理功能。它设计用于存储从MusicBrainz API
 * 获取的实体基本信息和详细数据。
 * 
 * **核心功能：**
 * - **基础属性管理**: ID、名称、类型、消歧等核心属性
 * - **详细数据存储**: 灵活的键值对存储，支持复杂的API数据
 * - **显示支持**: 提供格式化的显示名称和图标
 * - **扩展接口**: 虚拟方法支持子类化和特殊化
 * - **搜索评分**: 存储MusicBrainz搜索匹配度评分
 * 
 * **数据层次结构：**
 * ```
 * ResultItem (基类)
 * ├── 基础属性 (id, name, type, disambiguation, score)
 * ├── 详细数据 (任意键值对，来自API的完整信息)
 * └── 虚拟接口 (显示名称、详情获取、子项检查)
 * ```
 * 
 * **使用示例：**
 * ```cpp
 * // 创建结果项
 * auto item = QSharedPointer<ResultItem>::create(
 *     "5441c29d-3602-4898-b1a1-b77fa23b8e50",
 *     "The Beatles",
 *     EntityType::Artist
 * );
 * 
 * // 设置基础属性
 * item->setScore(95);
 * item->setDisambiguation("British rock band");
 * 
 * // 设置详细数据
 * item->setDetailProperty("country", "GB");
 * item->setDetailProperty("type", "Group");
 * item->setDetailProperty("begin-date", "1960");
 * 
 * // 获取显示信息
 * QString displayName = item->getDisplayName();
 * QMap<QString, QVariant> details = item->getDetails();
 * ```
 * 
 * **设计模式：**
 * - **值对象模式**: 不可变的标识信息（ID、类型）
 * - **属性包模式**: 灵活的详细数据存储
 * - **策略模式**: 虚拟方法支持不同实体类型的特定行为
 * 
 * @note 此类主要通过QSharedPointer使用，确保内存安全和数据共享
 * 
 * @author MusicBrainzQt Team
 * @see EntityType, QSharedPointer
 */
class ResultItem
{
public:
    /**
     * @brief 构造函数
     * @param id MusicBrainz实体的唯一标识符（MBID）
     * @param name 实体的名称
     * @param type 实体类型（Artist、Release等）
     * 
     * 创建一个新的结果项实例，设置核心标识属性。
     * 这些属性在创建后通常不应改变。
     */
    ResultItem(const QString &id, const QString &name, EntityType type);
    
    /**
     * @brief 虚拟析构函数
     * 
     * 确保派生类能够正确清理资源。
     */
    virtual ~ResultItem();

    // =============================================================================
    // 核心属性访问接口
    // =============================================================================
    
    /**
     * @brief 获取实体ID
     * @return MusicBrainz实体的唯一标识符
     */
    QString getId() const;
    
    /**
     * @brief 获取实体名称
     * @return 实体的显示名称
     */
    QString getName() const;
    
    /**
     * @brief 获取实体类型
     * @return EntityType枚举值
     */
    EntityType getType() const;
    
    /**
     * @brief 获取实体图标
     * @return 表示实体类型的图标
     * 
     * 返回与实体类型对应的图标，用于UI显示。
     */
    QIcon getIcon() const;
    
    /**
     * @brief 获取实体类型的字符串表示
     * @return 实体类型的本地化字符串
     */
    QString getTypeString() const;
    
    // =============================================================================
    // 属性修改接口
    // =============================================================================
    
    /**
     * @brief 设置实体ID
     * @param id 新的实体ID
     * 
     * @warning 通常不建议在创建后修改ID，可能导致数据不一致
     */
    void setId(const QString &id);
    
    /**
     * @brief 设置实体名称
     * @param name 新的实体名称
     */
    void setName(const QString &name);
    
    /**
     * @brief 设置消歧信息
     * @param disambiguation 用于区分同名实体的额外信息
     * 
     * 消歧信息帮助用户区分具有相同名称的不同实体，
     * 例如："The Beatles (British rock band)"中的"British rock band"部分。
     */
    void setDisambiguation(const QString &disambiguation);
    
    /**
     * @brief 获取消歧信息
     * @return 消歧字符串，如果未设置则返回空字符串
     */
    QString getDisambiguation() const;
    
    /**
     * @brief 设置搜索匹配评分
     * @param score 搜索匹配度评分（0-100）
     * 
     * MusicBrainz搜索API返回的匹配度评分，用于排序搜索结果。
     */
    void setScore(int score);
    
    /**
     * @brief 获取搜索匹配评分
     * @return 匹配度评分（0-100）
     */
    int getScore() const;
    
    // =============================================================================
    // 虚拟接口（支持子类扩展）
    // =============================================================================
    
    /**
     * @brief 获取格式化的显示名称
     * @return 包含消歧信息的完整显示名称
     * 
     * 默认实现组合名称和消歧信息，子类可以重写提供特定格式。
     * 例如："The Beatles (British rock band)"
     */
    virtual QString getDisplayName() const;
    
    /**
     * @brief 获取实体的详细信息映射
     * @return 包含所有详细属性的键值对映射
     * 
     * 返回适合在UI中显示的详细信息。子类可以重写以提供
     * 特定实体类型的格式化详情。
     */
    virtual QMap<QString, QVariant> getDetails() const;
    
    /**
     * @brief 检查实体是否有子项
     * @return true 如果实体包含子项（如Artist包含Release）
     * 
     * 用于UI决定是否显示展开/折叠控件。子类应根据实体类型重写此方法。
     */
    virtual bool hasChildren() const;

    // =============================================================================
    // 详细数据管理接口
    // =============================================================================
    
    /**
     * @brief 设置完整的详细数据
     * @param detailData 详细数据的键值对映射
     * 
     * 批量设置从MusicBrainz API获取的详细信息。
     * 通常在EntityDetailManager加载详情后调用。
     */
    void setDetailData(const QVariantMap &detailData);
    
    /**
     * @brief 获取完整的详细数据
     * @return 所有详细数据的键值对映射
     */
    QVariantMap getDetailData() const;
    
    /**
     * @brief 设置单个详细属性
     * @param key 属性键名
     * @param value 属性值
     * 
     * 设置或更新单个详细属性，支持增量数据更新。
     */
    void setDetailProperty(const QString &key, const QVariant &value);
    
    /**
     * @brief 获取单个详细属性
     * @param key 属性键名
     * @return 属性值，如果不存在则返回无效的QVariant
     */
    QVariant getDetailProperty(const QString &key) const;

protected:
    // =============================================================================
    // 成员变量
    // =============================================================================
    
    QString m_id;                   ///< 实体唯一标识符（MBID）
    QString m_name;                 ///< 实体名称
    EntityType m_type;              ///< 实体类型
    QString m_disambiguation;       ///< 消歧信息
    int m_score;                    ///< 搜索匹配评分
    QVariantMap m_detailData;       ///< 详细数据存储
};

#endif // RESULTITEM_H
