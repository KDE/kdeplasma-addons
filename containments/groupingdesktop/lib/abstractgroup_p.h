/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#ifndef ABSTRACTGROUPPRIVATE_H
#define ABSTRACTGROUPPRIVATE_H

#include "abstractgroup.h"
#include "groupingcontainment.h"

class AbstractGroupPrivate
{
    public:
        AbstractGroupPrivate(AbstractGroup *group);
        ~AbstractGroupPrivate();
        KConfigGroup *mainConfigGroup();
        void addChild(QGraphicsWidget *child);
        void removeChild(QGraphicsWidget *child);
        void destroyGroup();
        void appletDestroyed(Plasma::Applet *applet);
        void subGroupDestroyed(AbstractGroup *subGroup);

        AbstractGroup *q;
        Plasma::Applet::List applets;
        QList<AbstractGroup *> subGroups;
        bool destroying;
        GroupingContainment *containment;
        unsigned int id;
        Plasma::FrameSvg *background;
        Plasma::ImmutabilityType immutability;
        AbstractGroup::GroupType groupType;
        AbstractGroup *interestingGroup;
        bool isMainGroup;

    private:
        KConfigGroup *m_mainConfig;
};

#endif
