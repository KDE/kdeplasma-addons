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

#include "floatinggroup.h"

REGISTER_GROUP(FloatingGroup)

FloatingGroup::FloatingGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
             : AbstractGroup(parent, wFlags)
{
    resize(200,200);
    setGroupType(AbstractGroup::FreeGroup);
}

FloatingGroup::~FloatingGroup()
{

}

QString FloatingGroup::pluginName() const
{
    return QString("floating");
}

void FloatingGroup::restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group)
{
    QPointF pos = group.readEntry("Position", QPointF());

    child->setPos(pos);
}

void FloatingGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    group.writeEntry("Position", child->pos());
}

void FloatingGroup::layoutChild(QGraphicsWidget *, const QPointF &)
{

}

GroupInfo FloatingGroup::groupInfo()
{
    GroupInfo gi("floating", i18n("Floating Group"));

    return gi;
}

#include "floatinggroup.moc"
