#ifndef ENTITYLISTWIDGET_H
#define ENTITYLISTWIDGET_H

#include <QWidget>
#include <QSharedPointer>
#include <QMenu>
#include "../core/types.h"

QT_BEGIN_NAMESPACE
class QTableView;
class QToolBar;
class QAction;
QT_END_NAMESPACE

class ResultItem;
class ResultTableModel;

namespace Ui {
class EntityListWidget;
}

/**
 * 通用实体列表组件
 * 复用 ResultTableModel，可用于搜索结果和详细信息的子项显示
 */
class EntityListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EntityListWidget(QWidget *parent = nullptr);
    ~EntityListWidget();    // 设置数据
    void setItems(const QList<QSharedPointer<ResultItem>> &items);
    void clear();
    
    // 获取选中项
    QSharedPointer<ResultItem> getCurrentItem() const;
    QList<QSharedPointer<ResultItem>> getSelectedItems() const;
    
    // 列设置
    void setVisibleColumns(const QStringList &columnKeys);
    QStringList getVisibleColumns() const;
    void resetToDefaultColumns();
      // 工具栏设置
    void setToolBarVisible(bool visible);
    QToolBar* getToolBar() const;
    
    // 分页功能
    void setPaginationVisible(bool visible);
    void setPaginationInfo(const QString &info);
    void setPaginationEnabled(bool prevEnabled, bool nextEnabled);
    void setTitle(const QString &title);

signals:
    void itemDoubleClicked(QSharedPointer<ResultItem> item);
    void itemSelectionChanged();
    void openInBrowser(const QString &itemId, EntityType type);
    void copyId(const QString &itemId);
    void prevPageRequested();
    void nextPageRequested();

private slots:    void onTableDoubleClicked(const QModelIndex &index);
    void onTableSelectionChanged();
    void onContextMenuRequested(const QPoint &pos);
    void onHeaderContextMenuRequested(const QPoint &pos);
    void onOpenInBrowser();
    void onCopyId();
    void onShowColumnSettings();
    void onPrevPageClicked();
    void onNextPageClicked();

private:
    void setupUI();
    void setupActions();
    void setupContextMenu();
    void showColumnSettingsDialog();
    
    Ui::EntityListWidget *ui;
    ResultTableModel *m_model;
    QMenu *m_contextMenu;
    
    // Actions
    QAction *m_openInBrowserAction;
    QAction *m_copyIdAction;
    QAction *m_columnSettingsAction;
};

#endif // ENTITYLISTWIDGET_H
