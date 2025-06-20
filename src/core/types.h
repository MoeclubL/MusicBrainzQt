#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <QString>
#include <QMap>

#if __cplusplus >= 202002L
#include <concepts>
#include <string_view>
#define CONCEPTS_SUPPORTED 1
#else
#define CONCEPTS_SUPPORTED 0
#endif

// C++20特性：仅在支持的编译器中启用concepts

/**
 * @file types.h
 * @brief 核心类型定义 - MusicBrainzQt应用程序的基础数据结构
 * 
 * 此文件定义了应用程序中所有层都可以使用的基础类型和数据结构。
 * 这些类型确保了不同组件之间的一致性和类型安全。
 */

/**
 * @enum EntityType
 * @brief MusicBrainz实体类型枚举
 * 
 * 定义了MusicBrainz数据库中所有支持的实体类型。
 * 用于搜索、结果展示和API调用的类型识别。
 * 
 * **主要实体类型：**
 * - Artist: 艺术家/音乐人
 * - Release: 发行版本（专辑、单曲等）
 * - ReleaseGroup: 发行组（同一作品的不同版本集合）
 * - Recording: 录音/曲目
 * - Work: 作品（歌曲、乐曲的抽象概念）
 * - Label: 唱片公司/厂牌
 * 
 * **地理和场所类型：**
 * - Area: 地区（国家、城市等）
 * - Place: 场所（录音棚、演出场地等）
 * 
 * **辅助类型：**
 * - Annotation: 注释
 * - CDStub: CD占位符
 * - Editor: 编辑者
 * - Tag: 标签
 * - Instrument: 乐器
 * - Series: 系列
 * - Event: 事件
 * - Documentation: 文档
 * 
 * @see https://musicbrainz.org/doc/MusicBrainz_Database
 */
enum class EntityType {
    Artist,         ///< 艺术家实体
    Release,        ///< 发行版本实体
    ReleaseGroup,   ///< 发行组实体
    Recording,      ///< 录音实体
    Label,          ///< 厂牌实体
    Work,           ///< 作品实体
    Area,           ///< 地区实体
    Place,          ///< 场所实体
    Annotation,     ///< 注释实体
    CDStub,         ///< CD占位符实体
    Editor,         ///< 编辑者实体
    Tag,            ///< 标签实体
    Instrument,     ///< 乐器实体
    Series,         ///< 系列实体
    Event,          ///< 事件实体
    Documentation,  ///< 文档实体
    Unknown         ///< 未知类型（错误状态）
};

/**
 * @struct SearchParameters
 * @brief 搜索参数结构体
 * 
 * 封装了MusicBrainz搜索所需的所有参数，提供了统一的搜索接口。
 * 支持基本搜索和高级字段搜索功能。
 * 
 * **使用示例：**
 * ```cpp
 * SearchParameters params;
 * params.query = "The Beatles";
 * params.type = EntityType::Artist;
 * params.limit = 50;
 * params.advancedFields["country"] = "GB";
 * 
 * if (params.isValid()) {
 *     searchService->search(params);
 * }
 * ```
 * 
 * **高级字段示例：**
 * - Artist: "country", "gender", "type", "begin", "end"
 * - Release: "barcode", "format", "country", "date", "label"
 * - Recording: "isrc", "duration", "artist", "release"
 * 
 * @see EntityType, SearchService
 */
struct SearchParameters {
    QString query;                              ///< 主搜索查询字符串
    EntityType type;                            ///< 搜索的实体类型
    QMap<QString, QString> advancedFields;      ///< 高级搜索字段映射
    int limit = 25;                             ///< 结果数量限制（默认25）
    int offset = 0;                             ///< 结果偏移量（用于分页）
    
    /**
     * @brief 默认构造函数
     * 
     * 创建一个默认的搜索参数对象，实体类型设置为Artist。
     */
    SearchParameters() : type(EntityType::Artist) {}
    
    /**
     * @brief 验证搜索参数的有效性
     * @return true 如果参数有效（查询非空且类型不为Unknown）
     * 
     * 检查搜索参数是否满足最基本的要求：
     * - 查询字符串不能为空
     * - 实体类型不能为Unknown
     */
    bool isValid() const {
        return !query.isEmpty() && type != EntityType::Unknown;
    }
};

/**
 * @struct SearchResults
 * @brief 搜索结果统计信息
 * 
 * 存储搜索操作的统计数据，用于分页控制和结果展示。
 * 提供了搜索结果的完整统计信息。
 * 
 * **字段说明：**
 * - totalCount: 符合搜索条件的总结果数
 * - offset: 当前结果集的起始偏移量
 * - count: 当前返回的结果数量
 * 
 * **分页计算示例：**
 * ```cpp
 * SearchResults stats(150, 50, 25);  // 总共150个结果，从第50个开始，返回25个
 * int currentPage = (stats.offset / 25) + 1;  // 当前第3页
 * int totalPages = (stats.totalCount + 24) / 25;  // 总共6页
 * bool hasNextPage = (stats.offset + stats.count) < stats.totalCount;
 * ```
 */
struct SearchResults {
    int totalCount = 0;     ///< 搜索结果总数
    int offset = 0;         ///< 当前结果偏移量
    int count = 0;          ///< 当前返回的结果数量
    
    /**
     * @brief 默认构造函数
     * 
     * 创建一个空的搜索结果信息对象。
     */
    SearchResults() = default;
    
    /**
     * @brief 参数化构造函数
     * @param total 总结果数
     * @param off 偏移量
     * @param cnt 当前数量
     * 
     * 直接创建包含统计信息的搜索结果对象。
     */
    SearchResults(int total, int off, int cnt) 
        : totalCount(total), offset(off), count(cnt) {}
};

// =============================================================================
// C++20现代化工具函数
// =============================================================================

#if CONCEPTS_SUPPORTED
/**
 * @brief 概念：检查类型是否为有效的MusicBrainz实体类型
 */
template<typename T>
concept ValidEntityType = std::is_same_v<T, EntityType>;

/**
 * @brief 概念：检查是否为字符串类型
 */
template<typename T>
concept StringLike = std::convertible_to<T, QString> || std::convertible_to<T, std::string>;
#endif

/**
 * @brief 使用constexpr的现代化实体类型验证
 * @param type 实体类型
 * @return 是否为有效的实体类型
 */
constexpr bool isValidEntityType(EntityType type) noexcept {
    return type != EntityType::Unknown;
}

/**
 * @brief 现代化的搜索参数验证函数
 * @param params 搜索参数
 * @return 参数是否有效
 */
inline bool isValidSearchParams(const SearchParameters& params) noexcept {
    return !params.query.isEmpty() && 
           isValidEntityType(params.type) && 
           params.limit > 0 && 
           params.offset >= 0;
}

#endif // CORE_TYPES_H
