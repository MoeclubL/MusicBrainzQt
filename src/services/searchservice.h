#ifndef SEARCHSERVICE_H
#define SEARCHSERVICE_H

#include <QObject>
#include <QSharedPointer>
#include "../core/types.h"

class ResultItem;
class MusicBrainzApi;

/**
 * @class SearchService
 * @brief 搜索服务 - MusicBrainz搜索功能的核心服务层
 * 
 * SearchService作为UI层和API层之间的中介，提供了以下核心功能：
 * 
 * **主要职责：**
 * - 提供简化的搜索接口，隐藏API复杂性
 * - 管理搜索参数、状态和结果数据
 * - 处理分页逻辑和导航控制
 * - 统一的错误处理和状态通知
 * 
 * **使用模式：**
 * ```cpp
 * SearchService *service = new SearchService(this);
 * connect(service, &SearchService::searchCompleted, 
 *         this, &MainWindow::onSearchCompleted);
 * 
 * SearchParameters params;
 * params.query = "Beatles";
 * params.type = EntityType::Artist;
 * service->search(params);
 * ```
 * 
 * **分页支持：**
 * 支持前进/后退分页，自动管理页码状态和边界检查。
 * 
 * @author MusicBrainzQt Team
 * @see MusicBrainzApi, SearchParameters
 */
class SearchService : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象，用于Qt对象树管理
     */
    explicit SearchService(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 清理API连接和内部资源
     */
    ~SearchService();

    // =============================================================================
    // 主要搜索接口
    // =============================================================================
    
    /**
     * @brief 执行搜索操作
     * @param params 搜索参数，包含查询字符串、实体类型等
     * 
     * 发起新的搜索请求，会重置当前分页状态。
     * 搜索结果通过searchCompleted或searchFailed信号返回。
     */
    void search(const SearchParameters &params);
    
    /**
     * @brief 搜索下一页
     * 
     * 基于当前搜索参数获取下一页结果。
     * 需要先调用search()建立搜索上下文。
     */
    void searchNextPage();
    
    /**
     * @brief 搜索上一页
     * 
     * 基于当前搜索参数获取上一页结果。
     * 需要先调用search()建立搜索上下文。
     */
    void searchPrevPage();
    
    // =============================================================================
    // 状态查询接口
    // =============================================================================
    
    /**
     * @brief 检查是否可以向下翻页
     * @return true 如果存在下一页数据
     */
    bool canGoNextPage() const;
    
    /**
     * @brief 检查是否可以向上翻页
     * @return true 如果存在上一页数据
     */
    bool canGoPrevPage() const;
    
    /**
     * @brief 获取当前页码
     * @return 当前页码（从1开始）
     */
    int getCurrentPage() const;
    
    /**
     * @brief 获取总页数
     * @return 总页数，如果未进行搜索则返回0
     */
    int getTotalPages() const;
    
    /**
     * @brief 获取搜索结果总数
     * @return 符合条件的结果总数
     */
    int getTotalResults() const;
    
    /**
     * @brief 获取当前搜索参数
     * @return 最近一次搜索使用的参数
     */
    SearchParameters getCurrentSearchParams() const;    
public slots:
    /**
     * @brief 清空搜索结果和状态
     * 
     * 重置所有搜索相关的状态，包括结果数据、分页信息等。
     */
    void clearResults();

signals:
    /**
     * @brief 搜索开始信号
     * @param query 搜索查询字符串
     * 
     * 在搜索请求发送到API之前发出。
     */
    void searchStarted(const QString &query);
    
    /**
     * @brief 搜索完成信号
     * @param results 搜索结果列表
     * @param stats 搜索统计信息（总数、偏移量等）
     * 
     * 成功获取搜索结果时发出。
     */
    void searchCompleted(const QList<QSharedPointer<ResultItem>> &results, const SearchResults &stats);
    
    /**
     * @brief 搜索失败信号
     * @param error 错误描述信息
     * 
     * 搜索过程中发生错误时发出。
     */
    void searchFailed(const QString &error);
    
    /**
     * @brief 页面变化信号
     * @param currentPage 当前页码
     * @param totalPages 总页数
     * 
     * 分页状态发生变化时发出。
     */
    void pageChanged(int currentPage, int totalPages);

private slots:
    /**
     * @brief 处理API返回的搜索结果
     * @param results 结果项列表
     * @param totalCount 总结果数
     * @param offset 当前偏移量
     */
    void handleApiResults(const QList<QSharedPointer<ResultItem>> &results, int totalCount, int offset);
    
    /**
     * @brief 处理API错误
     * @param error 错误信息
     */
    void handleApiError(const QString &error);

private:
    /**
     * @brief 更新分页信息
     * 
     * 根据当前搜索结果更新页码、总页数等状态。
     */
    void updatePageInfo();
    
    /**
     * @brief 构建查询字符串
     * @param params 搜索参数
     * @return 格式化的查询字符串
     * 
     * 将搜索参数转换为MusicBrainz API可接受的查询格式。
     */
    QString buildQueryString(const SearchParameters &params) const;
    
    // =============================================================================
    // 成员变量
    // =============================================================================
    
    MusicBrainzApi *m_api;              ///< MusicBrainz API接口
    SearchParameters m_currentParams;       ///< 当前搜索参数
    SearchResults m_currentResults;         ///< 当前搜索结果统计
    int m_currentPage;                      ///< 当前页码（从1开始）
    int m_totalPages;                       ///< 总页数
    int m_itemsPerPage;                     ///< 每页项目数量
};

#endif // SEARCHSERVICE_H
