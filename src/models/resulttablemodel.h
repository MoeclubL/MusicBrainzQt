#ifndef RESULTTABLEMODEL_H
#define RESULTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QSharedPointer>
#include <QStringList>
#include "resultitem.h"
#include "../core/types.h"

struct ColumnInfo {
    QString key;        // 数据字段名
    QString header;     // 表头显示名
    QString description; // 字段描述
};

class ResultTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    ResultTableModel(QObject *parent = nullptr);
    void setItems(const QList<QSharedPointer<ResultItem>> &items, EntityType type);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    
    // 排序支持
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    
    EntityType currentType() const;
    QSharedPointer<ResultItem> getItem(int row) const;// 自定义列功能
    QList<ColumnInfo> getAvailableColumns(EntityType type) const;
    
    QStringList getVisibleColumns() const;
    void setVisibleColumns(const QStringList &columnKeys);
    void resetToDefaultColumns();
    QList<ColumnInfo> getDefaultColumns(EntityType type) const;
    
    // 动态列检测
    QList<ColumnInfo> detectColumnsFromData() const;
    void updateColumnsFromData();
    
    // 列生成辅助函数
    QString generateFriendlyColumnName(const QString &key) const;
    QString generateColumnDescription(const QString &key, EntityType type) const;
    
    
private:
    QList<QSharedPointer<ResultItem>> m_items;
    EntityType m_type;
    QList<ColumnInfo> m_visibleColumns;  // 当前显示的列
    
    // 根据字段键获取数据值
    QVariant getFieldValue(const QSharedPointer<ResultItem> &item, const QString &fieldKey) const;
};

#endif // RESULTTABLEMODEL_H
