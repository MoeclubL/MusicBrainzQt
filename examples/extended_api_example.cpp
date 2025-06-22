#include <QApplication>
#include <QDebug>
#include "../src/api/musicbrainzapi.h"
#include "../src/core/types.h"

/**
 * @file extended_api_example.cpp
 * @brief 展示扩展API功能的示例
 * 
 * 基于官方libmusicbrainz功能实现的新特性示例
 */

class ApiExample : public QObject
{
    Q_OBJECT

public:
    ApiExample(QObject *parent = nullptr) : QObject(parent)
    {
        // 初始化API
        api = new MusicBrainzApi(this);
        
        // 连接信号
        connect(api, &MusicBrainzApi::discIdLookupReady,
                this, &ApiExample::onDiscIdResults);
        connect(api, &MusicBrainzApi::genericQueryReady,
                this, &ApiExample::onGenericResults);
        connect(api, &MusicBrainzApi::browseResultsReady,
                this, &ApiExample::onBrowseResults);
        connect(api, &MusicBrainzApi::userCollectionsReady,
                this, &ApiExample::onCollectionsResults);
        connect(api, &MusicBrainzApi::collectionContentsReady,
                this, &ApiExample::onCollectionContents);
        connect(api, &MusicBrainzApi::collectionModified,
                this, &ApiExample::onCollectionModified);
        connect(api, &MusicBrainzApi::errorOccurred,
                this, &ApiExample::onError);
    }

public slots:
    void runExamples()
    {
        qDebug() << "=== 扩展MusicBrainz API功能示例 ===";
        qDebug() << "库版本:" << api->getVersion();
        
        // 示例1: DiscID查找
        qDebug() << "\n1. DiscID查找示例:";
        api->lookupDiscId("arIS30RPWowvwNEqsqdDnZzDGhk-");
        
        // 示例2: 通用查询
        qDebug() << "\n2. 通用查询示例 - 查询艺术家详细信息:";
        QStringList includes = {"aliases", "tags", "ratings", "recordings"};
        api->genericQuery("artist", "5b11f4ce-a62d-471e-81fc-a69a8278c7da", "", includes);
        
        // 示例3: 浏览功能
        qDebug() << "\n3. 浏览示例 - 查看艺术家的所有发行版:";
        api->browse("release", "artist", "5b11f4ce-a62d-471e-81fc-a69a8278c7da");
        
        // 示例4: 高级搜索
        qDebug() << "\n4. 高级搜索示例:";
        QVariantMap params;
        params["query"] = "artist:\"Nirvana\" AND release:\"Nevermind\"";
        params["limit"] = 5;
        api->genericQuery("release", "", "", QStringList(), params);
        
        // 认证相关功能需要用户凭据，这里只做演示
        qDebug() << "\n5. 集合管理示例 (需要认证):";
        qDebug() << "   设置认证信息...";
        api->setAuthentication("your_username", "your_password");
        
        qDebug() << "   获取用户集合...";
        // api->getUserCollections(); // 取消注释以实际运行
        
        qDebug() << "   查看集合内容...";
        // api->getCollectionContents("collection_id"); // 取消注释以实际运行
    }

private slots:
    void onDiscIdResults(const QList<QSharedPointer<ResultItem>> &releases, const QString &discId)
    {
        qDebug() << QString("DiscID查找结果 (%1): 找到 %2 个发行版").arg(discId).arg(releases.size());
        for (const auto &release : releases) {
            qDebug() << "  -" << release->getName() << "(" << release->getId() << ")";
        }
    }
    
    void onGenericResults(const QVariantMap &result, const QString &entity, const QString &id)
    {
        qDebug() << QString("通用查询结果 (实体: %1, ID: %2):").arg(entity, id);
        qDebug() << "  数据字段数:" << result.size();
        if (result.contains("name")) {
            qDebug() << "  名称:" << result["name"].toString();
        }
        if (result.contains("disambiguation")) {
            qDebug() << "  消歧义:" << result["disambiguation"].toString();
        }
    }
    
    void onBrowseResults(const QList<QSharedPointer<ResultItem>> &results, 
                        const QString &entity, int totalCount, int offset)
    {
        qDebug() << QString("浏览结果 (%1): %2/%3 (偏移: %4)")
                    .arg(entity).arg(results.size()).arg(totalCount).arg(offset);
        for (int i = 0; i < qMin(3, results.size()); ++i) {
            qDebug() << "  -" << results[i]->getName();
        }
        if (results.size() > 3) {
            qDebug() << "  ... 还有" << (results.size() - 3) << "个结果";
        }
    }
    
    void onCollectionsResults(const QList<QVariantMap> &collections)
    {
        qDebug() << "用户集合:" << collections.size() << "个";
        for (const auto &collection : collections) {
            qDebug() << "  -" << collection["name"].toString() 
                     << "(" << collection["id"].toString() << ")";
        }
    }
    
    void onCollectionContents(const QList<QSharedPointer<ResultItem>> &contents, 
                             const QString &collectionId)
    {
        qDebug() << QString("集合内容 (%1): %2 个条目").arg(collectionId).arg(contents.size());
        for (const auto &item : contents) {
            qDebug() << "  -" << item->getName();
        }
    }
    
    void onCollectionModified(bool success, const QString &collectionId, const QString &operation)
    {
        qDebug() << QString("集合修改 (%1 %2): %3")
                    .arg(operation, collectionId, success ? "成功" : "失败");
    }
    
    void onError(const QString &error)
    {
        qDebug() << "API错误:" << error;
        qDebug() << "HTTP状态码:" << api->getLastHttpCode();
        qDebug() << "错误信息:" << api->getLastErrorMessage();
    }

private:
    MusicBrainzApi *api;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ApiExample example;
    
    // 使用定时器启动示例，确保事件循环已启动
    QTimer::singleShot(100, &example, &ApiExample::runExamples);
    
    return app.exec();
}

#include "extended_api_example.moc"
