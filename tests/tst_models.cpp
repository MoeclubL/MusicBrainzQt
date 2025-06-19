#include <QtTest>
#include <QCoreApplication>
#include "album.h"
#include "artist.h"
#include "release.h"
#include "track.h"

class TestModels : public QObject
{
    Q_OBJECT

private slots:
    void testAlbum();
    void testArtist();
    void testRelease();
    void testTrack();
};

void TestModels::testAlbum()
{
    Album album;
    album.setTitle("Test Album");
    album.setArtist("Test Artist");
    QCOMPARE(album.title(), QString("Test Album"));
    QCOMPARE(album.artist(), QString("Test Artist"));
}

void TestModels::testArtist()
{
    Artist artist;
    artist.setName("Test Artist");
    QCOMPARE(artist.name(), QString("Test Artist"));
}

void TestModels::testRelease()
{
    Release release;
    release.setTitle("Test Release");
    release.setDate("2023-01-01");
    QCOMPARE(release.title(), QString("Test Release"));
    QCOMPARE(release.date(), QString("2023-01-01"));
}

void TestModels::testTrack()
{
    Track track;
    track.setTitle("Test Track");
    track.setDuration(180);
    QCOMPARE(track.title(), QString("Test Track"));
    QCOMPARE(track.duration(), 180);
}

QTEST_MAIN(TestModels)
#include "tst_models.moc"