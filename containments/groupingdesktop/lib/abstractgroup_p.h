/*
 *   Copyright 2009-2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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
        void startDestroyAnimation();
        void destroyGroup();
        void appletDestroyed(Plasma::Applet *applet);
        void subGroupDestroyed(AbstractGroup *subGroup);
        void restoreChildren();
        void onChildGeometryChanged();
        QString configDialogId();
        void setChildBorders(Plasma::Applet *applet, bool added);
        void setChildBorders(AbstractGroup *group, bool added);
        void setBackground();

        AbstractGroup *q;
        AbstractGroup *parentGroup;
        Plasma::Applet::List applets;
        QList<AbstractGroup *> subGroups;
        bool destroying;
        unsigned int id;
        Plasma::FrameSvg *background;
        Plasma::ImmutabilityType immutability;
        AbstractGroup::GroupType groupType;
        AbstractGroup *interestingGroup;
        bool isMainGroup;
        AbstractGroup::BackgroundHints backgroundHints;
        bool isLoading;
        bool hasInterface;
        bool simplerBackgroundChildren;
        QHash<Plasma::Applet *, Plasma::Applet::BackgroundHints> savedAppletsHints;
        QHash<AbstractGroup *, AbstractGroup::BackgroundHints> savedGroupsHints;

    private:
        KConfigGroup *m_mainConfig;
};

#endif
