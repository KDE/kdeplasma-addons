/*
 * Copyright 2008  Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FEEDDATA_HEADER
#define FEEDDATA_HEADER

class KIcon;
#include <QString>

class FeedData
{
    public:
        FeedData() : icon(0), itemNumber(0) {};
        QString text;
        QString title;
        QString url;
        QString extrainfo;
        KIcon *icon;
        int itemNumber;
        uint time;
};

#endif
