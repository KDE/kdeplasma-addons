/*
    SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <KRunner/AbstractRunnerTest>

#include <QDateTime>
#include <QTimeZone>

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
    void testConversion_data();
    void testConversion();
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
    std::sort(matches.begin(), matches.end(), [](const KRunner::QueryMatch &a, const KRunner::QueryMatch &b) {
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
    std::sort(matches.begin(), matches.end(), [](const KRunner::QueryMatch &a, const KRunner::QueryMatch &b) {
        return a.relevance() > b.relevance();
    });

    QVERIFY2(matches.size() >= minMatchCount,
             QLatin1String("searchTerm: %1, matches.size(): %2, minMatchCount: %3")
                 .arg(searchTerm, QString::number(matches.size()), QString::number(minMatchCount))
                 .toLatin1()
                 .constData());
    QVERIFY2(matches.first().text().contains(expectedTimezone),
             QLatin1String("first match: %1, expectedTimezone: %2").arg(matches.first().text(), expectedTimezone).toLatin1().constData());
}

void DateTimeRunnerTest::testFindTimezones_data()
{
    QTest::addColumn<QString>("searchTerm");
    QTest::addColumn<int>("minMatchCount");
    QTest::addColumn<QString>("expectedTimezone");

    QTest::newRow("Should find time zones by city name") << "Harare" << 1 << "Harare";
    QTest::newRow("Should find time zones by long name") << "Central Africa Time" << 1 << "Central Africa Time";
    QTest::newRow("Should find time zones by short name") << "GMT+2" << 1 << "GMT+2";
    QTest::newRow("Should find time zones by offset name") << "UTC+02:00" << 1 << "UTC+02:00";
    QTest::newRow("Should find time zones by abbreviation, and show all time zones with that abbreviation") << "PST" << 1 << "(PST)";
    QTest::newRow("Should find time zones by country name, and show all time zones in that country") << "Brazil" << 2 << "Brazil - ";
    QTest::newRow("Should find time zones with the 'in' keyword") << "in Harare" << 1 << "Harare";
}

void DateTimeRunnerTest::testConversion()
{
    QFETCH(QString, query);
    QFETCH(QString, expectedSourceTimezone);
    QFETCH(QString, expectedSourceTime);
    QFETCH(QString, expectedTargetTimezone);
    QFETCH(QString, expectedTargetTime);
    QFETCH(QString, expectedTimeDiff);

    launchQuery(query);
    auto matches = manager->matches();
    std::sort(matches.begin(), matches.end(), [](const KRunner::QueryMatch &a, const KRunner::QueryMatch &b) {
        return a.relevance() > b.relevance();
    });

    QVERIFY(!matches.isEmpty());
    qDebug() << matches.cbegin()->text() << expectedTargetTimezone << expectedTargetTime << expectedSourceTimezone << expectedSourceTime;
    QVERIFY(matches.first().text().contains(QString("%1: %2").arg(expectedTargetTimezone, expectedTargetTime)));
    QVERIFY(matches.first().text().contains(QString("%1: %2").arg(expectedSourceTimezone, expectedSourceTime)));
    QVERIFY(matches.first().text().contains(expectedTimeDiff));
}

void DateTimeRunnerTest::testConversion_data()
{
    const QTimeZone systemTimeZone = QTimeZone::systemTimeZone().isValid() ? QTimeZone::systemTimeZone() : QTimeZone::utc(); // needed for FreeBSD CI
    const QString systemTimeZoneName = systemTimeZone.abbreviation(QDateTime::currentDateTime());

    QTest::addColumn<QString>("query");
    QTest::addColumn<QString>("expectedSourceTimezone");
    QTest::addColumn<QString>("expectedSourceTime");
    QTest::addColumn<QString>("expectedTargetTimezone");
    QTest::addColumn<QString>("expectedTargetTime");
    QTest::addColumn<QString>("expectedTimeDiff");

    QTest::newRow("Should convert from first to second time zone")
        << "12:00 PM gmt-2 in greenwich"
        << "GMT-2" << QStringLiteral("12:00 PM") << "Greenwich Mean Time" << QStringLiteral("2:00 PM") << "2 hours later";
    QTest::newRow("Should convert from first to second time zone with date")
        << "1/5/23 12:00 PM gmt-2 in greenwich"
        << "GMT-2" << QStringLiteral("12:00 PM") << "Greenwich Mean Time" << QStringLiteral("2:00 PM") << "2 hours later";
    QTest::newRow("Should convert from system time zone to second time zone")
        << "12:00 PM in gmt+2" << systemTimeZoneName << QStringLiteral("12:00 PM") << "GMT+2" << QStringLiteral("2:00 PM") << "2 hours later";
    QTest::newRow("Should convert from first time zone to system time zone")
        << "12:00 PM gmt+2"
        << "GMT+2" << QStringLiteral("12:00 PM") << systemTimeZoneName << QStringLiteral("10:00 AM") << "2 hours earlier";
}

QTEST_MAIN(DateTimeRunnerTest)

#include "datetimerunnertest.moc"
