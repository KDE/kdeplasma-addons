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

#include <KIcon>

#include <KDateTime>
#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KSystemTimeZones>
#include <KTimeZone>

#include <Plasma/Applet>

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
        const QString date = KGlobal::locale()->formatDate(QDate::currentDate());
        addMatch(i18n("Today's date is %1", date), date, context);
    } else if (term.startsWith(dateWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QDateTime dt = datetime(term, true, tzName);
        if (dt.isValid()) {
            const QString date = KGlobal::locale()->formatDate(dt.date());
            addMatch(i18n("The date in %1 is %2", tzName, date), date, context);
        }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
        const QString time = KGlobal::locale()->formatTime(QTime::currentTime());
        addMatch(i18n("The current time is %1", time), time, context);
    } else if (term.startsWith(timeWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QDateTime dt = datetime(term, true, tzName);
        if (dt.isValid()) {
            const QString time = KGlobal::locale()->formatTime(dt.time());
            addMatch(i18n("The current time in %1 is %2", tzName, time), time, context);
        }
    }
}

QDateTime DateTimeRunner::datetime(const QString &term, bool date, QString &tzName)
{
    QDateTime dt;
    const QString tz = term.right(term.length() - (date ? dateWord.length() : timeWord.length()) - 1);

    if (tz.compare(QLatin1String( "UTC" ), Qt::CaseInsensitive) == 0) {
        tzName = QLatin1String( "UTC" );
        dt = KDateTime::currentDateTime(KTimeZone::utc()).dateTime();
        return dt;
    }

    KTimeZones::ZoneMap zones = KSystemTimeZones::zones();
    QMapIterator<QString, KTimeZone> it(zones);
    while (it.hasNext()) {
        it.next();
        if (it.key().compare(tz, Qt::CaseInsensitive) == 0 ||
            it.value().name().compare(tz, Qt::CaseInsensitive) == 0) {
            tzName = it.value().name();
            dt = KDateTime::currentDateTime(it.value()).dateTime();
            break;
        } else if (!dt.isValid()) {
            if (it.key().contains(tz, Qt::CaseInsensitive) ||
                it.value().name().contains(tz, Qt::CaseInsensitive)) {
                tzName = it.value().name();
                dt = KDateTime::currentDateTime(it.value()).dateTime();
            } else {
                foreach (const QByteArray &abbrev, it.value().abbreviations()) {
                    if (QString( abbrev ).contains(tz, Qt::CaseInsensitive)) {
                        tzName = abbrev;
                        dt = KDateTime::currentDateTime(it.value()).dateTime();
                    }
                }
            }
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
    match.setIcon(KIcon(QLatin1String( "clock" )));

    QList<Plasma::QueryMatch> matches;
    matches << match;
    context.addMatches(context.query(), matches);
}

K_EXPORT_PLASMA_RUNNER(datetime, DateTimeRunner)

#include "datetimerunner.moc"

