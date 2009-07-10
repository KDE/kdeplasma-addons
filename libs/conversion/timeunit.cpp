/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "timeunit.h"
#include <KLocale>

Time::Time(QObject* parent)
: Conversion::UnitCategory(parent)
{
    setObjectName("time");
    setName(i18n("Time"));

    U(Time::Yottasecond, 1e+24,
      i18nc("time unit symbol", "Ys"),
      i18nc("unit description in lists", "yottaseconds"),
      i18nc("unit synonyms for matching user input", "yottasecond;yottaseconds;Ys"),
      ki18nc("amount in units (real)", "%1 yottaseconds"),
      ki18ncp("amount in units (integer)", "%1 yottasecond", "%1 yottaseconds")
    );
    U(Time::Zettasecond, 1e+21,
      i18nc("time unit symbol", "Zs"),
      i18nc("unit description in lists", "zettaseconds"),
      i18nc("unit synonyms for matching user input", "zettasecond;zettaseconds;Zs"),
      ki18nc("amount in units (real)", "%1 zettaseconds"),
      ki18ncp("amount in units (integer)", "%1 zettasecond", "%1 zettaseconds")
    );
    U(Time::Exasecond, 1e+18,
      i18nc("time unit symbol", "Es"),
      i18nc("unit description in lists", "exaseconds"),
      i18nc("unit synonyms for matching user input", "exasecond;exaseconds;Es"),
      ki18nc("amount in units (real)", "%1 exaseconds"),
      ki18ncp("amount in units (integer)", "%1 exasecond", "%1 exaseconds")
    );
    U(Time::Petasecond, 1e+15,
      i18nc("time unit symbol", "Ps"),
      i18nc("unit description in lists", "petaseconds"),
      i18nc("unit synonyms for matching user input", "petasecond;petaseconds;Ps"),
      ki18nc("amount in units (real)", "%1 petaseconds"),
      ki18ncp("amount in units (integer)", "%1 petasecond", "%1 petaseconds")
    );
    U(Time::Terasecond, 1e+12,
      i18nc("time unit symbol", "Ts"),
      i18nc("unit description in lists", "teraseconds"),
      i18nc("unit synonyms for matching user input", "terasecond;teraseconds;Ts"),
      ki18nc("amount in units (real)", "%1 teraseconds"),
      ki18ncp("amount in units (integer)", "%1 terasecond", "%1 teraseconds")
    );
    U(Time::Gigasecond, 1e+09,
      i18nc("time unit symbol", "Gs"),
      i18nc("unit description in lists", "gigaseconds"),
      i18nc("unit synonyms for matching user input", "gigasecond;gigaseconds;Gs"),
      ki18nc("amount in units (real)", "%1 gigaseconds"),
      ki18ncp("amount in units (integer)", "%1 gigasecond", "%1 gigaseconds")
    );
    U(Time::Megasecond, 1e+06,
      i18nc("time unit symbol", "Ms"),
      i18nc("unit description in lists", "megaseconds"),
      i18nc("unit synonyms for matching user input", "megasecond;megaseconds;Ms"),
      ki18nc("amount in units (real)", "%1 megaseconds"),
      ki18ncp("amount in units (integer)", "%1 megasecond", "%1 megaseconds")
    );
    U(Time::Kilosecond, 1000,
      i18nc("time unit symbol", "ks"),
      i18nc("unit description in lists", "kiloseconds"),
      i18nc("unit synonyms for matching user input", "kilosecond;kiloseconds;ks"),
      ki18nc("amount in units (real)", "%1 kiloseconds"),
      ki18ncp("amount in units (integer)", "%1 kilosecond", "%1 kiloseconds")
    );
    U(Time::Hectosecond, 100,
      i18nc("time unit symbol", "hs"),
      i18nc("unit description in lists", "hectoseconds"),
      i18nc("unit synonyms for matching user input", "hectosecond;hectoseconds;hs"),
      ki18nc("amount in units (real)", "%1 hectoseconds"),
      ki18ncp("amount in units (integer)", "%1 hectosecond", "%1 hectoseconds")
    );
    U(Time::Decasecond, 10,
      i18nc("time unit symbol", "das"),
      i18nc("unit description in lists", "decaseconds"),
      i18nc("unit synonyms for matching user input", "decasecond;decaseconds;das"),
      ki18nc("amount in units (real)", "%1 decaseconds"),
      ki18ncp("amount in units (integer)", "%1 decasecond", "%1 decaseconds")
    );
    setDefaultUnit(U(Time::Second, 1,
      i18nc("time unit symbol", "s"),
      i18nc("unit description in lists", "seconds"),
      i18nc("unit synonyms for matching user input", "second;seconds;s"),
      ki18nc("amount in units (real)", "%1 seconds"),
      ki18ncp("amount in units (integer)", "%1 second", "%1 seconds")
    ));
    U(Time::Decisecond, 0.1,
      i18nc("time unit symbol", "ds"),
      i18nc("unit description in lists", "deciseconds"),
      i18nc("unit synonyms for matching user input", "decisecond;deciseconds;ds"),
      ki18nc("amount in units (real)", "%1 deciseconds"),
      ki18ncp("amount in units (integer)", "%1 decisecond", "%1 deciseconds")
    );
    U(Time::Centisecond, 0.01,
      i18nc("time unit symbol", "cs"),
      i18nc("unit description in lists", "centiseconds"),
      i18nc("unit synonyms for matching user input", "centisecond;centiseconds;cs"),
      ki18nc("amount in units (real)", "%1 centiseconds"),
      ki18ncp("amount in units (integer)", "%1 centisecond", "%1 centiseconds")
    );
    U(Time::Millisecond, 0.001,
      i18nc("time unit symbol", "ms"),
      i18nc("unit description in lists", "milliseconds"),
      i18nc("unit synonyms for matching user input", "millisecond;milliseconds;ms"),
      ki18nc("amount in units (real)", "%1 milliseconds"),
      ki18ncp("amount in units (integer)", "%1 millisecond", "%1 milliseconds")
    );
    U(Time::Microsecond, 1e-06,
      i18nc("time unit symbol", "µs"),
      i18nc("unit description in lists", "microseconds"),
      i18nc("unit synonyms for matching user input", "microsecond;microseconds;µs;us"),
      ki18nc("amount in units (real)", "%1 microseconds"),
      ki18ncp("amount in units (integer)", "%1 microsecond", "%1 microseconds")
    );
    U(Time::Nanosecond, 1e-09,
      i18nc("time unit symbol", "ns"),
      i18nc("unit description in lists", "nanoseconds"),
      i18nc("unit synonyms for matching user input", "nanosecond;nanoseconds;ns"),
      ki18nc("amount in units (real)", "%1 nanoseconds"),
      ki18ncp("amount in units (integer)", "%1 nanosecond", "%1 nanoseconds")
    );
    U(Time::Picosecond, 1e-12,
      i18nc("time unit symbol", "ps"),
      i18nc("unit description in lists", "picoseconds"),
      i18nc("unit synonyms for matching user input", "picosecond;picoseconds;ps"),
      ki18nc("amount in units (real)", "%1 picoseconds"),
      ki18ncp("amount in units (integer)", "%1 picosecond", "%1 picoseconds")
    );
    U(Time::Femtosecond, 1e-15,
      i18nc("time unit symbol", "fs"),
      i18nc("unit description in lists", "femtoseconds"),
      i18nc("unit synonyms for matching user input", "femtosecond;femtoseconds;fs"),
      ki18nc("amount in units (real)", "%1 femtoseconds"),
      ki18ncp("amount in units (integer)", "%1 femtosecond", "%1 femtoseconds")
    );
    U(Time::Attosecond, 1e-18,
      i18nc("time unit symbol", "as"),
      i18nc("unit description in lists", "attoseconds"),
      i18nc("unit synonyms for matching user input", "attosecond;attoseconds;as"),
      ki18nc("amount in units (real)", "%1 attoseconds"),
      ki18ncp("amount in units (integer)", "%1 attosecond", "%1 attoseconds")
    );
    U(Time::Zeptosecond, 1e-21,
      i18nc("time unit symbol", "zs"),
      i18nc("unit description in lists", "zeptoseconds"),
      i18nc("unit synonyms for matching user input", "zeptosecond;zeptoseconds;zs"),
      ki18nc("amount in units (real)", "%1 zeptoseconds"),
      ki18ncp("amount in units (integer)", "%1 zeptosecond", "%1 zeptoseconds")
    );
    U(Time::Yoctosecond, 1e-24,
      i18nc("time unit symbol", "ys"),
      i18nc("unit description in lists", "yoctoseconds"),
      i18nc("unit synonyms for matching user input", "yoctosecond;yoctoseconds;ys"),
      ki18nc("amount in units (real)", "%1 yoctoseconds"),
      ki18ncp("amount in units (integer)", "%1 yoctosecond", "%1 yoctoseconds")
    );
    U(Time::Minute, 60,
      i18nc("time unit symbol", "min"),
      i18nc("unit description in lists", "minutes"),
      i18nc("unit synonyms for matching user input", "minute;minutes;min"),
      ki18nc("amount in units (real)", "%1 minutes"),
      ki18ncp("amount in units (integer)", "%1 minute", "%1 minutes")
    );
    U(Time::Hour, 3600,
      i18nc("time unit symbol", "h"),
      i18nc("unit description in lists", "hours"),
      i18nc("unit synonyms for matching user input", "hour;hours;h"),
      ki18nc("amount in units (real)", "%1 hours"),
      ki18ncp("amount in units (integer)", "%1 hour", "%1 hours")
    );
    U(Time::Day, 86400,
      i18nc("time unit symbol", "d"),
      i18nc("unit description in lists", "days"),
      i18nc("unit synonyms for matching user input", "day;days;d"),
      ki18nc("amount in units (real)", "%1 days"),
      ki18ncp("amount in units (integer)", "%1 day", "%1 days")
    );
    U(Time::Week, 604800,
      i18nc("time unit symbol", ""),
      i18nc("unit description in lists", "weeks"),
      i18nc("unit synonyms for matching user input", "week;weeks;"),
      ki18nc("amount in units (real)", "%1 weeks"),
      ki18ncp("amount in units (integer)", "%1 week", "%1 weeks")
    );
    U(Time::JulianYear, 3.15576e+07,
      i18nc("time unit symbol", "a"),
      i18nc("unit description in lists", "julian years"),
      i18nc("unit synonyms for matching user input", "julian year;julian years;a"),
      ki18nc("amount in units (real)", "%1 julian years"),
      ki18ncp("amount in units (integer)", "%1 julian year", "%1 julian years")
    );
    U(Time::LeapYear, 3.16224e+07,
      i18nc("time unit symbol", ""),
      i18nc("unit description in lists", "leap years"),
      i18nc("unit synonyms for matching user input", "leap year;leap years;"),
      ki18nc("amount in units (real)", "%1 leap years"),
      ki18ncp("amount in units (integer)", "%1 leap year", "%1 leap years")
    );
    //Let's say we call that a normal year
    U(Time::Year, 3.1536e+07,
      i18nc("time unit symbol", "y"),
      i18nc("unit description in lists", "year"),
      i18nc("unit synonyms for matching user input", "year;years;y"),
      ki18nc("amount in units (real)", "%1 year"),
      ki18ncp("amount in units (integer)", "%1 year", "%1 years")
    );
}
