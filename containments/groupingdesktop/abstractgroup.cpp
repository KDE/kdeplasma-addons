/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "abstractgroup.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QPainter>

#include <Plasma/FrameSvg>

AbstractGroup::AbstractGroup(int id, QGraphicsItem* parent, Qt::WindowFlags wFlags)
             : QGraphicsWidget(parent, wFlags),
               m_id(id)
{
    setAcceptDrops(true);

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/translucentbackground");
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
}

AbstractGroup::~AbstractGroup()
{

}

int AbstractGroup::id() const
{
    return m_id;
}

void AbstractGroup::assignApplet(Plasma::Applet *applet)
{
    kDebug()<<"adding applet in group"<<id();

    applet->setParentItem(this);
    m_applets << applet;
}

Plasma::Applet::List AbstractGroup::assignedApplets()
{
    return m_applets;
}

void AbstractGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    m_background->resizeFrame(event->newSize());
}

void AbstractGroup::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    scene()->sendEvent(parentItem(), event);
}

void AbstractGroup::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    m_background->paintFrame(painter);
}

#include "abstractgroup.moc"