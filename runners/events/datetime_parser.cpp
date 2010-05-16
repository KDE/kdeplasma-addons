/*
 *   Copyright (C) 2010 Alexey Noskov <alexey.noskov@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "datetime_parser.h"

#include <KLocalizedString>
#include <QDebug>

// Phrase regular expressions
static const QRegExp inMinutes( i18nc( "In number of minutes phrase (may contain regexp symbols)", "in %1 minutes (after)?", "([+-]?\\d+)" ).replace(" ","\\s*") );
static const QRegExp inHours( i18nc( "In number of hours phrase (may contain regexp symbols)", "in %1 hours (after)?", "([+-]?\\d+)" ).replace(" ","\\s*") );
static const QRegExp inDays( i18nc( "In number of days phrase (may contain regexp symbols)", "in %1 days (after)?", "([+-]?\\d+)" ).replace(" ","\\s*") );
static const QRegExp inWeeks( i18nc( "In number of weeks phrase (may contain regexp symbols)", "in %1 weeks (after)?", "([+-]?\\d+)" ).replace(" ","\\s*") );
static const QRegExp inMonths( i18nc( "In number of months phrase (may contain regexp symbols)", "in %1 months (after)?", "([+-]?\\d+)" ).replace(" ","\\s*") );
static const QRegExp inYears( i18nc( "In number of years phrase (may contain regexp symbols)", "in %1 years (after)?", "([+-]?\\d+)" ).replace(" ","\\s*") );

// Keywords
static const QString now = i18nc( "Current time keyword", "now" );
static const QString today = i18nc( "Current day keyword", "today" );
static const QString tomorrow = i18nc( "Next day keyword", "tomorrow" );
static const QString yesterday = i18nc( "Previous day keyword", "yesterday" );

static const QString from = i18nc( "Keyword for start datetime", "from" ) + " ";
static const QString to = i18nc( "Keyword for finish datetime", "to" ) + " ";

DateTimeParser::DateTimeParser() {
    addTimeFormat( "h:mm" );
    
    addDateFormat( "d.M.yyyy" );
}

void DateTimeParser::addTimeFormat( const QString & s ) {
    if ( timeFormats.contains( s ) )
        return;
    
    QString formatRegexp = QRegExp::escape( s );
    
    formatRegexp.replace( QRegExp("hh|mm|ss"), "\\d\\d" );
    formatRegexp.replace( QRegExp("h|m|s"), "\\d\\d?" );
    formatRegexp.replace( "zzz", "\\d\\d\\d" );
    formatRegexp.replace( "z", "\\d\\d?\\d?" );
    formatRegexp.replace( QRegExp("AP|ap"), "\\w{2}" );
    
    timeFormats.insert( s, QRegExp( formatRegexp ) );
}

void DateTimeParser::addDateFormat( const QString & s ) {
    if ( dateFormats.contains( s ) )
        return;
    
    QString formatRegexp = QRegExp::escape( s );
        
    formatRegexp.replace( 'd', 'D' );
    formatRegexp.replace( "yyyy", "\\d\\d\\d\\d" );
    formatRegexp.replace( QRegExp("DDDD|MMMM"), "\\w+" );
    formatRegexp.replace( QRegExp("DDD|MMM"), "\\w{3}" );
    formatRegexp.replace( QRegExp("DD|MM|yy"), "\\d\\d" );
    formatRegexp.replace( QRegExp("D|M"), "\\d\\d?" );
    
    dateFormats.insert( s, QRegExp( formatRegexp ) );
}

DateTimeRange DateTimeParser::parseRange( const QString & s ) {
    DateTimeRange range;
    QString remaining = s.trimmed();
    DateTimeRange::Elements elems = DateTimeRange::Both;

    while ( !remaining.isEmpty() ) {
        if ( remaining.startsWith( from ) ) {
            elems = DateTimeRange::Start;
            remaining = remaining.mid( from.length() ).trimmed();
        } else if ( remaining.startsWith( to ) ) {
            elems = DateTimeRange::Finish;
            remaining = remaining.mid( to.length() ).trimmed();
        } else {
            remaining = parseElement( remaining, range, elems );
        }
    }

    return range;
}

QString DateTimeParser::parseElement( const QString & s, DateTimeRange & range, DateTimeRange::Elements elems, const QDate & defaultDate, const QTime & defaultTime ) {
    if ( s.startsWith( now ) ) {
        range.setDate( QDate::currentDate(), elems );
        range.setTime( QTime::currentTime(), elems );

        return s.mid( now.length() ).trimmed();
    } else if ( s.startsWith( today ) ) {
        range.setDate( QDate::currentDate(), elems );

        return s.mid( today.length() ).trimmed();
    } else if ( s.startsWith( tomorrow ) ) {
        range.setDate( QDate::currentDate().addDays( 1 ), elems );

        return s.mid( tomorrow.length() ).trimmed();
    } else if ( s.startsWith( yesterday ) ) {
        range.setDate( QDate::currentDate().addDays( -1 ), elems );

        return s.mid( yesterday.length() ).trimmed();
    }

    if ( inMinutes.indexIn( s ) == 0 ) {
        int value = inMinutes.cap( 1 ).toInt();
        QString rem = s.mid( inMinutes.matchedLength() ).trimmed();
        QString res = parseElement( rem, range, elems, QDate(), QTime::currentTime() );

        range.addSecs( value * 60, elems );

        return res;
    }

    if ( inHours.indexIn( s ) == 0 ) {
        int value = inHours.cap( 1 ).toInt();
        QString rem = s.mid( inHours.matchedLength() ).trimmed();
        QString res = parseElement( rem, range, elems, QDate(), QTime::currentTime() );

        range.addSecs( value * 3600, elems );

        return res;
    }

    if ( inDays.indexIn( s ) == 0 ) {
        int value = inDays.cap( 1 ).toInt();
        QString rem = s.mid( inDays.matchedLength() ).trimmed();
        QString res = parseElement( rem, range, elems, QDate::currentDate() );

        range.addDays( value, elems );

        return res;
    }

    if ( inWeeks.indexIn( s ) == 0 ) {
        int value = inWeeks.cap( 1 ).toInt();
        QString rem = s.mid( inWeeks.matchedLength() ).trimmed();
        QString res = parseElement( rem, range, elems, QDate::currentDate() );

        range.addDays( value * 7, elems );

        return res;
    }

    if ( inMonths.indexIn( s ) == 0 ) {
        int value = inMonths.cap( 1 ).toInt();
        QString rem = s.mid( inMonths.matchedLength() ).trimmed();
        QString res = parseElement( rem, range, elems, QDate::currentDate() );

        range.addMonths( value, elems );

        return res;
    }

    if ( inYears.indexIn( s ) == 0 ) {
        int value = inYears.cap( 1 ).toInt();
        QString rem = s.mid( inYears.matchedLength() ).trimmed();
        QString res = parseElement( rem, range, elems, QDate::currentDate() );

        range.addYears( value, elems );

        return res;
    }

    for ( FormatMap::iterator it = timeFormats.begin(); it != timeFormats.end(); ++ it ) {
        if ( it.value().indexIn( s ) == 0 ) {
            range.setTime( QTime::fromString( s.left( it.value().matchedLength() ), it.key() ), elems );

            return s.mid( it.value().matchedLength() ).trimmed();
        }
    }

    for ( FormatMap::iterator it = dateFormats.begin(); it != dateFormats.end(); ++ it ) {
        if ( it.value().indexIn( s ) == 0 ) {
            range.setDate( QDate::fromString( s.left( it.value().matchedLength() ), it.key() ), elems );

            return s.mid( it.value().matchedLength() ).trimmed();
        }
    }

    range.setDate( defaultDate, elems );
    range.setTime( defaultTime, elems );

    return "";
}

KDateTime DateTimeParser::parse( const QString& s ) {
    return parseRange( s ).start;
}
