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
                           i18n("Displays the current date in a given timezone")));
    addSyntax(RunnerSyntax(timeWord + i18nc("The <> and space are part of the example query", " <timezone>"), //
                           i18n("Displays the current time in a given timezone")));
    addSyntax(
        RunnerSyntax(i18nc("The <> and space are part of the example query", "(<timezone>) (<date>) <time> (<timezone>)"), //
                     i18n("Converts the time from the first timezone to the second timezone. If not speicfied, the current time zone and date are used.")));
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
        addMatch(i18n("Today's date is %1", dateStr), dateStr, context, QStringLiteral("view-calendar-day"));
    } else if (term.startsWith(dateWord + QLatin1Char(' '), Qt::CaseInsensitive)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const auto zoneTerm = term.rightRef(term.length() - dateWord.length() - 1);
#else
        const auto zoneTerm = QStringView(term).right(term.length() - dateWord.length() - 1);
#endif
        const auto zones = matchingTimeZones(zoneTerm);
        for (auto it = zones.constBegin(), itEnd = zones.constEnd(); it != itEnd; ++it) {
            const QString zoneStr = it.key();
            const QDate date = QDateTime::currentDateTimeUtc().toTimeZone(*it).date();
            const QString dateStr = QLocale().toString(date);
            addMatch(QStringLiteral("%1 - %2").arg(zoneStr, dateStr), dateStr, context, QStringLiteral("view-calendar-day"));
        }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
        const QTime time = QTime::currentTime();
        const QString timeStr = QLocale().toString(time);
        addMatch(i18n("Current time is %1", timeStr), timeStr, context, QStringLiteral("clock"));
    } else if (term.startsWith(timeWord + QLatin1Char(' '), Qt::CaseInsensitive)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const auto zoneTerm = term.rightRef(term.length() - timeWord.length() - 1);
#else
        const auto zoneTerm = QStringView(term).right(term.length() - timeWord.length() - 1);
#endif
        const auto zones = matchingTimeZones(zoneTerm);
        for (auto it = zones.constBegin(), itEnd = zones.constEnd(); it != itEnd; ++it) {
            const QString zoneStr = it.key();
            const QDate date = QDateTime::currentDateTimeUtc().toTimeZone(*it).date();
            const QTime time = QDateTime::currentDateTimeUtc().toTimeZone(*it).time();
            const QString timeStr = QLocale().toString(time, QLocale::ShortFormat);

            const int dateDiff = QDateTime::currentDateTime().daysTo(QDateTime(date, time));
            const QString dateDiffStr = dateDiff ? QString().asprintf(" %+d d", dateDiff) : QString();

            const int timeDiffInMinutes = round((double)QDateTime::currentDateTime().secsTo(QDateTime(date, time)) / 60);
            const int timeDiffFullHours = (double)abs(timeDiffInMinutes) / 60;
            const int timeDiffFullMinutes = (double)abs(timeDiffInMinutes) - timeDiffFullHours * 60;
            const QString timeDiffNumStr = ((timeDiffFullHours ? QString("%1 h ").arg(timeDiffFullHours) : QString())
                                            + (timeDiffFullMinutes ? QString("%1 min ").arg(timeDiffFullMinutes) : QString()))
                                               .trimmed();
            const QString timeDiffStr =
                ((timeDiffInMinutes > 0 ? i18nc("time zone difference, e.g. in Stockholm it's 4 hours later than in Brasilia", "%1 later", timeDiffNumStr)
                      : timeDiffInMinutes < 0
                      ? i18nc("time zone difference, e.g. in Brasilia it's 4 hours ealier than in Stockholm", "%1 earlier", timeDiffNumStr)
                      : i18nc("no time zone difference, e.g. in Stockholm it's the same time as in Berlin", "no time difference")));
            addMatch(QStringLiteral("%1 - %2%3 (%4)").arg(zoneStr, timeStr, dateDiffStr, timeDiffStr), timeStr, context, QStringLiteral("clock"));
        }
    } else { // convert user-given time between user-given timezones
        const int minLen = QLocale::system().timeFormat(QLocale::ShortFormat).length();
        if (term.length() < minLen) {
            return; // query is too short to contain a time specification: not a match
        }
        QDate date;
        QTime time;
        QString dtTerm;
        // check all query substrings
        for (int i = 0; i < term.length() - minLen + 1 && time.isNull(); ++i) {
            for (int n = minLen; n < term.length() - i + 1 && time.isNull(); ++n) {
                dtTerm = term.mid(i, n);
                // try to parse substring as datetime or time
                if (QDateTime dateTimeParse = QLocale::system().toDateTime(dtTerm, QLocale::ShortFormat); dateTimeParse.isValid()) {
                    date = dateTimeParse.date();
                    time = dateTimeParse.time();
                } else if (QTime timeParse = QLocale::system().toTime(dtTerm, QLocale::ShortFormat); timeParse.isValid()) {
                    time = timeParse;
                    // unpsecified date will later be initialized to current date in the first timezone
                }
            }
        }
        if (time.isNull()) {
            return; // no valid time specification in the query: not a match
        }

        // time zone to convert from: left of time spec, otherwise default to current time zone
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QStringRef fromZoneTerm = term.leftRef(term.indexOf(dtTerm)).trimmed();
#else
        QStringView fromZoneMatch = QStringView(term).left(term.indexOf(dtMatch)).trimmed();
#endif
        QHash<QString, QTimeZone> fromZones = fromZoneTerm.isEmpty() ? systemTimeZone() : matchingTimeZones(fromZoneTerm, QDateTime(date, time));

        // time zone to convert to: right of time spec, otherwise default to current time zone
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QStringRef toZoneTerm = term.rightRef(term.length() - dtTerm.length() - term.indexOf(dtTerm)).trimmed();
#else
        QStringView toZoneMatch = QStringView(term).right(term.length() - dtMatch.length() - term.indexOf(dtMatch)).trimmed();
#endif
        QHash<QString, QTimeZone> toZones = toZoneTerm.isEmpty() ? systemTimeZone() : matchingTimeZones(toZoneTerm, QDateTime(date, time));

        if (fromZoneTerm.isEmpty() && toZoneTerm.isEmpty()) {
            return;
        }

        for (auto it = fromZones.constBegin(), itEnd = fromZones.constEnd(); it != itEnd; ++it) {
            const QString fromZoneStr = it.key();
            const QTimeZone fromZone = it.value();
            const QDate fromDate = date.isValid() ? date : QDateTime::currentDateTimeUtc().toTimeZone(fromZone).date();
            const QTime fromTime = time;
            const QDateTime fromDateTime = QDateTime(fromDate, fromTime, fromZone);
            const QString fromTimeStr = QLocale().toString(fromTime, QLocale::ShortFormat);

            for (auto jt = toZones.constBegin(), itEnd = toZones.constEnd(); jt != itEnd; ++jt) {
                const QString toZoneStr = jt.key();
                const QTimeZone toZone = jt.value();
                const QDateTime toDateTime = fromDateTime.toTimeZone(toZone);
                const QDate toDate = toDateTime.date();
                const QTime toTime = toDateTime.time();
                const QString toTimeStr = QLocale().toString(toTime, QLocale::ShortFormat);

                const int dateDiff = QDateTime(fromDate, fromTime).daysTo(QDateTime(toDate, toTime));
                const QString dateDiffStr = dateDiff ? QString().asprintf(" %+d d", dateDiff) : QString();

                const int timeDiffInMinutes = round((double)QDateTime(fromDate, fromTime).secsTo(QDateTime(toDate, toTime)) / 60);
                const int timeDiffFullHours = (double)abs(timeDiffInMinutes) / 60;
                const int timeDiffFullMinutes = (double)abs(timeDiffInMinutes) - timeDiffFullHours * 60;
                const QString timeDiffNumStr = ((timeDiffFullHours ? QString("%1 h ").arg(timeDiffFullHours) : QString())
                                                + (timeDiffFullMinutes ? QString("%1 min ").arg(timeDiffFullMinutes) : QString()))
                                                   .trimmed();
                const QString timeDiffStr =
                    ((timeDiffInMinutes > 0 ? i18nc("time zone difference, e.g. in Stockholm it's 4 hours later than in Brasilia", "%1 later", timeDiffNumStr)
                          : timeDiffInMinutes < 0
                          ? i18nc("time zone difference, e.g. in Brasilia it's 4 hours ealier than in Stockholm", "%1 earlier", timeDiffNumStr)
                          : i18nc("no time zone difference, e.g. in Stockholm it's the same time as in Berlin", "no time difference")));

                addMatch(QStringLiteral("%1 - %2%3 (%4)<br/>%5 - %6").arg(toZoneStr, toTimeStr, dateDiffStr, timeDiffStr, fromZoneStr, fromTimeStr),
                         toTimeStr,
                         context,
                         QStringLiteral("clock"));
            }
        }
    }
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QHash<QString, QTimeZone> DateTimeRunner::matchingTimeZones(const QStringRef &searchTerm, const QDateTime &atDateTime)
#else
QHash<QString, QTimeZone> DateTimeRunner::macthingTmeZones(const QStringView &searchTerm, const QDateTime &atDateTime)
#endif
{
    QHash<QString, QTimeZone> ret;
    for (const QByteArray &zoneId : QTimeZone::availableTimeZoneIds()) {
        QTimeZone timeZone(zoneId);
        const QDate atDate = atDateTime.date().isValid() ? atDateTime.date() : QDateTime::currentDateTimeUtc().toTimeZone(timeZone).date();
        const QTime atTime = atDateTime.time().isValid() ? atDateTime.time() : QDateTime::currentDateTimeUtc().toTimeZone(timeZone).time();
        for (const QString &zoneName : QStringList{QString::fromUtf8(zoneId),
                                                   timeZone.displayName(QDateTime(atDate, atTime), QTimeZone::LongName),
                                                   timeZone.displayName(QDateTime(atDate, atTime), QTimeZone::ShortName),
                                                   timeZone.displayName(QDateTime(atDate, atTime), QTimeZone::OffsetName),
                                                   timeZone.abbreviation(QDateTime(atDate, atTime)),
                                                   QLocale::countryToString(timeZone.country())}) {
            if (zoneName.contains(searchTerm, Qt::CaseInsensitive)) {
                ret.insert(zoneName, timeZone);
            }
        }
    }
    return ret;
}

QHash<QString, QTimeZone> DateTimeRunner::systemTimeZone()
{
    const QByteArray zoneId = QTimeZone::systemTimeZoneId();
    const QString zoneName = QString::fromUtf8(zoneId);
    return {{zoneName, QTimeZone(zoneId)}};
}

void DateTimeRunner::addMatch(const QString &text, const QString &clipboardText, RunnerContext &context, const QString &iconName)
{
    QueryMatch match(this);
    match.setText(text);
    match.setMultiLine(true);
    match.setData(clipboardText);
    match.setType(QueryMatch::InformationalMatch);
    match.setIconName(iconName);

    context.addMatch(match);
}

K_PLUGIN_CLASS_WITH_JSON(DateTimeRunner, "plasma-runner-datetime.json")

#include "datetimerunner.moc"
