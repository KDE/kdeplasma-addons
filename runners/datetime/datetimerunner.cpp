/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2010 Marco MArtin <notmart@gmail.com>
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
        addMatch(i18n("Today's date is %1", date), date, context);
    } else if (term.startsWith(dateWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QString tz = term.right(term.length() - dateWord.length());
        QDateTime dt = datetime(tz, tzName);
        if (dt.isValid()) {
            const QString date = QLocale().toString(dt.date());
            addMatch(QString("%1 - %2").arg(tzName, date), date, context);
        }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
        const QString time = QLocale().toString(QTime::currentTime());
        addMatch(i18n("Current time is %1", time), time, context);
    } else if (term.startsWith(timeWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QString tz = term.right(term.length() - timeWord.length());
        QDateTime dt = datetime(tz, tzName);
        if (dt.isValid()) {
            const QString time = QLocale().toString(dt.time());
            addMatch(QString("%1 - %2").arg(tzName, time), time, context);
        }
    }
}

QDateTime DateTimeRunner::datetime(const QString &tz, QString &tzName)
{
    QDateTime dt;

    //
    // KTimeZone gives us the actual timezone names such as "Asia/Kolkatta" and does
    // not give us country info. QTimeZone does not give us the actual timezone name
    // This is why we are using both for now.
    //
    QList<QByteArray> timeZoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray& zoneId : timeZoneIds) {
        QTimeZone timeZone(zoneId);

        const QString zoneName = QString::fromUtf8(zoneId);
        if (zoneName.startsWith(tz, Qt::CaseInsensitive)) {
            tzName = zoneName;
            dt = QDateTime::currentDateTimeUtc().toTimeZone(timeZone);
            return dt;
        }

        const QString country = QLocale::countryToString(timeZone.country());
        if (country.startsWith(tz, Qt::CaseInsensitive)) {
            tzName = country;
            dt = QDateTime::currentDateTimeUtc().toTimeZone(timeZone);
            return dt;
        }

        // FIXME: This only includes the current abbreviation and not old abbreviation or
        // other possible names.
        // Eg - depending on the current date, only CET or CEST will work
        const QString abbr = timeZone.abbreviation(QDateTime::currentDateTime());
        if (abbr.startsWith(tz, Qt::CaseInsensitive)) {
            tzName = abbr;
            dt = QDateTime::currentDateTimeUtc().toTimeZone(timeZone);
            return dt;
        }
    }

    for (const QByteArray& zoneId : timeZoneIds) {
        QTimeZone timeZone(zoneId);

        const QString zoneName = QString::fromUtf8(zoneId);
        if (zoneName.contains(tz, Qt::CaseInsensitive)) {
            tzName = zoneName;
            dt = QDateTime::currentDateTimeUtc().toTimeZone(timeZone);
            return dt;
        }
    }

    return dt;
}

void DateTimeRunner::addMatch(const QString &text, const QString &clipboardText, Plasma::RunnerContext &context)
{
    Plasma::QueryMatch match(this);
    match.setText(text);
    match.setData(clipboardText);
    match.setType(Plasma::QueryMatch::InformationalMatch);
    match.setIcon(QIcon::fromTheme(QLatin1String( "clock" )));

    QList<Plasma::QueryMatch> matches;
    matches << match;
    context.addMatches(matches);
}

K_EXPORT_PLASMA_RUNNER(krunner_datetime, DateTimeRunner)

#include "datetimerunner.moc"

