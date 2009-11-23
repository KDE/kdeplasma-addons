/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@kde.org>
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

FloatingGroup::FloatingGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
             : AbstractGroup(parent, wFlags)
{
    resize(200,200);
}

FloatingGroup::~FloatingGroup()
{

}

QString FloatingGroup::pluginName() const
{
    return QString("floating");
}

void FloatingGroup::restoreAppletLayoutInfo(Plasma::Applet *applet, const KConfigGroup &group)
{
    QPointF pos = group.readEntry("Position", QPointF());

    applet->setPos(pos);
}

void FloatingGroup::saveAppletLayoutInfo(Plasma::Applet *applet, KConfigGroup group) const
{
    group.writeEntry("Position", applet->pos());
}

void FloatingGroup::layoutApplet(Plasma::Applet *applet)
{
    applet->setPos(mapFromItem(parentItem(), applet->pos()));
}

#include "floatinggroup.moc"