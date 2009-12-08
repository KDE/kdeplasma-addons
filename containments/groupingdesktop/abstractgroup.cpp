/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>
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
#include "abstractgroup_p.h"

#include <QtCore/QTimer>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneResizeEvent>

#include <kservice.h>
#include <kservicetypetrader.h>

#include <Plasma/Containment>
#include <Plasma/FrameSvg>
#include <Plasma/Animator>
#include <Plasma/Animation>

#include "groupingcontainment.h"

AbstractGroupPrivate::AbstractGroupPrivate(AbstractGroup *group)
    : q(group),
        destroying(false),
        containment(0),
        immutability(Plasma::Mutable),
        groupType(AbstractGroup::FreeGroup),
        m_mainConfig(0)
{
    background = new Plasma::FrameSvg(q);
    background->setImagePath("widgets/translucentbackground");
    background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
}

AbstractGroupPrivate::~AbstractGroupPrivate()
{
    delete m_mainConfig;
}

KConfigGroup *AbstractGroupPrivate::mainConfigGroup()
{
    if (m_mainConfig) {
        return m_mainConfig;
    }

    KConfigGroup containmentGroup = containment->config();
    KConfigGroup groupsConfig = KConfigGroup(&containmentGroup, "Groups");
    KConfigGroup *mainConfig = new KConfigGroup(&groupsConfig, QString::number(id));

    return mainConfig;
}

void AbstractGroupPrivate::destroyGroup()
{
    mainConfigGroup()->deleteGroup();
    emit q->configNeedsSaving();

    Plasma::Animation *anim = Plasma::Animator::create(Plasma::Animator::DisappearAnimation, q);
    if (anim) {
    anim->setWidgetToAnimate(q);
    anim->start();
    }

    q->scene()->removeItem(q);
    delete q;
}

void AbstractGroupPrivate::appletDestroyed(Plasma::Applet *applet)
{
    if (applets.contains(applet)) {
        kDebug()<<"removed applet"<<applet->id()<<"inside group"<<id<<"of type"<<q->pluginName();

        applets.removeAll(applet);

        if (destroying && (applets.count() == 0)) {
            destroyGroup();
            destroying = false;
        }

//         emit q->appletRemovedFromGroup(applet, q);
    }
}

void AbstractGroupPrivate::callLayoutApplet()
{
    if (!currApplet || currAppletPos.isNull()) {
        return;
    }

    currApplet->setFlag(QGraphicsItem::ItemIsMovable, false);
    currApplet->setPos(currAppletPos);
    q->layoutApplet(currApplet, currAppletPos);

    currApplet->installEventFilter(q);
    q->connect(currApplet, SIGNAL(appletDestroyed(Plasma::Applet *)),
            q, SLOT(appletDestroyed(Plasma::Applet *)));

    emit q->appletAddedInGroup(currApplet, q);

    currApplet = 0;
    currAppletPos = QPointF();
}

void AbstractGroupPrivate::repositionRemovedApplet()
{
    if (!currApplet || currAppletPos.isNull()) {
        return;
    }

    currApplet->setPos(currAppletPos);
    emit q->appletRemovedFromGroup(currApplet, q);

    currApplet = 0;
    currAppletPos = QPointF();
}

//-----------------------------AbstractGroup------------------------------

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

void AbstractGroup::setImmutability(Plasma::ImmutabilityType immutability)
{
    setFlag(QGraphicsItem::ItemIsMovable, immutability == Plasma::Mutable);
    d->immutability = immutability;
}

Plasma::ImmutabilityType AbstractGroup::immutability() const
{
    return d->immutability;
}

uint AbstractGroup::id() const
{
    return d->id;
}

void AbstractGroup::addApplet(Plasma::Applet *applet, bool layoutApplets)
{
    if (!applet || applets().contains(applet)) {
        return;
    }

    kDebug()<<"adding applet"<<applet->id()<<"in group"<<id()<<"of type"<<pluginName();

    d->applets << applet;
    QPointF newPos = mapFromItem(containment(), applet->pos());
    applet->setParentItem(this);
    //FIXME this simple line breaks everything when adding plasmoids from the containment!! Why???
//     applet->setPos(newPos); 

    if (layoutApplets) {
        d->currApplet = applet;
        d->currAppletPos = newPos;
        //HACK so i workarounded the above-mentioned problem with this QTimer::singleShot
        QTimer::singleShot(0, this, SLOT(callLayoutApplet()));
    } else {
        applet->installEventFilter(this);
        connect(applet, SIGNAL(appletDestroyed(Plasma::Applet *)),
                this, SLOT(appletDestroyed(Plasma::Applet *)));

        emit appletAddedInGroup(applet, this);
    }

    emit configNeedsSaving();
}

Plasma::Applet::List AbstractGroup::applets() const
{
    return d->applets;
}

void AbstractGroup::removeApplet(Plasma::Applet *applet)
{
    kDebug()<<"removing applet"<<applet->id()<<"from group"<<id()<<"of type"<<pluginName();

    d->applets.removeAll(applet);
    applet->removeEventFilter(this);
    applet->setParentItem(containment());

    KConfigGroup appletConfig = applet->config().parent();
    KConfigGroup groupConfig(&appletConfig, QString("GroupInformation"));
    groupConfig.deleteGroup();
    emit configNeedsSaving();

    d->currApplet = applet;
    d->currAppletPos = mapToParent(applet->pos());
    //HACK like the one in layoutApplet
    QTimer::singleShot(0, this, SLOT(repositionRemovedApplet()));
}

void AbstractGroup::destroy()
{
    kDebug()<<"destroying group"<<id()<<"of type"<<pluginName();

    if (applets().count() == 0) {
        d->destroyGroup();
        return;
    }

    d->destroying = true;

    foreach (Plasma::Applet *applet, applets()) {
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

GroupingContainment *AbstractGroup::containment() const
{
    return d->containment;
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

void AbstractGroup::showDropZone(const QPointF& pos)
{
    Q_UNUSED(pos)

    //base implementation does nothing
}

void AbstractGroup::setGroupType(AbstractGroup::GroupType type)
{
    d->groupType = type;
}

AbstractGroup::GroupType AbstractGroup::groupType() const
{
    return d->groupType;
}

bool AbstractGroup::eventFilter(QObject *obj, QEvent *event)
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(obj);
    if (applet && applets().contains(applet)) {
        switch (event->type()) {
            case QEvent::GraphicsSceneMove:
                if (!contentsRect().contains(applet->geometry())) {
                    removeApplet(applet);
                }
                break;

            default:
                break;
        }

        return false;
    }


    return QGraphicsWidget::eventFilter(obj, event);
}

void AbstractGroup::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    showDropZone(event->pos());
}

void AbstractGroup::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    scene()->sendEvent(d->containment, event);
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