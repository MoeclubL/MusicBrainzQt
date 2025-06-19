#ifndef SEARCHRESULTTAB_H
#define SEARCHRESULTTAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QSharedPointer>
#include "../core/types.h"
#include "../core/error_types.h"

QT_BEGIN_NAMESPACE
class Ui_SearchResultTab;
QT_END_NAMESPACE

class ResultTableModel;
class ResultItem;
class EntityDetailManager;
class EntityListWidget;

/**
 * 搜索结果标签页组件
 * 显示搜索结果列表和分页控件，支持自动加载详细信息
 */
class SearchResultTab : public QWidget
{
    Q_OBJECT

public:
    explicit SearchResultTab(const QString &query, EntityType type, QWidget *parent = nullptr);
    ~SearchResultTab();

    // 设置搜索结果
    void setResults(const QList<QSharedPointer<ResultItem>> &results, const SearchResults &stats);
      // 获取搜索信息
    QString getQuery() const { return m_query; }
    EntityType getEntityType() const { return m_entityType; }
    
    // 获取详细信息管理器
    EntityDetailManager* getDetailManager() const { return m_detailManager; }
    
    // 清空结果
    void clearResults();

signals:
    void itemDoubleClicked(const QSharedPointer<ResultItem> &item);
    void prevPageRequested();
    void nextPageRequested();
    void itemDetailsUpdated(const QSharedPointer<ResultItem> &item);
    
    // 右键菜单信号
    void openInBrowser(const QString &entityId, EntityType entityType);
    void copyId(const QString &entityId);

private slots:
    void onEntityDetailsLoaded(const QString &entityId, const QVariantMap &details);
    void onDetailLoadingFailed(const QString &entityId, const ErrorInfo &error);

private:
    void setupUI();
    void updatePaginationControls(const SearchResults &stats);
    QString getMusicBrainzUrl(const QSharedPointer<ResultItem> &item) const;
    
    // UI对象
    Ui_SearchResultTab *ui;
    EntityListWidget *m_entityListWidget;
    
    // 数据
    QString m_query;
    EntityType m_entityType;
    SearchResults m_currentStats;
    
    // 详细信息管理器
    EntityDetailManager *m_detailManager;
};

#endif // SEARCHRESULTTAB_H
