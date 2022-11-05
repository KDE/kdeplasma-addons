/*
 *   SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
 *   SPDX-FileCopyrightText: 2010 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
 *   SPDX-FileCopyrightText: 2022 Natalie Clarius <natalie_clarius@yahoo.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "datetimerunner.h"

#include <QFile>
#include <QIcon>
#include <QLocale>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimeZone>

#include <KLocalizedString>

#include <math.h>

static const QString dateWord = i18nc("Note this is a KRunner keyword", "date");
static const QString timeWord = i18nc("Note this is a KRunner keyword", "time");

DateTimeRunner::DateTimeRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : AbstractRunner(parent, metaData, args)
{
    setObjectName(QLatin1String("DataTimeRunner"));

    addSyntax(RunnerSyntax(dateWord, i18n("Displays the current date")));
    addSyntax(RunnerSyntax(timeWord, i18n("Displays the current time")));
    addSyntax(RunnerSyntax(dateWord + i18nc("The <> and space are part of the example query", " <timezone>"), //
                           i18n("Displays the current date in a given timezone")));
    addSyntax(RunnerSyntax(timeWord + i18nc("The <> and space are part of the example query", " <timezone>"), //
                           i18n("Displays the current time in a given timezone")));
    setTriggerWords({timeWord, dateWord});
    parseCityTZData();
}

DateTimeRunner::~DateTimeRunner()
{
}

void DateTimeRunner::match(RunnerContext &context)
{
    const QString term = context.query();
    if (term.compare(dateWord, Qt::CaseInsensitive) == 0) {
        const QString date = QLocale().toString(QDate::currentDate());
        addMatch(i18n("Today's date is %1", date), date, context, QStringLiteral("view-calendar-day"));
    } else if (term.startsWith(dateWord + QLatin1Char(' '), Qt::CaseInsensitive)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const auto tz = term.rightRef(term.length() - dateWord.length() - 1);
#else
        const auto tz = QStringView(term).right(term.length() - dateWord.length() - 1);
#endif
        const auto dates = datetime(tz);
        for (auto it = dates.constBegin(), itEnd = dates.constEnd(); it != itEnd; ++it) {
            const QString date = QLocale().toString(*it);
            addMatch(QStringLiteral("%1 - %2").arg(it.key(), date), date, context, QStringLiteral("view-calendar-day"));
        }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
        const QString time = QLocale().toString(QTime::currentTime());
        addMatch(i18n("Current time is %1", time), time, context, QStringLiteral("clock"));
    } else if (term.startsWith(timeWord + QLatin1Char(' '), Qt::CaseInsensitive)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const auto tz = term.rightRef(term.length() - timeWord.length() - 1);
#else
        const auto tz = QStringView(term).right(term.length() - timeWord.length() - 1);
#endif
        const auto times = datetime(tz);
        for (auto it = times.constBegin(), itEnd = times.constEnd(); it != itEnd; ++it) {
            const QString timeZone = it.key();
            const QString time = QLocale().toString(*it, QLocale::ShortFormat);

            const int timeDiffInMinutes = round((double)QDateTime::currentDateTime().secsTo(QDateTime(it.value().date(), it.value().time())) / 60);
            const int timeDiffHours = (double)abs(timeDiffInMinutes) / 60;
            const int timeDiffMinutes = (double)abs(timeDiffInMinutes) - timeDiffHours * 60;
            const QString timeDiff = (timeDiffHours ? QString("%1 h ").arg(timeDiffHours) : QString())
                + (timeDiffMinutes ? QString("%1 min ").arg(timeDiffMinutes) : QString())
                + ((timeDiffInMinutes > 0       ? i18nc("time zone difference", "later")
                        : timeDiffInMinutes < 0 ? i18nc("time zone difference", "earlier")
                                                : i18nc("no time zone difference", "no time difference")));
            addMatch(QStringLiteral("%1 - %2 (%3)").arg(timeZone, time, timeDiff), time, context, QStringLiteral("clock"));
        }
    }
}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QHash<QString, QDateTime> DateTimeRunner::datetime(const QStringRef &tz)
#else
QHash<QString, QDateTime> DateTimeRunner::datetime(const QStringView &tz)
#endif
{
    QHash<QString, QDateTime> ret;

    QStringList doneZones;
    for (const QStringList &cityRow : cityTZData) {
        if (cityRow[0].contains(tz, Qt::CaseInsensitive) || cityRow[1].contains(tz, Qt::CaseInsensitive)) {
            ret[cityRow[0]] = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone({cityRow[2].toUtf8()}));
            doneZones << cityRow[2];
        }
    }

    const QList<QByteArray> timeZoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray &zoneId : timeZoneIds) {
        QTimeZone timeZone(zoneId);

        const QString zoneName = QString::fromUtf8(zoneId);

        if (doneZones.contains(zoneName)) {
            // avoid things like "Berlin" (from city data) and "Europe/Berlin" (from zone IDs) both showing up
            continue;
        }

        if (zoneName.contains(tz, Qt::CaseInsensitive)) {
            ret[zoneName] = QDateTime::currentDateTimeUtc().toTimeZone(timeZone);
            continue;
        }

        const QString displayName = timeZone.displayName(QDateTime::currentDateTime(), QTimeZone::LongName);
        if (displayName.contains(tz, Qt::CaseInsensitive)) {
            ret[displayName] = QDateTime::currentDateTimeUtc().toTimeZone(timeZone);
            continue;
        }

        const QString country = QLocale::countryToString(timeZone.country());
        if (country.contains(tz, Qt::CaseInsensitive)) {
            ret[country] = QDateTime::currentDateTimeUtc().toTimeZone(timeZone);
            continue;
        }

        // FIXME: This only includes the current abbreviation and not old abbreviation or
        // other possible names.
        // Eg - depending on the current date, only CET or CEST will work
        const QString abbr = timeZone.abbreviation(QDateTime::currentDateTime());
        if (abbr.contains(tz, Qt::CaseInsensitive)) {
            ret[abbr] = QDateTime::currentDateTimeUtc().toTimeZone(timeZone);
            continue;
        }
    }

    return ret;
}

void DateTimeRunner::addMatch(const QString &text, const QString &clipboardText, RunnerContext &context, const QString &iconName)
{
    QueryMatch match(this);
    match.setText(text);
    match.setData(clipboardText);
    match.setType(QueryMatch::InformationalMatch);
    match.setIconName(iconName);

    context.addMatch(match);
}

void DateTimeRunner::parseCityTZData()
{
    QString databaseFilePath =
        QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("plasma/datetimerunner/majorcities.tsv"), QStandardPaths::LocateFile);
    if (databaseFilePath.isEmpty()) {
        return;
    }
    QFile databaseFile;
    databaseFile.setFileName(databaseFilePath);
    if (databaseFile.open(QFile::ReadOnly)) {
        QTextStream in(&databaseFile);
        QString line;
        while (in.readLineInto(&line)) {
            if (!line.isEmpty() && !line.startsWith(QLatin1Char('#'))) {
                cityTZData << line.split(QLatin1Char('\t'));
            }
        }
    }
}

K_PLUGIN_CLASS_WITH_JSON(DateTimeRunner, "plasma-runner-datetime.json")

#include "datetimerunner.moc"
