#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <QString>
#include <QMap>

/**
 * 核心类型定义 - 所有层都可以使用的基础类型
 */

/**
 * 统一的MusicBrainz实体类型枚举
 * 用于搜索和结果展示
 */
enum class EntityType {
    Artist,
    Release,
    ReleaseGroup,
    Recording,
    Label,
    Work,
    Area,
    Place,
    Annotation,
    CDStub,
    Editor,
    Tag,
    Instrument,
    Series,
    Event,
    Documentation,
    Unknown
};

/**
 * 搜索参数结构
 */
struct SearchParameters {
    QString query;
    EntityType type;
    QMap<QString, QString> advancedFields;
    int limit = 25;
    int offset = 0;
    
    SearchParameters() : type(EntityType::Artist) {}
    
    bool isValid() const {
        return !query.isEmpty() && type != EntityType::Unknown;
    }
};

/**
 * 搜索结果统计
 */
struct SearchResults {
    int totalCount = 0;
    int offset = 0;
    int count = 0;
    
    SearchResults() = default;
    SearchResults(int total, int off, int cnt) 
        : totalCount(total), offset(off), count(cnt) {}
};

#endif // CORE_TYPES_H
