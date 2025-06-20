#include "musicbrainz_utils.h"
#include <algorithm>

namespace MusicBrainzUtils {

QString entityTypeToString(EntityType type)
{
    if (const auto* mapping = findEntityMapping(type)) {
        return QString::fromLatin1(mapping->singular.data(), mapping->singular.size());
    }
    return "unknown";
}

EntityType stringToEntityType(const QString &typeStr)
{
    const auto typeView = typeStr.toLatin1();
    
    auto it = std::find_if(ENTITY_MAPPINGS.begin(), ENTITY_MAPPINGS.end(),
        [&typeView](const EntityMapping& mapping) {
            return mapping.singular == typeView.data();
        });
    
    return (it != ENTITY_MAPPINGS.end()) ? it->type : EntityType::Unknown;
}

QString getEntityPluralName(EntityType type)
{
    if (const auto* mapping = findEntityMapping(type)) {
        return QString::fromLatin1(mapping->plural.data(), mapping->plural.size());
    }
    return "unknown";
}

QString getDefaultIncludes(EntityType type)
{
    if (const auto* mapping = findEntityMapping(type)) {
        return QString::fromLatin1(mapping->defaultIncludes.data(), mapping->defaultIncludes.size());
    }
    return "tags+ratings+genres";
}

QString buildMusicBrainzUrl(const QString &entityId, EntityType type)
{
    if (entityId.isEmpty()) {
        return QString();
    }
    
    QString typeStr = entityTypeToString(type);
    if (typeStr == "unknown") {
        return QString();
    }
    
    return QString("https://musicbrainz.org/%1/%2").arg(typeStr, entityId);
}

}
