/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>
 *   Copyright (C) 2015 Vishesh Handa <vhanda@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "datetimerunner.h"

#include <QLocale>
#include <QIcon>
#include <QTimeZone>

#include <KLocalizedString>

static const QString dateWord = i18nc("Note this is a KRunner keyword", "date");
static const QString timeWord = i18nc("Note this is a KRunner keyword", "time");

DateTimeRunner::DateTimeRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    setObjectName(QLatin1String( "DataTimeRunner" ));

    addSyntax(Plasma::RunnerSyntax(dateWord, i18n("Displays the current date")));
    addSyntax(Plasma::RunnerSyntax(dateWord + QLatin1String( " :q:" ), i18n("Displays the current date in a given timezone")));
    addSyntax(Plasma::RunnerSyntax(timeWord, i18n("Displays the current time")));
    addSyntax(Plasma::RunnerSyntax(timeWord + QLatin1String( " :q:" ), i18n("Displays the current time in a given timezone")));
}

DateTimeRunner::~DateTimeRunner()
{
}

void DateTimeRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.compare(dateWord, Qt::CaseInsensitive) == 0) {
        const QString date = QLocale().toString(QDate::currentDate());
        addMatch(i18n("Today's date is %1", date), date, context, QStringLiteral("view-calendar-day"));
    } else if (term.startsWith(dateWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        const auto tz = term.rightRef(term.length() - dateWord.length() - 1);
        const auto dates = datetime(tz);
        for(auto it = dates.constBegin(), itEnd = dates.constEnd(); it != itEnd; ++it) {
            const QString date = QLocale().toString(*it);
            addMatch(QStringLiteral("%1 - %2").arg(it.key(), date), date, context, QStringLiteral("view-calendar-day"));
        }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
        const QString time = QLocale().toString(QTime::currentTime());
        addMatch(i18n("Current time is %1", time), time, context, QStringLiteral("clock"));
    } else if (term.startsWith(timeWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        const auto tz = term.rightRef(term.length() - timeWord.length() - 1);
        const auto times = datetime(tz);
        for(auto it = times.constBegin(), itEnd = times.constEnd(); it != itEnd; ++it) {
            const QString time = QLocale().toString(*it, QLocale::ShortFormat);
            addMatch(QStringLiteral("%1 - %2").arg(it.key(), time), time, context, QStringLiteral("clock"));
        }
    }
}

QHash<QString, QDateTime> DateTimeRunner::datetime(const QStringRef& tz)
{
    QHash<QString, QDateTime> ret;
    //
    // KTimeZone gives us the actual timezone names such as "Asia/Kolkatta" and does
    // not give us country info. QTimeZone does not give us the actual timezone name
    // This is why we are using both for now.
    //
    const QList<QByteArray> timeZoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray& zoneId : timeZoneIds) {
        QTimeZone timeZone(zoneId);

        const QString zoneName = QString::fromUtf8(zoneId);
        if (zoneName.contains(tz, Qt::CaseInsensitive)) {
            ret[zoneName] = QDateTime::currentDateTimeUtc().toTimeZone(timeZone);
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

void DateTimeRunner::addMatch(const QString &text, const QString &clipboardText,
                              Plasma::RunnerContext &context, const QString& iconName)
{
    Plasma::QueryMatch match(this);
    match.setText(text);
    match.setData(clipboardText);
    match.setType(Plasma::QueryMatch::InformationalMatch);
    match.setIconName(iconName);

    context.addMatch(match);
}

K_EXPORT_PLASMA_RUNNER_WITH_JSON(DateTimeRunner, "plasma-runner-datetime.json")

#include "datetimerunner.moc"

