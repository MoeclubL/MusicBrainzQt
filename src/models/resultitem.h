#ifndef RESULTITEM_H
#define RESULTITEM_H

#include <QString>
#include <QVariant>
#include <QIcon>
#include <QMap>
#include "../core/types.h"

// 结果项基类
class ResultItem
{
public:
    ResultItem(const QString &id, const QString &name, EntityType type);
    virtual ~ResultItem();

    QString getId() const;
    QString getName() const;
    EntityType getType() const;
    QIcon getIcon() const;
    QString getTypeString() const;
    
    // 添加 setter 方法
    void setId(const QString &id);
    void setName(const QString &name);
    void setDisambiguation(const QString &disambiguation);
    QString getDisambiguation() const;
    void setScore(int score);
    int getScore() const;
    
    virtual QString getDisplayName() const;
    virtual QMap<QString, QVariant> getDetails() const;
    virtual bool hasChildren() const;

    void setDetailData(const QVariantMap &detailData);
    QVariantMap getDetailData() const;
    
    // 设置和获取单个详细属性
    void setDetailProperty(const QString &key, const QVariant &value);
    QVariant getDetailProperty(const QString &key) const;

protected:
    QString m_id;
    QString m_name;
    EntityType m_type;
    QString m_disambiguation;
    int m_score;
    QVariantMap m_detailData;
};

#endif // RESULTITEM_H
