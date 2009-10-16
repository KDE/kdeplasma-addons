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

#include <Plasma/Containment>
#include <Plasma/FrameSvg>

class AbstractGroupPrivate
{
    public:
        AbstractGroupPrivate()
        {
        }

        ~AbstractGroupPrivate()
        {
        }

        Plasma::Containment *containment;
        int id;
        Plasma::Applet::List applets;
        Plasma::FrameSvg *background;
};

AbstractGroup::AbstractGroup(int id, Plasma::Containment *parent, Qt::WindowFlags wFlags)
             : QGraphicsWidget(parent, wFlags),
               d(new AbstractGroupPrivate())
{
    setAcceptDrops(true);

    d->containment = parent;
    d->id = id;
    d->background = new Plasma::FrameSvg(this);
    d->background->setImagePath("widgets/translucentbackground");
    d->background->setEnabledBorders(Plasma::FrameSvg::AllBorders);

    connect(d->containment, SIGNAL(appletRemoved(Plasma::Applet *)),
            this, SLOT(onAppletRemoved(Plasma::Applet *)));
}

AbstractGroup::~AbstractGroup()
{

}

int AbstractGroup::id() const
{
    return d->id;
}

void AbstractGroup::assignApplet(Plasma::Applet *applet, bool layoutApplets)
{
    kDebug()<<"adding applet in group"<<id();

    applet->setParentItem(this);
    d->applets << applet;

    if (layoutApplets) {
        layoutApplet(applet);
    }
}

Plasma::Applet::List AbstractGroup::assignedApplets()
{
    return d->applets;
}

Plasma::Containment *AbstractGroup::containment()
{
    return d->containment;
}

void AbstractGroup::onAppletRemoved(Plasma::Applet* applet)
{
    foreach (Plasma::Applet *ownApplet, d->applets) {
        if (applet = ownApplet) {
            d->applets.removeAll(applet);
        }
    }
}

void AbstractGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->background->resizeFrame(event->newSize());
}

void AbstractGroup::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    scene()->sendEvent(parentItem(), event);
}

void AbstractGroup::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    d->background->paintFrame(painter);
}

#include "abstractgroup.moc"