#ifndef SEARCHSERVICE_H
#define SEARCHSERVICE_H

#include <QObject>
#include <QSharedPointer>
#include "../core/types.h"

class ResultItem;
class LibMusicBrainzApi;

/**
 * 搜索服务 - 封装搜索逻辑，作为UI和API之间的中介
 * 职责：
 * 1. 提供简化的搜索接口
 * 2. 管理搜索参数和结果
 * 3. 处理分页逻辑
 * 4. 缓存搜索结果
 */
class SearchService : public QObject
{
    Q_OBJECT

public:
    explicit SearchService(QObject *parent = nullptr);
    ~SearchService();

    // 主要搜索接口
    void search(const SearchParameters &params);
    void searchNextPage();
    void searchPrevPage();
    
    // 状态查询
    bool canGoNextPage() const;
    bool canGoPrevPage() const;
    int getCurrentPage() const;
    int getTotalPages() const;
    int getTotalResults() const;
    
    // 当前搜索参数
    SearchParameters getCurrentSearchParams() const;
    
public slots:
    void clearResults();

signals:
    void searchStarted(const QString &query);
    void searchCompleted(const QList<QSharedPointer<ResultItem>> &results, const SearchResults &stats);
    void searchFailed(const QString &error);
    void pageChanged(int currentPage, int totalPages);

private slots:
    void handleApiResults(const QList<QSharedPointer<ResultItem>> &results, int totalCount, int offset);
    void handleApiError(const QString &error);

private:
    void updatePageInfo();
    QString buildQueryString(const SearchParameters &params) const;
    
    LibMusicBrainzApi *m_api;
    SearchParameters m_currentParams;
    SearchResults m_currentResults;
    int m_currentPage;
    int m_totalPages;
    int m_itemsPerPage;
};

#endif // SEARCHSERVICE_H
