/*
 *   SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
 *   SPDX-FileCopyrightText: 2010 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
 *   SPDX-FileCopyrightText: 2022 Natalie Clarius <natalie_clarius@yahoo.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "datetimerunner.h"

#include <QIcon>
#include <QLocale>
#include <QTimeZone>

#include <KFormat>
#include <KLocalizedString>

#include <math.h>

static const QString dateWord = i18nc("Note this is a KRunner keyword", "date");
static const QString timeWord = i18nc("Note this is a KRunner keyword", "time");

DateTimeRunner::DateTimeRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : AbstractRunner(parent, metaData, args)
{
    setObjectName(QLatin1String("DateTimeRunner"));

    addSyntax(RunnerSyntax(dateWord, i18n("Displays the current date")));
    addSyntax(RunnerSyntax(timeWord, i18n("Displays the current time")));
    addSyntax(RunnerSyntax(dateWord + i18nc("The <> and space are part of the example query", " <timezone>"), //
                           i18n("Displays the current date and difference to system date in a given timezone")));
    addSyntax(RunnerSyntax(timeWord + i18nc("The <> and space are part of the example query", " <timezone>"), //
                           i18n("Displays the current time and difference to system time in a given timezone")));
    setTriggerWords({timeWord, dateWord});
}

DateTimeRunner::~DateTimeRunner()
{
}

void DateTimeRunner::match(RunnerContext &context)
{
    const QString term = context.query();
    if (term.compare(dateWord, Qt::CaseInsensitive) == 0) {
        const QDate date = QDate::currentDate();
        const QString dateStr = QLocale().toString(date);
        addMatch(i18n("Today's date is %1", dateStr), dateStr, 1.0, QStringLiteral("view-calendar-day"), context);
    } else if (term.startsWith(dateWord + QLatin1Char(' '), Qt::CaseInsensitive)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const auto zoneTerm = term.rightRef(term.length() - dateWord.length() - 1);
#else
        const auto zoneTerm = QStringView(term).right(term.length() - dateWord.length() - 1);
#endif
        const auto zones = datetimeAt(zoneTerm);
        for (auto it = zones.constBegin(), itEnd = zones.constEnd(); it != itEnd; ++it) {
            const QString zoneStr = it.key();
            const QDateTime datetime = it.value();
            const QString dateStr = QLocale().toString(datetime.date());

            const qint64 dateDiff = QDateTime::currentDateTime().daysTo(QDateTime(datetime.date(), datetime.time())) * (24 * 60 * 60 * 1000); // full days in ms
            const QString dateDiffStr = dateDiff > 0 ? i18nc("date difference between time zones, e.g. in Stockholm it's 1 calendar day later than in Brasilia",
                                                             "%1 later",
                                                             KFormat().formatSpelloutDuration(abs(dateDiff)))
                : dateDiff < 0
                ? i18nc("date difference between time zones, e.g. in Brasilia it's 1 calendar day earlier than in Stockholm",
                        "%1 earlier",
                        KFormat().formatSpelloutDuration(abs(dateDiff)))
                : i18nc("no date difference between time zones, e.g. in Stockholm it's the same calendar day as in Berlin", "no date difference");

            addMatch(QStringLiteral("%1: %2 (%3)").arg(zoneStr, dateStr, dateDiffStr),
                     dateStr,
                     ((qreal)(zoneStr.count(zoneTerm, Qt::CaseInsensitive)) * zoneTerm.length() - (qreal)zoneStr.indexOf(zoneTerm, Qt::CaseInsensitive))
                         / zoneStr.length(),
                     QStringLiteral("view-calendar-day"),
                     context);
        }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
        const QTime time = QTime::currentTime();
        const QString timeStr = QLocale().toString(time);
        addMatch(i18n("Current time is %1", timeStr), timeStr, 1.0, QStringLiteral("clock"), context);
    } else if (term.startsWith(timeWord + QLatin1Char(' '), Qt::CaseInsensitive)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const auto zoneTerm = term.rightRef(term.length() - timeWord.length() - 1);
#else
        const auto zoneTerm = QStringView(term).right(term.length() - timeWord.length() - 1);
#endif
        const auto zones = datetimeAt(zoneTerm);
        for (auto it = zones.constBegin(), itEnd = zones.constEnd(); it != itEnd; ++it) {
            const QString zoneStr = it.key();
            const QDateTime datetime = it.value();
            const QString timeStr = QLocale().toString(datetime.time(), QLocale::ShortFormat);

            const qint64 dateDiff = QDateTime::currentDateTime().daysTo(QDateTime(datetime.date(), datetime.time())) * (24 * 60 * 60 * 1000); // full days in ms
            const QString dayDiffStr = dateDiff > 0 ? QString(" + %1").arg(KFormat().formatSpelloutDuration(abs(dateDiff)))
                : dateDiff < 0                      ? QString(" - %1").arg(KFormat().formatSpelloutDuration(abs(dateDiff)))
                                                    : QString();

            const qint64 timeDiff = round((double)QDateTime::currentDateTime().secsTo(QDateTime(datetime.date(), datetime.time())) / 60)
                * (60 * 1000); // time in ms rounded to the nearest full minutes
            const QString timeDiffStr = timeDiff > 0 ? i18nc("time difference between time zones, e.g. in Stockholm it's 4 hours later than in Brasilia",
                                                             "%1 later",
                                                             KFormat().formatSpelloutDuration(abs(timeDiff)))
                : timeDiff < 0                       ? i18nc("time difference between time zones, e.g. in Brasilia it's 4 hours ealier than in Stockholm",
                                       "%1 earlier",
                                       KFormat().formatSpelloutDuration(abs(timeDiff)))
                               : i18nc("no time difference between time zones, e.g. in Stockholm it's the same time as in Berlin", "no time difference");

            addMatch(QStringLiteral("%1: %2%3 (%4)").arg(zoneStr, timeStr, dayDiffStr, timeDiffStr),
                     timeStr,
                     ((qreal)(zoneStr.count(zoneTerm, Qt::CaseInsensitive)) * zoneTerm.length() - (qreal)zoneStr.indexOf(zoneTerm, Qt::CaseInsensitive))
                         / zoneStr.length(),
                     QStringLiteral("clock"),
                     context);
        }
    }
}

void DateTimeRunner::run(const RunnerContext &context, const QueryMatch &match)
{
    const QString clipboardText = match.data().toString();
    context.requestQueryStringUpdate(clipboardText, clipboardText.length());
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QHash<QString, QDateTime> DateTimeRunner::datetimeAt(const QStringRef &zoneTerm, const QDateTime referenceTime)
#else
QHash<QString, QDateTime> DateTimeRunner::datetimeAt(const QStringView &zoneTerm, const QDateTime referenceTime)
#endif
{
    QHash<QString, QDateTime> ret;
    const QList<QByteArray> timeZoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray &zoneId : timeZoneIds) {
        QTimeZone timeZone(zoneId);
        QDateTime datetime = referenceTime.toTimeZone(QTimeZone(zoneId));

        const QString country = QLocale::countryToString(timeZone.country());
        const QString comment = timeZone.comment();
        if (country.contains(zoneTerm, Qt::CaseInsensitive) || comment.contains(zoneTerm, Qt::CaseInsensitive)) {
            const QString regionName = comment.isEmpty() ? country : QString("%1/%2").arg(country, comment);
            ret[regionName] = datetime;
            continue;
        }

        // FIXME: This only includes the current abbreviation and not old abbreviation or other possible names.
        // Eg - depending on the current date, only CET or CEST will work
        const QString abbr = timeZone.abbreviation(datetime);
        if (abbr.contains(zoneTerm, Qt::CaseInsensitive)) {
            // Combine abbreviation with display name to disambiguate
            // Eg - Pacific Standard Time (PST) and Philippine Standard Time (PST)
            const QString display = timeZone.displayName(datetime);
            const QString abbrName = QString("%1 (%2)").arg(display, abbr);
            ret[abbrName] = datetime;
            continue;
        }

        const QString zoneName = QString::fromUtf8(zoneId).replace("_", " ");
        if (zoneName.contains(zoneTerm, Qt::CaseInsensitive)) {
            ret[zoneName] = datetime;
            continue;
        }

        const QString longName = timeZone.displayName(datetime, QTimeZone::LongName);
        if (longName.contains(zoneTerm, Qt::CaseInsensitive)) {
            ret[longName] = datetime;
            continue;
        }

        const QString shortName = timeZone.displayName(datetime, QTimeZone::ShortName);
        if (shortName.contains(zoneTerm, Qt::CaseInsensitive)) {
            ret[shortName] = datetime;
            continue;
        }

        const QString offsetName = timeZone.displayName(datetime, QTimeZone::OffsetName);
        if (offsetName.contains(zoneTerm, Qt::CaseInsensitive)) {
            ret[offsetName] = datetime;
            continue;
        }
    }

    return ret;
}

void DateTimeRunner::addMatch(const QString &text, const QString &clipboardText, const qreal &relevance, const QString &iconName, RunnerContext &context)
{
    QueryMatch match(this);
    match.setText(text);
    match.setData(clipboardText);
    match.setType(QueryMatch::HelperMatch);
    match.setRelevance(relevance);
    match.setIconName(iconName);
    match.setMultiLine(true);

    context.addMatch(match);
}

K_PLUGIN_CLASS_WITH_JSON(DateTimeRunner, "plasma-runner-datetime.json")

#include "datetimerunner.moc"
