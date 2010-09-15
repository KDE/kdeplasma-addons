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

#include "griddesktop.h"

#include <Plasma/Corona>

#include "../lib/abstractgroup.h"

K_EXPORT_PLASMA_APPLET(griddesktop, GridDesktop)

GridDesktop::GridDesktop(QObject* parent, const QVariantList& args)
               : GroupingContainment(parent, args)
{
    setHasConfigurationInterface(true);
    KGlobal::locale()->insertCatalog("libplasma_groupingcontainment");
    setContainmentType(Plasma::Containment::DesktopContainment);
    useMainGroup("grid");
}

GridDesktop::~GridDesktop()
{
}

void GridDesktop::init()
{
    connect(corona(), SIGNAL(availableScreenRegionChanged()), this, SLOT(layoutMainGroup()));

    GroupingContainment::init();
}

void GridDesktop::layoutMainGroup()
{
    QRectF rect = corona()->availableScreenRegion(screen()).boundingRect();
    mainGroup()->setGeometry(rect);
}

#include "griddesktop.moc"
