#include <QtTest>
#include <QSignalSpy>
#include <QEventLoop>
#include "../src/api/libmusicbrainzapi.h"
#include "../src/core/types.h"

class TestLibMusicBrainzApi : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testSearchAlbum();
    void testSearchArtist();
    void testGetReleaseDetails();

private:
    LibMusicBrainzApi *api;
};

void TestLibMusicBrainzApi::initTestCase()
{
    api = new LibMusicBrainzApi();
}

void TestLibMusicBrainzApi::cleanupTestCase()
{
    delete api;
}

void TestLibMusicBrainzApi::testSearchAlbum()
{
    // 测试专辑搜索
    api->search("Abbey Road", EntityType::Release);
}

void TestLibMusicBrainzApi::testSearchArtist()
{
    // 测试艺术家搜索
    api->search("The Beatles", EntityType::Artist);
}

void TestLibMusicBrainzApi::testGetReleaseDetails()
{
    // 测试获取发布详情
    api->getDetails("1e0eee38-a9f6-49bf-84d0-45d0647799af", EntityType::Release);
}

QTEST_MAIN(TestLibMusicBrainzApi)
#include "tst_api.moc"
