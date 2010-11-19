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

#ifndef DATETIME_RANGE_H
#define DATETIME_RANGE_H

#include <KDateTime>

class DateTimeRange {
public:
    enum Elements {
        Start = 1,
        Finish = 2,
        Both = 3
    };
public:
    DateTimeRange() {}
    DateTimeRange( const KDateTime & dateTime ) : start( dateTime ), finish( dateTime ) {}
    DateTimeRange( const KDateTime & start, const KDateTime & finish ) : start( start ), finish( finish ) {}

    bool isPoint() const {
        return start == finish;
    }

    bool isValid() const {
        return start.isValid() && finish.isValid() && start <= finish;
    }

    void setDate( const QDate & date, Elements elems );
    void setTime( const QTime & time, Elements elems );

    void addSecs( int secs, Elements elems );
    void addDays( int days, Elements elems );
    void addMonths( int months, Elements elems );
    void addYears( int years, Elements elems );

    bool includes( const KDateTime & dt ) const;
    bool intersects( const DateTimeRange & range ) const;
    bool intersects( const KDateTime & rangeStart, const KDateTime & rangeFinish ) const;
public:
    KDateTime start;
    KDateTime finish;
};

#endif
