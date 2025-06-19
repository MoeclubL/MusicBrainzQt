#ifndef MUSICBRAINZTYPES_H
#define MUSICBRAINZTYPES_H

#include "../core/types.h"

/**
 * @file musicbrainztypes.h
 * @brief MusicBrainz API类型定义重定向文件
 * 
 * 此文件作为历史兼容性保留，现在重定向到统一的核心类型定义。
 * 在重构过程中，原本分散的类型定义已经整合到core/types.h中，
 * 以避免重复定义和依赖混乱。
 * 
 * **重构说明：**
 * - EntityType和相关类型统一定义在core/types.h
 * - 消除了API层和核心层的类型重复
 * - 简化了头文件依赖关系
 * - 提高了类型定义的一致性
 * 
 * @deprecated 此文件将在未来版本中移除，请直接使用core/types.h
 * @see core/types.h
 */

#endif // MUSICBRAINZTYPES_H
