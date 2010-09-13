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

#ifndef GROUPFACTORY_H
#define GROUPFACTORY_H

#include <QtCore/QString>
#include <QtCore/QMap>

#include <KDebug>

#include <Plasma/Plasma>

#include "groupinfo.h"

class QGraphicsItem;
class AbstractGroup;

typedef AbstractGroup *(*CreatorFunction)(QGraphicsItem *);

class GroupFactory
{

    public:

        template<class T> static bool registerGroup()
        {
            if (!m_groups) {
                m_groups = new QMap<GroupInfo, CreatorFunction>;
            }

            GroupInfo gi = T::groupInfo();

            m_groups->insert(gi, &createGroup<T>);
            return true;
        }

        static AbstractGroup *load(const QString &name, QGraphicsItem *parent = 0);

        QList<GroupInfo> static groupInfos();

        template<class T> static AbstractGroup *createGroup(QGraphicsItem *parent)
        {
            return new T(parent);
        }

        static QMap<GroupInfo, CreatorFunction> *m_groups;
};

#endif // GROUPFACTORY_H
