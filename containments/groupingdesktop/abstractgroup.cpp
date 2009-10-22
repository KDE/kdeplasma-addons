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

#include <kservice.h>
#include <kservicetypetrader.h>

#include <Plasma/Containment>
#include <Plasma/FrameSvg>

class AbstractGroupPrivate
{
    public:
        AbstractGroupPrivate(AbstractGroup *group)
            : q(group),
              destroying(false),
              containment(0),
              m_mainConfig(0)
        {
            background = new Plasma::FrameSvg(q);
            background->setImagePath("widgets/translucentbackground");
            background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        }

        ~AbstractGroupPrivate()
        {
            delete m_mainConfig;
        }

        KConfigGroup *mainConfigGroup()
        {
            if (m_mainConfig) {
                return m_mainConfig;
            }

            KConfigGroup containmentGroup = containment->config();
            KConfigGroup groupsConfig = KConfigGroup(&containmentGroup, "Groups");
            KConfigGroup *mainConfig = new KConfigGroup(&groupsConfig, QString::number(id));

            return mainConfig;
        }

        void destroyGroup()
        {
            mainConfigGroup()->deleteGroup();
            delete m_mainConfig;

            q->scene()->removeItem(q);
            delete q;
        }

        Plasma::Applet::List applets;
        AbstractGroup *q;
        bool destroying;
        Plasma::Containment *containment;
        unsigned int id;
        Plasma::FrameSvg *background;

    private:
        KConfigGroup *m_mainConfig;
};

AbstractGroup::AbstractGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
             : QGraphicsWidget(parent, wFlags),
               d(new AbstractGroupPrivate(this))
{
    setAcceptDrops(true);
}

AbstractGroup::~AbstractGroup()
{
    emit groupRemoved(this);

    delete d;
}

void AbstractGroup::setContainment(Plasma::Containment *containment)
{
    d->containment = containment;

    connect(containment, SIGNAL(appletRemoved(Plasma::Applet *)),
            this, SLOT(onAppletRemoved(Plasma::Applet *)));
    connect (containment, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
             this, SLOT(setImmutability(Plasma::ImmutabilityType)));
}

Plasma::Containment* AbstractGroup::containment() const
{
    return d->containment;
}

void AbstractGroup::setImmutability(Plasma::ImmutabilityType immutability)
{
    if (immutability == Plasma::Mutable) {
        setFlag(QGraphicsItem::ItemIsMovable, true);
    } else {
        setFlag(QGraphicsItem::ItemIsMovable, false);
    }
}

void AbstractGroup::setId(unsigned int id)
{
    d->id = id;
}

unsigned int AbstractGroup::id() const
{
    return d->id;
}

void AbstractGroup::assignApplet(Plasma::Applet *applet, bool layoutApplets)
{
    if (!applet) {
        return;
    }

    kDebug()<<"adding applet"<<applet->id()<<"in group"<<id();

    applet->setParentItem(this);
//     applet->setImmutability(Plasma::SystemImmutable);
    d->applets << applet;

    connect(applet, SIGNAL(appletDestroyed(Plasma::Applet*)),
            this, SLOT(onAppletRemoved(Plasma::Applet*)));

    if (layoutApplets) {
        layoutApplet(applet);
    }
}

Plasma::Applet::List AbstractGroup::assignedApplets() const
{
    return d->applets;
}

void AbstractGroup::onAppletRemoved(Plasma::Applet *applet)
{
    kDebug()<<"removed";
    foreach (Plasma::Applet *ownApplet, d->applets) {
        if (applet == ownApplet) {
            kDebug()<<"removed";
            emit appletRemoved(applet);

            d->applets.removeAll(applet);

            if (d->destroying && (d->applets.count() == 0)) {
                d->destroyGroup();
                d->destroying = false;
            }
        }
    }
}

void AbstractGroup::destroy()
{
    if (assignedApplets().count() == 0) {
        d->destroyGroup();
        return;
    }

    d->destroying = true;

    foreach (Plasma::Applet *applet, assignedApplets()) {
        kDebug()<<"deleting applet"<<applet->id()<<"in group"<<id();
        applet->destroy();
    }
}

KConfigGroup AbstractGroup::config() const
{
    KConfigGroup config = KConfigGroup(d->mainConfigGroup(), "Configuration");

    return config;
}

void AbstractGroup::save(KConfigGroup &group) const
{
    if (!group.isValid()) {
        group = *d->mainConfigGroup();
    }

    group.writeEntry("plugin", pluginName());
    group.writeEntry("geometry", geometry());
}

void AbstractGroup::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    scene()->sendEvent(containment(), event);
}

void AbstractGroup::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    d->background->resizeFrame(event->newSize());
}

void AbstractGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    d->background->paintFrame(painter);
}

#include "abstractgroup.moc"