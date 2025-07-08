#include "searchservice.h"
#include "../api/musicbrainzapi.h"
#include "../models/resultitem.h"
#include <QDebug>

SearchService::SearchService(QObject *parent)
    : QObject(parent)
    , m_api(new MusicBrainzApi(this))
    , m_currentPage(0)
    , m_totalPages(0)
    , m_itemsPerPage(25)
{
    // 连接API信号
    connect(m_api, &MusicBrainzApi::searchResultsReady,
            this, &SearchService::handleApiResults);
    connect(m_api, &MusicBrainzApi::errorOccurred,
            this, &SearchService::handleApiError);
}

SearchService::~SearchService()
{
}

void SearchService::search(const SearchParameters &params)
{
    if (!params.isValid()) {
        emit searchFailed(tr("Invalid search parameters"));
        return;
    }
    
    m_currentParams = params;
    m_currentPage = 0;
    m_itemsPerPage = params.limit; // 使用参数中的limit值
    
    QString queryString = buildQueryString(params);
    emit searchStarted(queryString);
    
    qDebug() << "SearchService: Starting search" << queryString 
             << "Type:" << static_cast<int>(params.type)
             << "Limit:" << m_itemsPerPage;
      // 直接使用核心类型，无需转换
    m_api->search(queryString, params.type, m_itemsPerPage, 0);
}

void SearchService::searchNextPage()
{
    if (!canGoNextPage()) {
        return;
    }
    
    m_currentPage++;
    int offset = m_currentPage * m_itemsPerPage;
      QString queryString = buildQueryString(m_currentParams);
    EntityType apiType = m_currentParams.type;
    
    qDebug() << "SearchService: Next page" << m_currentPage << "offset:" << offset;
    
    m_api->search(queryString, apiType, m_itemsPerPage, offset);
}

void SearchService::searchPrevPage()
{
    if (!canGoPrevPage()) {
        return;
    }
    
    m_currentPage--;
    int offset = m_currentPage * m_itemsPerPage;
      QString queryString = buildQueryString(m_currentParams);
    EntityType apiType = m_currentParams.type;
    
    qDebug() << "SearchService: Previous page" << m_currentPage << "offset:" << offset;
    
    m_api->search(queryString, apiType, m_itemsPerPage, offset);
}

bool SearchService::canGoNextPage() const
{
    return m_currentPage < m_totalPages - 1;
}

bool SearchService::canGoPrevPage() const
{
    return m_currentPage > 0;
}

int SearchService::getCurrentPage() const
{
    return m_currentPage;
}

int SearchService::getTotalPages() const
{
    return m_totalPages;
}

int SearchService::getTotalResults() const
{
    return m_currentResults.totalCount;
}

SearchParameters SearchService::getCurrentSearchParams() const
{
    return m_currentParams;
}



void SearchService::handleApiResults(const QList<QSharedPointer<ResultItem>> &results, int totalCount, int offset)
{
    m_currentResults = SearchResults(totalCount, offset, results.size());
    updatePageInfo();
    
    qDebug() << "SearchService: Received" << results.size() << "results, total:" << totalCount;
    
    emit searchCompleted(results, m_currentResults);
    emit pageChanged(m_currentPage, m_totalPages);
}

void SearchService::handleApiError(const QString &error)
{
    qDebug() << "SearchService: API error:" << error;
    emit searchFailed(error);
}

void SearchService::updatePageInfo()
{
    if (m_currentResults.totalCount > 0) {
        m_totalPages = (m_currentResults.totalCount + m_itemsPerPage - 1) / m_itemsPerPage;
        m_currentPage = m_currentResults.offset / m_itemsPerPage;
    } else {
        m_totalPages = 0;
        m_currentPage = 0;
    }
}

QString SearchService::buildQueryString(const SearchParameters &params) const
{
    if (params.advancedFields.isEmpty()) {
        // 简单搜索
        return params.query;
    }
    
    // 高级搜索 - 构建查询字符串
    QStringList queryParts;
    
    // 如果有基本查询，先添加
    if (!params.query.isEmpty()) {
        queryParts << params.query;
    }
    
    // 添加高级字段
    for (auto it = params.advancedFields.constBegin(); it != params.advancedFields.constEnd(); ++it) {
        QString value = it.value().trimmed();
        if (!value.isEmpty() && value != "Any" && value != "0") {
            // 如果值包含空格，用引号包围
            if (value.contains(' ') && !value.startsWith('"')) {
                value = "\"" + value + "\"";
            }
            queryParts << QString("%1:%2").arg(it.key(), value);
        }
    }
    
    return queryParts.join(" AND ");
}
