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

#include "datetime_range.h"

void DateTimeRange::setDate( const QDate & date, Elements elems ) {
    if ( !date.isValid() )
        return;

    if ( elems & Start ) {
        if ( start.isValid() )
            start.setDate( date );
        else
            start = KDateTime( date );
    }

    if ( elems & Finish ) {
        if ( finish.isValid() )
            finish.setDate( date );
        else
            finish = KDateTime( date );
    }
}

void DateTimeRange::setTime( const QTime & time, Elements elems ) {
    if ( !time.isValid() )
        return;

    if ( elems & Start ) {
        if ( !start.isValid() )
            start = KDateTime( QDate::currentDate() );

        start.setDateOnly( false );
        start.setTime( time );
    }

    if ( elems & Finish ) {
        if ( !finish.isValid() )
            finish = KDateTime( QDate::currentDate() );

        finish.setDateOnly( false );
        finish.setTime( time );
    }
}

void DateTimeRange::addSecs( int secs, Elements elems ) {
    if ( elems & Start ) start = start.addSecs( secs );
    if ( elems & Finish ) finish = finish.addSecs( secs );
}

void DateTimeRange::addDays( int days, Elements elems ) {
    if ( elems & Start ) start = start.addDays( days );
    if ( elems & Finish ) finish = finish.addDays( days );
}

void DateTimeRange::addMonths( int months, Elements elems ) {
    if ( elems & Start ) start = start.addMonths( months );
    if ( elems & Finish ) finish = finish.addMonths( months );
}

void DateTimeRange::addYears( int years, Elements elems ) {
    if ( elems & Start ) start = start.addYears( years );
    if ( elems & Finish ) finish = finish.addYears( years );
}

bool DateTimeRange::includes( const KDateTime & dt ) const {
    return dt >= start && dt <= finish;
}

bool DateTimeRange::intersects( const DateTimeRange & range ) const {
    return range.finish >= start && range.start <= finish;
}

bool DateTimeRange::intersects( const KDateTime & rangeStart, const KDateTime & rangeFinish ) const {
    return rangeFinish >= start && rangeStart <= finish;
}

