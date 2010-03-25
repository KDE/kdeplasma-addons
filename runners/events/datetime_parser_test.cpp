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

#include "datetime_parser_test.h"

void DateTimeParserTest::testSimpleKeywords() {
    QVERIFY( KDateTime::currentLocalDateTime() == parser.parse("now") );
    QVERIFY( KDateTime( KDateTime::currentLocalDate() ) == parser.parse("today") );
    QVERIFY( KDateTime( KDateTime::currentLocalDate().addDays( 1 ) ) == parser.parse("tomorrow") );
    QVERIFY( KDateTime( KDateTime::currentLocalDate().addDays( -1 ) ) == parser.parse("yesterday") );
}

void DateTimeParserTest::testRelativeKeywords() {
    QVERIFY( KDateTime( KDateTime::currentLocalDate().addDays( 5 ) ) == parser.parse("in 5 days") );
    QVERIFY( KDateTime( KDateTime::currentLocalDate().addMonths( 2 ) ) == parser.parse("in 2 months") );
    QVERIFY( KDateTime( KDateTime::currentLocalDate().addYears( 3 ) ) == parser.parse("in 3 years") );
    QVERIFY( KDateTime( KDateTime::currentLocalDate().addDays( -1 ).addYears( 3 ) ) == parser.parse("in 3 years after yesterday") );
}

void DateTimeParserTest::testPreciseSpecs() {
    QVERIFY( KDateTime( QDate::fromString( "21.10.2009", "d.M.yyyy" ) ) == parser.parse("21.10.2009") );
}

void DateTimeParserTest::testPointRanges() {
    DateTimeRange r1 = parser.parseRange("21.10.2009");
    QVERIFY( r1.isPoint() );
    QVERIFY( r1.start == KDateTime( QDate::fromString( "21.10.2009", "d.M.yyyy" ) ) );

    DateTimeRange r2 = parser.parseRange("21.10.2009");
    QVERIFY( r2.isPoint() );
    QVERIFY( r2.start == KDateTime( QDate::fromString( "21.10.2009", "d.M.yyyy" ) ) );
}

void DateTimeParserTest::testNonPointRanges() {
    DateTimeRange r1 = parser.parseRange("from today to tomorrow");
    QVERIFY( r1.start == KDateTime( KDateTime::currentLocalDate() ) );
    QVERIFY( r1.finish == KDateTime( KDateTime::currentLocalDate().addDays( 1 ) ) );

    DateTimeRange r2 = parser.parseRange("today from 12:00 to 13:00");
    QVERIFY( r2.start == KDateTime( KDateTime::currentLocalDate(), QTime::fromString("12:00","H:m") ) );
    QVERIFY( r2.finish == KDateTime( KDateTime::currentLocalDate(), QTime::fromString("13:00","H:m") ) );
}

QTEST_MAIN(DateTimeParserTest)
