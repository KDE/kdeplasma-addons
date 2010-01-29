#include "datetime_parser.h"

#include <KLocalizedString>
#include <QDebug>

QRegExp inMinutes("in\\s*([+-]?\\d+)\\s*minutes\\s*(from\\s+)?");
QRegExp inHours("in\\s*([+-]?\\d+)\\s*hours\\s*(from\\s+)?");
QRegExp inDays("in\\s*([+-]?\\d+)\\s*days\\s*(from\\s+)?");
QRegExp inWeeks("in\\s*([+-]?\\d+)\\s*weeks\\s*(from\\s+)?");
QRegExp inMonths("in\\s*([+-]?\\d+)\\s*months\\s*(from\\s+)?");
QRegExp inYears("in\\s*([+-]?\\d+)\\s*years\\s*(from\\s+)?");

DateTimeParser::DateTimeParser() {
    now = i18n("now");
    today = i18n("today");
    tomorrow = i18n("tomorrow");
    yesterday = i18n("yesterday");
    
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
    formatRegexp.replace( "AP", "(AM|PM)" );
    formatRegexp.replace( "ap", "(am|pm)" );
    
    timeFormats.insert( s, QRegExp( formatRegexp ) );
}

void DateTimeParser::addDateFormat( const QString & s ) {
    if ( dateFormats.contains( s ) )
        return;
    
    QString formatRegexp = QRegExp::escape( s );
        
    formatRegexp.replace( 'd', 'D' );
    formatRegexp.replace( "yyyy", "\\d\\d\\d\\d" );
    formatRegexp.replace( QRegExp("DDDD|MMMM"), "\\w+" );
    formatRegexp.replace( "DDD", "(Mon|Tue|Wed|Thu|Fri|Sat|Sun)" );
    formatRegexp.replace( "MMM", "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)" );
    formatRegexp.replace( QRegExp("DD|MM|yy"), "\\d\\d" );
    formatRegexp.replace( QRegExp("D|M"), "\\d\\d?" );
    
    dateFormats.insert( s, QRegExp( formatRegexp ) );
}

KDateTime DateTimeParser::parse( const QString& s ) {    
    if ( s.isEmpty() )
        return KDateTime();    
    
    if ( s.startsWith( now ) ) {
        return KDateTime::currentLocalDateTime();
    } else if ( s.startsWith( today ) ) {
        return merge( QDate::currentDate(), s, today.length() );
    } else if ( s.startsWith( tomorrow ) ) {
        return merge( QDate::currentDate().addDays( 1 ), s, tomorrow.length() );
    } else if ( s.startsWith( yesterday ) ) {
        return merge( QDate::currentDate().addDays( -1 ), s, yesterday.length() );
    }
    
    if ( inMinutes.indexIn( s ) == 0 )
        return parseOrNow( s.mid( inMinutes.matchedLength() ) ).addSecs( inMinutes.cap( 1 ).toInt() * 60 );
    
    if ( inHours.indexIn( s ) == 0 )
        return parseOrNow( s.mid( inHours.matchedLength() ) ).addSecs( inHours.cap( 1 ).toInt() * 3600 );
    
    if ( inDays.indexIn( s ) == 0 )
        return parseOrToday( s.mid( inDays.matchedLength() ) ).addDays( inDays.cap( 1 ).toInt() );
    
    if ( inWeeks.indexIn( s ) == 0 )
        return parseOrToday( s.mid( inWeeks.matchedLength() ) ).addDays( inWeeks.cap( 1 ).toInt() * 7 );
    
    if ( inMonths.indexIn( s ) == 0 )
        return parseOrToday( s.mid( inMonths.matchedLength() ) ).addMonths( inMonths.cap( 1 ).toInt() );
    
    if ( inYears.indexIn( s ) == 0 )
        return parseOrToday( s.mid( inYears.matchedLength() ) ).addYears( inYears.cap( 1 ).toInt() );
    
    for ( FormatMap::iterator it = timeFormats.begin(); it != timeFormats.end(); ++ it )
        if ( it.value().indexIn( s ) == 0 )
            return merge( QTime::fromString( s.left( it.value().matchedLength() ), it.key() ), s, it.value().matchedLength() );
    
    for ( FormatMap::iterator it = dateFormats.begin(); it != dateFormats.end(); ++ it )
        if ( it.value().indexIn( s ) == 0 )
            return merge( QDate::fromString( s.left( it.value().matchedLength() ), it.key() ), s, it.value().matchedLength() );
            
    return KDateTime();
}

KDateTime DateTimeParser::parseOrNow( const QString& s ) {
    KDateTime dt = parse( s );
    
    if ( !dt.isValid() )
        dt = KDateTime::currentLocalDateTime();
    
    return dt;
}

KDateTime DateTimeParser::parseOrToday( const QString& s ) {
    KDateTime dt = parse( s );
    
    if ( !dt.isValid() )
        dt = KDateTime( QDate::currentDate() );
    
    return dt;
}

KDateTime DateTimeParser::merge( const QDate & date, const QString & s, int offset ) {
    KDateTime dt = parse( s.mid( offset ).trimmed() );
    
    if ( dt.isValid() ) {
        dt.setDate( date );
        return dt;
    } else {
        return KDateTime( date );
    }
}

KDateTime DateTimeParser::merge( const QTime & time, const QString & s, int offset ) {
    KDateTime dt = parse( s.mid( offset ).trimmed() );
    
    if ( dt.isValid() ) {
        dt.setDateOnly( false );
        dt.setTime( time );
        return dt;
    } else {
        return KDateTime( QDate::currentDate(), time );
    }
}