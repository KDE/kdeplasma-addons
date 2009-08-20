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
#include "conversioni18ncatalog.h"
#include <KLocale>

Time::Time(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
    
    setObjectName("time");
    setName(i18n("Time"));
    addSIUnit("s", i18n("second"), i18n("seconds"));

    U(i18n("minute"), i18n("minutes"), "min", 60.0, );
    U(i18n("hour"), i18n("hours"), "h", 60.0 * 60.0, );
    U(i18n("day"), i18n("days"), "d", 24.0 * 60.0 * 60.0, );
    U(i18n("week"), i18n("weeks"), "", 7.0 * 24.0 * 60.0 * 60.0, );
    U(i18n("julian year"), i18n("julian years"), "a", 365.25 * 24.0 * 60.0 * 60.0, );
	U(i18n("leap year"), i18n("leap years"), "", 366 * 24.0 * 60.0 * 60.0, );
	//Let's say we call that a normal year
	U(i18n("year"), i18n("year"), "y", 365 * 24.0 * 60.0 * 60.0, );
}
