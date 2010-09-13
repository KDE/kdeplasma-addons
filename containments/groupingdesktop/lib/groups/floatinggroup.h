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

#ifndef FLOATINGGROUP_H
#define FLOATINGGROUP_H

#include "abstractgroup.h"

class FloatingGroup : public AbstractGroup
{
    Q_OBJECT
    public:
        explicit FloatingGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        ~FloatingGroup();

        QString pluginName() const;
        void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group);
        void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const;

        static GroupInfo groupInfo();

    protected:
        void layoutChild(QGraphicsWidget *child, const QPointF &pos);

};

#endif // FLOATINGGROUP_H
