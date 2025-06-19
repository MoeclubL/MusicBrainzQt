#include "resultitemmodel.h"

ResultItemModel::ResultItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ResultItemModel::~ResultItemModel()
{
}

int ResultItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    
    return m_items.size();
}

QVariant ResultItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();
    
    const QSharedPointer<ResultItem> &item = m_items.at(index.row());
    
    switch (role) {
        case Qt::DisplayRole:
            return item->getDisplayName();
        case Qt::DecorationRole:
            return item->getIcon();
        case Qt::UserRole:
            return QVariant::fromValue(item.data());
        case Qt::ToolTipRole:
            return QString("%1: %2").arg(item->getTypeString(), item->getName());
        default:
            return QVariant();
    }
}

Qt::ItemFlags ResultItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ResultItemModel::addItem(QSharedPointer<ResultItem> item)
{
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();
}

void ResultItemModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

QSharedPointer<ResultItem> ResultItemModel::getItem(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return nullptr;
    
    return m_items.at(index.row());
}

QSharedPointer<ResultItem> ResultItemModel::getItem(int row) const
{
    if (row < 0 || row >= rowCount())
        return nullptr;
    
    return m_items.at(row);
}
