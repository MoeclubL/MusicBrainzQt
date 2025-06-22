#include <QtTest>
#include <QSignalSpy>
#include <QEventLoop>
#include "../src/api/musicbrainzapi.h"
#include "../src/core/types.h"

class TestMusicBrainzApi : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testSearchAlbum();
    void testSearchArtist();
    void testGetReleaseDetails();

private:
    MusicBrainzApi *api;
};

void TestMusicBrainzApi::initTestCase()
{
    api = new MusicBrainzApi();
}

void TestMusicBrainzApi::cleanupTestCase()
{
    delete api;
}

void TestMusicBrainzApi::testSearchAlbum()
{
    // 测试专辑搜索
    api->search("Abbey Road", EntityType::Release);
}

void TestMusicBrainzApi::testSearchArtist()
{
    // 测试艺术家搜索
    api->search("The Beatles", EntityType::Artist);
}

void TestMusicBrainzApi::testGetReleaseDetails()
{
    // 测试获取发布详情
    api->getDetails("1e0eee38-a9f6-49bf-84d0-45d0647799af", EntityType::Release);
}

QTEST_MAIN(TestMusicBrainzApi)
#include "tst_api.moc"
