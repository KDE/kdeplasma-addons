/*
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#ifndef GROUPLIST_H
#define GROUPLIST_H

#include "groupicon.h"
#include "abstracticonlist.h"

class GroupList : public Plasma::AbstractIconList
{
    Q_OBJECT
    public:
        explicit GroupList(Plasma::Location location = Plasma::BottomEdge, QGraphicsItem *parent = 0);
        ~GroupList();

    protected:
        void updateVisibleIcons();
        void setSearch(const QString &searchString);

    private:
        void createGroupIcon(const QString &name);

};

#endif //GROUPLIST_H
