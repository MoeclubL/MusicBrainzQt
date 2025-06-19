#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QSharedPointer>
#include <QSplitter>
#include <QDockWidget>
#include "core/types.h"
#include "models/resultitem.h"
#include "ui/advancedsearchwidget.h"
#include "services/searchservice.h"
#include "services/entitydetailmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SearchResultTab;
class ItemDetailTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    SearchService *m_searchService;
    EntityDetailManager *m_detailManager;
    
    // UI 组件
    QDockWidget *m_searchDock;
    AdvancedSearchWidget *m_searchWidget;
    QTabWidget *m_mainTabWidget;
    
    // 搜索状态
    SearchParameters m_currentSearchParams;
    
    // 详细标签页映射 - 用于跟踪ItemDetailTab和对应的ResultItem
    QMap<QString, ItemDetailTab*> m_itemDetailTabs; // entityId -> ItemDetailTab

    // UI 初始化方法
    void initializeUI();
    void setupSearchDock();
    void setupMainTabWidget();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    
    // 标签页管理
    SearchResultTab* createSearchResultTab(const QString &query, EntityType type);
    ItemDetailTab* createItemDetailTab(const QSharedPointer<ResultItem> &item);
    void closeTab(int index);
    QString generateTabTitle(const QString &query, EntityType type);
    QString generateDetailTabTitle(const QSharedPointer<ResultItem> &item);
    
private slots:
    void onAdvancedSearchRequested(const SearchParameters &params);
    void onSearchCompleted(const QList<QSharedPointer<ResultItem>> &results, const SearchResults &stats);
    void onSearchFailed(const QString &error);
    void onItemDoubleClicked(const QSharedPointer<ResultItem> &item);
    void onTabCloseRequested(int index);
    void onTabChanged(int index);
    void onPrevPageRequested();
    void onNextPageRequested();
    void onItemDetailsUpdated(const QSharedPointer<ResultItem> &item);
    
    // 右键菜单相关槽函数
    void onOpenInBrowser(const QString &entityId, EntityType entityType);
    void onCopyId(const QString &entityId);
    
    // 详细信息加载相关槽函数
    void onEntityDetailsLoaded(const QString &entityId, const QVariantMap &details);
};

#endif // MAINWINDOW_H