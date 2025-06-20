#ifndef MUSICBRAINZ_UTILS_H
#define MUSICBRAINZ_UTILS_H

#include <QString>
#include <array>
#include <string_view>
#include "../core/types.h"

/**
 * @namespace MusicBrainzUtils
 * @brief MusicBrainz工具函数命名空间
 * 
 * 包含所有与MusicBrainz实体类型转换、验证和处理相关的工具函数。
 * 使用C++20现代化特性提高性能和类型安全。
 */
namespace MusicBrainzUtils {

// =============================================================================
// C++20现代化常量定义
// =============================================================================

/**
 * @brief 使用constexpr数组存储实体类型映射，提高运行时性能
 */
struct EntityMapping {
    EntityType type;
    std::string_view singular;
    std::string_view plural;
    std::string_view defaultIncludes;
};

/**
 * @brief 编译时实体类型映射表
 */
constexpr std::array<EntityMapping, 16> ENTITY_MAPPINGS = {{
    {EntityType::Artist, "artist", "artists", "aliases+tags+ratings+genres+recordings+releases+release-groups+works"},
    {EntityType::Release, "release", "releases", "aliases+tags+ratings+genres+artists+labels+recordings+release-groups"},
    {EntityType::Recording, "recording", "recordings", "aliases+tags+ratings+genres+artists+releases+release-groups"},
    {EntityType::ReleaseGroup, "release-group", "release-groups", "aliases+tags+ratings+genres+artists+releases"},
    {EntityType::Label, "label", "labels", "aliases+tags+ratings+genres+releases"},
    {EntityType::Work, "work", "works", "aliases+tags+ratings+genres+artists+recordings"},
    {EntityType::Area, "area", "areas", "aliases+tags+ratings+genres"},
    {EntityType::Place, "place", "places", "aliases+tags+ratings+genres"},
    {EntityType::Annotation, "annotation", "annotations", "tags+ratings+genres"},
    {EntityType::CDStub, "cdstub", "cdstubs", "tags+ratings+genres"},
    {EntityType::Editor, "editor", "editors", "tags+ratings+genres"},
    {EntityType::Tag, "tag", "tags", "tags+ratings+genres"},
    {EntityType::Instrument, "instrument", "instruments", "aliases+tags+ratings+genres"},
    {EntityType::Series, "series", "series", "aliases+tags+ratings+genres"},
    {EntityType::Event, "event", "events", "aliases+tags+ratings+genres"},
    {EntityType::Documentation, "documentation", "documentation", "tags+ratings+genres"}
}};

// =============================================================================
// 现代化转换函数
// =============================================================================

/**
 * @brief 现代化的实体类型转换函数（使用constexpr查找表）
 * @param type 实体类型枚举值
 * @return 对应的字符串表示
 */
QString entityTypeToString(EntityType type);

/**
 * @brief 现代化的字符串转实体类型函数
 * @param typeStr 实体类型字符串
 * @return 对应的EntityType枚举值
 */
EntityType stringToEntityType(const QString &typeStr);

/**
 * @brief 获取实体的复数形式名称（使用constexpr查找）
 * @param type 实体类型
 * @return 复数形式名称（如"artists", "releases"）
 */
QString getEntityPluralName(EntityType type);

/**
 * @brief 获取实体类型的默认包含参数（编译时优化）
 * @param type 实体类型
 * @return 该实体类型的默认包含参数字符串
 */
QString getDefaultIncludes(EntityType type);

/**
 * @brief 构建MusicBrainz网站URL
 * @param entityId 实体ID（MBID）
 * @param type 实体类型
 * @return 完整的MusicBrainz网站URL
 * 
 * 统一的URL构建函数，用于所有UI组件中的"在浏览器中打开"功能。
 * 避免在多个UI组件中重复实现相同的URL构建逻辑。
 */
QString buildMusicBrainzUrl(const QString &entityId, EntityType type);

#if CONCEPTS_SUPPORTED
/**
 * @brief C++20版本：使用概念约束的查找函数
 * @param type 实体类型（必须是有效类型）
 * @return 实体映射信息
 */
template<ValidEntityType T = EntityType>
constexpr const EntityMapping* findEntityMapping(T type) noexcept {
    for (const auto& mapping : ENTITY_MAPPINGS) {
        if (mapping.type == type) {
            return &mapping;
        }
    }
    return nullptr;
}
#else
/**
 * @brief 传统版本：查找实体映射信息
 * @param type 实体类型
 * @return 实体映射信息
 */
constexpr const EntityMapping* findEntityMapping(EntityType type) noexcept {
    for (const auto& mapping : ENTITY_MAPPINGS) {
        if (mapping.type == type) {
            return &mapping;
        }
    }
    return nullptr;
}
#endif

}

#endif // MUSICBRAINZ_UTILS_H
