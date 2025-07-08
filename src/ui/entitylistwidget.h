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
 * @brief 通用实体列表显示组件
 * 
 * EntityListWidget是一个高度可复用的组件，用于显示MusicBrainz实体列表。
 * 它被设计为在整个应用程序中统一处理实体列表的显示和交互。
 * 
 * 核心特性：
 * - 通用性：支持所有类型的MusicBrainz实体（艺术家、专辑、录音等）
 * - 可配置：支持自定义列显示、工具栏配置、分页设置
 * - 交互性：提供右键菜单、双击操作、键盘快捷键支持
 * - 可扩展：易于添加新的操作和功能
 * 
 * 使用场景：
 * - 搜索结果列表显示（SearchResultTab）
 * - 实体详情页的相关项目列表（ItemDetailTab）
 * - 任何需要显示实体列表的场景
 * 
 * 设计模式：
 * - 模板方法模式：提供通用的列表操作模板
 * - 策略模式：支持不同的列配置策略
 * - 观察者模式：通过信号槽传递用户交互事件
 * 
 * @see ResultTableModel 表格数据模型
 * @see ResultItem 实体数据容器
 */
class EntityListWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口组件
     */
    explicit EntityListWidget(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~EntityListWidget();
    
    // =============================================================================
    // 数据管理方法
    // =============================================================================
    
    /**
     * @brief 设置要显示的实体列表
     * @param items 实体项目列表
     */
    void setItems(const QList<QSharedPointer<ResultItem>> &items);
    
    
    
    // =============================================================================
    // 选择管理方法
    // =============================================================================
    
    /**
     * @brief 获取当前选中的实体项目
     * @return 当前选中的实体，如果没有选中则返回空指针
     */
    QSharedPointer<ResultItem> getCurrentItem() const;
    
    /**
     * @brief 获取所有选中的实体项目
     * @return 选中的实体列表
     */
    QList<QSharedPointer<ResultItem>> getSelectedItems() const;
    
    // =============================================================================
    // 列管理方法
    // =============================================================================
    
    /**
     * @brief 设置可见的列
     * @param columnKeys 要显示的列键名列表
     */
    void setVisibleColumns(const QStringList &columnKeys);
    
    /**
     * @brief 获取当前可见的列
     * @return 当前显示的列键名列表
     */
    QStringList getVisibleColumns() const;
    
    /**
     * @brief 重置为默认列配置
     */
    void resetToDefaultColumns();
    
    // =============================================================================
    // UI配置方法
    // =============================================================================
    
    /**
     * @brief 设置工具栏可见性
     * @param visible 是否显示工具栏
     */
    void setToolBarVisible(bool visible);
    
    /**
     * @brief 获取工具栏对象
     * @return 工具栏指针，用于添加自定义操作
     */
    QToolBar* getToolBar() const;
    
    /**
     * @brief 设置分页控件可见性
     * @param visible 是否显示分页控件
     */
    void setPaginationVisible(bool visible);
    
    /**
     * @brief 设置分页信息文本
     * @param info 分页信息字符串
     */
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

private slots:
    void onTableDoubleClicked(const QModelIndex &index);
    void onTableSelectionChanged();
    void onContextMenuRequested(const QPoint &pos);
    void onHeaderContextMenuRequested(const QPoint &pos);
    void onOpen();
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
    QAction *m_openAction;
    QAction *m_openInBrowserAction;
    QAction *m_copyIdAction;
    QAction *m_columnSettingsAction;
};

#endif // ENTITYLISTWIDGET_H
