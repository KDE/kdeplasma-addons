/*
    SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <KRunner/AbstractRunnerTest>

#include <QDateTime>
#include <QTimeZone>
#include <math.h>

#include <qtestcase.h>

class DateTimeRunnerTest : public AbstractRunnerTest
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testLocalTimeInfo();
    void testRemoteTimeInfo();
    void testFindTimezones_data();
    void testFindTimezones();
};

#ifndef Q_OS_WIN
void initEnv()
{
    setenv("LC_ALL", "en_US.UTF-8", 1);
    setenv("TZ", "GMT", 1);
}
Q_CONSTRUCTOR_FUNCTION(initEnv)
#endif

void DateTimeRunnerTest::initTestCase()
{
    initProperties();
}

void DateTimeRunnerTest::testLocalTimeInfo()
{
    const QTime localTime = QDateTime::currentDateTime().time();
    const QString timeStr = QLocale().toString(localTime);

    launchQuery("time");

    QCOMPARE(manager->matches().count(), 1);
    QVERIFY(manager->matches().first().text().contains(timeStr));
}

void DateTimeRunnerTest::testRemoteTimeInfo()
{
    const QTime remoteTime = QDateTime::currentDateTime().toTimeZone(QTimeZone("UTC-02:00")).time();
    const QString timeStr = QLocale().toString(remoteTime, QLocale::ShortFormat);
    const QString timeDiffStr = QString("2 hours earlier");

    launchQuery("time gmt-2");
    auto matches = manager->matches();
    std::sort(matches.begin(), matches.end(), [](const Plasma::QueryMatch &a, const Plasma::QueryMatch &b) {
        return a.relevance() > b.relevance();
    });

    QCOMPARE(manager->matches().count(), 1);
    QVERIFY(manager->matches().first().text().contains(timeStr));
    QVERIFY(manager->matches().first().text().contains(timeDiffStr));
}

void DateTimeRunnerTest::testFindTimezones()
{
    QFETCH(QString, searchTerm);
    QFETCH(int, minMatchCount);
    QFETCH(QString, expectedTimezone);

    launchQuery("time " + searchTerm);
    auto matches = manager->matches();
    std::sort(matches.begin(), matches.end(), [](const Plasma::QueryMatch &a, const Plasma::QueryMatch &b) {
        return a.relevance() > b.relevance();
    });

    QVERIFY(matches.count() >= minMatchCount);
    QVERIFY(matches.first().text().contains(expectedTimezone));
}

void DateTimeRunnerTest::testFindTimezones_data()
{
    QTest::addColumn<QString>("searchTerm");
    QTest::addColumn<int>("minMatchCount");
    QTest::addColumn<QString>("expectedTimezone");

    QTest::newRow("Should find time zones by zone name") << "Harare" << 1 << "Africa/Harare";
    QTest::newRow("Should find time zones by long name") << "Central Africa Time" << 1 << "Central Africa Time";
    QTest::newRow("Should find time zones by short name") << "GMT+2" << 1 << "GMT+2";
    QTest::newRow("Should find time zones by offset name") << "UTC+02" << 1 << "UTC+02:00";
    QTest::newRow("Should find time zones by abbreviation, and show all time zones with that abbreviation") << "PST" << 2 << "(PST)";
    QTest::newRow("Should find time zones by country name, and show all time zones in that country") << "Mexico" << 2 << "Mexico/";
}

QTEST_MAIN(DateTimeRunnerTest)

#include "datetimerunnertest.moc"
