# MusicBrainzQt API Documentation

This document details how MusicBrainzQt integrates with the MusicBrainz Web Service API.

## 🌐 MusicBrainz API Overview

MusicBrainzQt uses the [MusicBrainz Web Service API v2](https://musicbrainz.org/doc/MusicBrainz_API) to access the MusicBrainz database. The API provides access to music metadata including artists, releases, recordings, works, and more.

### Base API Information
- **Base URL**: `https://musicbrainz.org/ws/2/`
- **Rate Limiting**: 1 request per second (strictly enforced)
- **User Agent**: Required for all requests
- **Format**: JSON (preferred) or XML

## 🏗️ API Integration Architecture

### LibMusicBrainzApi Class

The core API integration is handled by the `LibMusicBrainzApi` class located in `src/api/libmusicbrainzapi.h/.cpp`.

#### Key Features
- **Rate Limiting**: Automatic 1-second delays between requests
- **Error Handling**: Comprehensive error detection and reporting
- **User Agent**: Proper identification as required by MusicBrainz
- **JSON Parsing**: Robust parsing of API responses
- **Async Operations**: Non-blocking network requests

#### Usage Example
```cpp
#include "api/libmusicbrainzapi.h"

// Create API instance
auto api = QSharedPointer<LibMusicBrainzApi>::create();

// Search for artists
connect(api.get(), &LibMusicBrainzApi::searchCompleted,
        this, &MyClass::onSearchResults);
        
api->searchEntity(EntityType::Artist, "Beatles", 10, 0);
```

## 🔍 Supported Operations

### Search Operations

#### Artist Search
```cpp
api->searchEntity(EntityType::Artist, "artist name", limit, offset);
```

**API Endpoint**: `GET /artist/?query={query}&limit={limit}&offset={offset}&fmt=json`

**Response Fields**:
- `id`: MusicBrainz ID (UUID)
- `name`: Artist name
- `disambiguation`: Disambiguation comment
- `type`: Artist type (Person, Group, etc.)
- `country`: Country code
- `begin-area`: Birth/formation location
- `life-span`: Birth/formation and death/dissolution dates

#### Release Group Search
```cpp
api->searchEntity(EntityType::ReleaseGroup, "album name", limit, offset);
```

**API Endpoint**: `GET /release-group/?query={query}&limit={limit}&offset={offset}&fmt=json`

**Response Fields**:
- `id`: MusicBrainz ID (UUID)
- `title`: Release group title
- `disambiguation`: Disambiguation comment
- `primary-type`: Primary type (Album, Single, EP, etc.)
- `secondary-types`: Array of secondary types
- `artist-credit`: Array of credited artists
- `first-release-date`: Date of first release

#### Release Search
```cpp
api->searchEntity(EntityType::Release, "release name", limit, offset);
```

**API Endpoint**: `GET /release/?query={query}&limit={limit}&offset={offset}&fmt=json`

#### Recording Search
```cpp
api->searchEntity(EntityType::Recording, "song name", limit, offset);
```

**API Endpoint**: `GET /recording/?query={query}&limit={limit}&offset={offset}&fmt=json`

#### Work Search
```cpp
api->searchEntity(EntityType::Work, "work name", limit, offset);
```

**API Endpoint**: `GET /work/?query={query}&limit={limit}&offset={offset}&fmt=json`

### Detail Operations

#### Entity Detail Requests
When you have an MBID (MusicBrainz Identifier), you can request detailed information about a specific entity.

```cpp
api->getEntityDetails(EntityType::Artist, "5b11f4ce-a62d-471e-81fc-a69a8278c7da");
```

**API Endpoint**: `GET /{entity_type}/{mbid}?inc={includes}&fmt=json`

#### Supported Include Parameters

The `inc=` parameter allows you to request additional information about the entity. According to the MusicBrainz API documentation, different entity types support different includes:

##### Artist (`/artist/{mbid}`)
**Supported includes**:
- `recordings` - Recordings by this artist
- `releases` - Releases by this artist  
- `release-groups` - Release groups by this artist
- `works` - Works by this artist
- `aliases` - Alternative names for the artist
- `tags` - Tags applied to this artist
- `ratings` - User ratings for this artist
- `genres` - Genres applied to this artist
- Various relationship types: `artist-rels`, `release-rels`, etc.

**Default includes used by MusicBrainzQt**: `aliases+tags+ratings+genres+recordings+releases+release-groups+works`

##### Recording (`/recording/{mbid}`)
**Supported includes**:
- `artists` - Artists credited for this recording
- `releases` - Releases containing this recording
- `release-groups` - Release groups containing this recording
- `isrcs` - ISRC codes for this recording
- `url-rels` - URL relationships
- `aliases` - Alternative names
- `tags`, `ratings`, `genres` - Metadata

**Default includes used by MusicBrainzQt**: `aliases+tags+ratings+genres+artists+releases+release-groups+isrcs`

##### Release (`/release/{mbid}`)
**Supported includes**:
- `artists` - Artists credited for this release
- `labels` - Labels that released this
- `recordings` - Recordings on this release
- `release-groups` - Release group this belongs to
- `collections` - Collections containing this release
- `aliases`, `tags`, `ratings`, `genres` - Metadata

**Default includes used by MusicBrainzQt**: `aliases+tags+ratings+genres+artists+labels+recordings+release-groups`

##### Release Group (`/release-group/{mbid}`)
**Supported includes**:
- `artists` - Artists credited for this release group
- `releases` - Releases in this group
- `aliases`, `tags`, `ratings`, `genres` - Metadata

**Default includes used by MusicBrainzQt**: `aliases+tags+ratings+genres+artists+releases`

##### Label (`/label/{mbid}`)
**Supported includes**:
- `releases` - Releases on this label
- `aliases`, `tags`, `ratings`, `genres` - Metadata

**Default includes used by MusicBrainzQt**: `aliases+tags+ratings+genres+releases`

##### Work (`/work/{mbid}`)
**Supported includes**:
- `aliases`, `tags`, `ratings`, `genres` - Metadata
- Various relationship types

**Default includes used by MusicBrainzQt**: `aliases+tags+ratings+genres`

#### Rate Limiting
All API requests are subject to MusicBrainz's rate limiting:
- **Maximum**: 1 request per second
- **Enforcement**: Strictly enforced by the server
- **Implementation**: Automatic delays in LibMusicBrainzApi

## 🔧 API Implementation Details

#### Get Entity Details
```cpp
api->getEntityDetails(entityId, EntityType::Artist);
```

**API Endpoint**: `GET /{entity-type}/{id}?inc={includes}&fmt=json`

**Include Parameters**:
- `aliases`: Alternative names
- `tags`: User-submitted tags
- `genres`: Musical genres
- `artist-rels`: Artist relationships
- `release-group-rels`: Release group relationships
- `release-rels`: Release relationships
- `recording-rels`: Recording relationships
- `work-rels`: Work relationships
- `url-rels`: URL relationships

**Example Full URL**:
```
https://musicbrainz.org/ws/2/artist/b10bbbfc-cf9e-42e0-be17-e2c3e1d2600d?inc=aliases+tags+artist-rels+release-group-rels+url-rels&fmt=json
```

## 📊 Data Models

### ResultItem Class

The `ResultItem` class (`src/models/resultitem.h/.cpp`) serves as a unified container for all entity types returned by the API.

#### Core Properties
```cpp
class ResultItem {
private:
    QString m_id;                    // MusicBrainz ID
    QString m_name;                  // Entity name/title
    EntityType m_type;               // Entity type
    QString m_disambiguation;        // Disambiguation comment
    int m_score;                     // Search relevance score
    QVariantMap m_detailData;       // Full API response data
};
```

#### Supported Entity Types
```cpp
enum class EntityType {
    Artist,
    ReleaseGroup,
    Release,
    Recording,
    Work,
    Label,
    Area,
    Place,
    Instrument,
    Series,
    Event,
    Url
};
```

### Data Flow

1. **User Input** → Search query entered
2. **API Request** → LibMusicBrainzApi formats and sends request
3. **JSON Response** → MusicBrainz returns JSON data
4. **Parsing** → Response parsed into ResultItem objects
5. **Model Update** → ResultTableModel updated with new data
6. **UI Display** → EntityListWidget displays results
7. **Detail Loading** → User selects item, detailed data loaded
8. **Detail Display** → ItemDetailTab shows comprehensive information

## 🛡️ Error Handling

### HTTP Status Codes

The API handles various HTTP status codes:

- **200 OK**: Successful request
- **400 Bad Request**: Invalid query parameters
- **403 Forbidden**: Rate limit exceeded
- **404 Not Found**: Entity not found
- **503 Service Unavailable**: MusicBrainz server issues

### Error Response Structure

```cpp
struct ErrorInfo {
    int httpStatusCode;
    QString errorMessage;
    QString detailMessage;
    QDateTime timestamp;
};
```

### Rate Limiting

MusicBrainzQt implements strict rate limiting:

```cpp
// Ensure 1 second between requests
QTimer::singleShot(1000, [this, request]() {
    m_networkManager->get(request);
});
```

## 🔧 Configuration

### User Agent

MusicBrainzQt identifies itself with a proper user agent:

```cpp
const QString USER_AGENT = "MusicBrainzQt/1.0.0 ( https://github.com/MoeclubL/MusicBrainzQt )";
```

### Request Headers

Standard headers included with all requests:

```cpp
request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
```

## 📝 API Usage Examples

### Basic Search

```cpp
void MyWidget::searchForArtist(const QString &query)
{
    auto api = QSharedPointer<LibMusicBrainzApi>::create();
    
    connect(api.get(), &LibMusicBrainzApi::searchCompleted,
            this, [this](const QList<QSharedPointer<ResultItem>> &results) {
        // Handle search results
        for (const auto &item : results) {
            qDebug() << "Found artist:" << item->getName();
        }
    });
    
    connect(api.get(), &LibMusicBrainzApi::searchError,
            this, [this](const QString &error) {
        QMessageBox::warning(this, "Search Error", error);
    });
    
    api->searchEntity(EntityType::Artist, query, 25, 0);
}
```

### Loading Detailed Information

```cpp
void MyWidget::loadArtistDetails(const QString &artistId)
{
    auto api = QSharedPointer<LibMusicBrainzApi>::create();
    
    connect(api.get(), &LibMusicBrainzApi::entityDetailsLoaded,
            this, [this](QSharedPointer<ResultItem> item) {
        // Access detailed data
        QVariantMap details = item->getDetailData();
        
        // Extract aliases
        QVariantList aliases = details.value("aliases").toList();
        
        // Extract relationships
        QVariantList relations = details.value("relations").toList();
        
        // Extract tags
        QVariantList tags = details.value("tags").toList();
        
        // Update UI with detailed information
        updateDetailView(item);
    });
    
    api->getEntityDetails(artistId, EntityType::Artist);
}
```

### Handling Complex Relationships

```cpp
void MyWidget::processArtistRelationships(const QVariantMap &artistData)
{
    QVariantList relations = artistData.value("relations").toList();
    
    for (const QVariant &relationVar : relations) {
        QVariantMap relation = relationVar.toMap();
        QString type = relation.value("type").toString();
        QString direction = relation.value("direction").toString();
        
        if (type == "member of band") {
            QVariantMap band = relation.value("artist").toMap();
            QString bandName = band.value("name").toString();
            QString bandId = band.value("id").toString();
            
            // Process band membership
            addBandMembership(bandName, bandId);
        }
        else if (type == "collaboration") {
            // Process collaboration
            processCollaboration(relation);
        }
    }
}
```

## 🎯 Best Practices

### API Usage Guidelines

1. **Respect Rate Limits**: Never exceed 1 request per second
2. **Use Appropriate User Agent**: Always identify your application
3. **Handle Errors Gracefully**: Provide meaningful error messages to users
4. **Cache Results**: Store frequently accessed data locally
5. **Use Includes Wisely**: Only request data you actually need

### Performance Optimization

```cpp
// Cache frequently accessed entities
QMap<QString, QSharedPointer<ResultItem>> m_entityCache;

QSharedPointer<ResultItem> getCachedEntity(const QString &id) {
    if (m_entityCache.contains(id)) {
        return m_entityCache[id];
    }
    return nullptr;
}
```

### Error Recovery

```cpp
void LibMusicBrainzApi::handleNetworkError(QNetworkReply::NetworkError error)
{
    switch (error) {
        case QNetworkReply::TimeoutError:
            // Retry with exponential backoff
            retryRequest();
            break;
            
        case QNetworkReply::ServiceUnavailableError:
            // Show user-friendly message
            emit apiError("MusicBrainz service is temporarily unavailable");
            break;
            
        default:
            emit apiError("Network error occurred");
            break;
    }
}
```

## 🔍 Debugging API Issues

### Enable API Logging

```cpp
// In main.cpp or initialization code
QLoggingCategory::setFilterRules("musicbrainz.api.debug=true");
```

### Log Network Requests

```cpp
void LibMusicBrainzApi::logRequest(const QNetworkRequest &request)
{
    qCDebug(logAPI) << "Request URL:" << request.url().toString();
    qCDebug(logAPI) << "User Agent:" << request.header(QNetworkRequest::UserAgentHeader).toString();
    qCDebug(logAPI) << "Request time:" << QDateTime::currentDateTime().toString();
}
```

### Validate Response Data

```cpp
bool LibMusicBrainzApi::validateJsonResponse(const QJsonObject &json)
{
    // Check for required fields
    if (!json.contains("offset") || !json.contains("count")) {
        qCWarning(logAPI) << "Invalid API response: missing pagination fields";
        return false;
    }
    
    return true;
}
```

## 📚 API Reference Links

- [MusicBrainz API Documentation](https://musicbrainz.org/doc/MusicBrainz_API)
- [Rate Limiting Guidelines](https://musicbrainz.org/doc/MusicBrainz_API/Rate_Limiting)
- [Search Syntax](https://musicbrainz.org/doc/Indexed_Search_Syntax)
- [Entity Relationships](https://musicbrainz.org/doc/Relationship_Types)
- [JSON Web Service](https://musicbrainz.org/doc/MusicBrainz_API#JSON_Web_Service)

## 🚀 Future Enhancements

### Planned API Features

1. **Advanced Search**: Support for complex Lucene queries
2. **Batch Operations**: Multiple entity requests in single call
3. **Real-time Updates**: WebSocket support for live data updates
4. **Offline Mode**: Local database synchronization
5. **Custom Includes**: User-configurable include parameters

### API Extensions

1. **Cover Art Archive**: Album artwork integration
2. **Acoust-ID**: Audio fingerprinting support
3. **ListenBrainz**: Listening statistics integration
4. **Picard Integration**: Tag writing capabilities

---

This API documentation is maintained alongside the codebase. Please update it when making changes to the API integration layer.
