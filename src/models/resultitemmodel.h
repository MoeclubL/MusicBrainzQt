#ifndef RESULTITEMMODEL_H
#define RESULTITEMMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QSharedPointer>
#include "resultitem.h"

class ResultItemModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ResultItemModel(QObject *parent = nullptr);
    ~ResultItemModel();

    // 基本的 QAbstractListModel 覆写
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // 添加结果项
    void addItem(QSharedPointer<ResultItem> item);
    
    // 清除所有项
    void clear();
      // 获取特定索引处的项
    QSharedPointer<ResultItem> getItem(const QModelIndex &index) const;
    QSharedPointer<ResultItem> getItem(int row) const;

private:
    QList<QSharedPointer<ResultItem>> m_items;
};

#endif // RESULTITEMMODEL_H
