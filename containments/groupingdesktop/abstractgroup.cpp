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

#include "abstractgroup.h"

#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneResizeEvent>

#include <kservice.h>
#include <kservicetypetrader.h>

#include <Plasma/Containment>
#include <Plasma/FrameSvg>
#include <Plasma/Animator>
#include <Plasma/AbstractAnimation>

class AbstractGroupPrivate
{
    public:
        AbstractGroupPrivate(AbstractGroup *group)
            : q(group),
              destroying(false),
              containment(0),
              immutability(Plasma::Mutable),
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

            Plasma::AbstractAnimation *anim = Plasma::Animator::create(Plasma::Animator::DisappearAnimation, q);
            if (anim) {
            anim->setWidgetToAnimate(q);
            anim->start();
            }

            q->scene()->removeItem(q);
            delete q;
        }

        void appletDestroyed(Plasma::Applet *applet)
        {
            if (applets.contains(applet)) {
                kDebug()<<"removed applet"<<applet->id()<<"inside group"<<id;

                applets.removeAll(applet);

                if (destroying && (applets.count() == 0)) {
                    destroyGroup();
                    destroying = false;
                }

//                 emit q->appletRemovedFromGroup(applet); //FIXME crash! ???
            }
        }

        Plasma::Applet::List applets;
        AbstractGroup *q;
        bool destroying;
        Plasma::Containment *containment;
        unsigned int id;
        Plasma::FrameSvg *background;
        Plasma::ImmutabilityType immutability;

    private:
        KConfigGroup *m_mainConfig;
};

AbstractGroup::AbstractGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
             : QGraphicsWidget(parent, wFlags),
               d(new AbstractGroupPrivate(this))
{
    setAcceptHoverEvents(true);
    setAcceptDrops(true);
    setContentsMargins(10, 10, 10, 10);
}

AbstractGroup::~AbstractGroup()
{
    emit groupDestroyed(this);

    delete d;
}

void AbstractGroup::setContainment(Plasma::Containment *containment)
{
    d->containment = containment;

    connect (containment, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
             this, SLOT(setImmutability(Plasma::ImmutabilityType)));
}

Plasma::Containment *AbstractGroup::containment() const
{
    return d->containment;
}

void AbstractGroup::setImmutability(Plasma::ImmutabilityType immutability)
{
    setFlag(QGraphicsItem::ItemIsMovable, immutability == Plasma::Mutable);
    d->immutability = immutability;
}

Plasma::ImmutabilityType AbstractGroup::immutability() const
{
    return d->immutability;
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

    connect(applet, SIGNAL(appletDestroyed(Plasma::Applet *)),
            this, SLOT(appletDestroyed(Plasma::Applet *)));

    if (layoutApplets) {
        layoutApplet(applet);
    }
}

Plasma::Applet::List AbstractGroup::assignedApplets() const
{
    return d->applets;
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

QGraphicsView *AbstractGroup::view() const
{
    // It's assumed that we won't be visible on more than one view here.
    // Anything that actually needs view() should only really care about
    // one of them anyway though.
    if (!scene()) {
        return 0;
    }

    QGraphicsView *found = 0;
    QGraphicsView *possibleFind = 0;
    //kDebug() << "looking through" << scene()->views().count() << "views";
    foreach (QGraphicsView *view, scene()->views()) {
        //kDebug() << "     checking" << view << view->sceneRect()
        //         << "against" << sceneBoundingRect() << scenePos();
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            //kDebug() << "     found something!" << view->isActiveWindow();
            if (view->isActiveWindow()) {
                found = view;
            } else {
                possibleFind = view;
            }
        }
    }

    return found ? found : possibleFind;
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

void AbstractGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->background->resizeFrame(event->newSize());

    emit geometryChanged();
}

void AbstractGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    d->background->paintFrame(painter);
}

#include "abstractgroup.moc"