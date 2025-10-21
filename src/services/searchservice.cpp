#include "searchservice.h"
#include "../api/musicbrainzapi.h"
#include "../models/resultitem.h"
#include <QDebug>
#include <algorithm>

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

void SearchService::search(const SearchParameters &params)
{
    if (!isValidSearchParams(params)) {
        emit searchFailed(tr("Invalid search parameters"));
        return;
    }

    m_currentParams = params;
    m_currentParams.offset = 0;
    m_currentPage = 0;
    m_totalPages = 0;
    m_currentResults = {};
    m_itemsPerPage = std::max(1, params.limit);
    m_cachedQueryString = buildQueryString(m_currentParams);

    emit searchStarted(m_cachedQueryString);

    qDebug() << "SearchService: Starting search" << m_cachedQueryString
             << "Type:" << static_cast<int>(m_currentParams.type)
             << "Limit:" << m_itemsPerPage;

    requestOffset(0);
}

void SearchService::searchNextPage()
{
    if (!canGoNextPage()) {
        return;
    }

    const int targetPage = m_currentPage + 1;
    const int offset = targetPage * m_itemsPerPage;

    qDebug() << "SearchService: Next page" << (targetPage + 1)
             << "offset:" << offset;

    requestOffset(offset);
}

void SearchService::searchPrevPage()
{
    if (!canGoPrevPage()) {
        return;
    }

    const int targetPage = std::max(0, m_currentPage - 1);
    const int offset = targetPage * m_itemsPerPage;

    qDebug() << "SearchService: Previous page" << (targetPage + 1)
             << "offset:" << offset;

    requestOffset(offset);
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
    return m_totalPages == 0 ? 0 : m_currentPage + 1;
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
    m_currentParams.offset = offset;
    updatePageInfo();

    qDebug() << "SearchService: Received" << results.size() << "results, total:" << totalCount;

    emit searchCompleted(results, m_currentResults);
    const int currentPageNumber = m_totalPages == 0 ? 0 : m_currentPage + 1;
    emit pageChanged(currentPageNumber, m_totalPages);
}

void SearchService::handleApiError(const QString &error)
{
    qDebug() << "SearchService: API error:" << error;
    emit searchFailed(error);
}

void SearchService::updatePageInfo()
{
    if (m_currentResults.totalCount > 0 && m_itemsPerPage > 0) {
        m_totalPages = (m_currentResults.totalCount + m_itemsPerPage - 1) / m_itemsPerPage;
        m_currentPage = m_currentResults.offset / m_itemsPerPage;
    } else {
        m_totalPages = 0;
        m_currentPage = 0;
    }
}

void SearchService::requestOffset(int offset)
{
    if (m_cachedQueryString.isEmpty()) {
        qWarning() << "SearchService: Attempted to request a page without an active query";
        return;
    }

    m_currentParams.offset = offset;
    m_api->search(m_cachedQueryString, m_currentParams.type, m_itemsPerPage, offset);
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
