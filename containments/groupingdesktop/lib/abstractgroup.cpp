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
        interestingGroup(0),
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
        kDebug()<<"removed applet"<<applet->id()<<"from group"<<id<<"of type"<<q->pluginName();

        applets.removeAll(applet);

        if (destroying && (q->children().count() == 0)) {
            destroyGroup();
            destroying = false;
        }

//         emit q->appletRemovedFromGroup(applet, q);
    }
}

void AbstractGroupPrivate::subGroupDestroyed(AbstractGroup *subGroup)
{
    if (subGroups.contains(subGroup)) {
        kDebug()<<"removed sub group"<<subGroup->id()<<"from group"<<id<<"of type"<<q->pluginName();

        subGroups.removeAll(subGroup);

        if (destroying && (q->children().count() == 0)) {
            destroyGroup();
            destroying = false;
        }

//         emit q->appletRemovedFromGroup(applet, q);
    }
}

void AbstractGroupPrivate::callLayoutChild()
{
    if (!currChild || currChildPos.isNull()) {
        return;
    }

    currChild->setFlag(QGraphicsItem::ItemIsMovable, false);
    currChild->setPos(currChildPos);
    q->layoutChild(currChild, currChildPos);

    currChild->installEventFilter(q);

    currChild = 0;
    currChildPos = QPointF();
}

void AbstractGroupPrivate::repositionRemovedChild()
{
    if (!currChild || currChildPos.isNull()) {
        return;
    }

    currChild->setPos(containment->mapFromScene(currChildPos));

    currChild = 0;
    currChildPos = QPointF();
}

void AbstractGroupPrivate::addChild(QGraphicsWidget *child, bool layoutChild)
{
    QPointF newPos = q->mapFromScene(child->scenePos());
    child->setParentItem(q);
    //FIXME this simple line breaks everything when adding plasmoids from the containment!! Why???
//     applet->setPos(newPos);

    if (layoutChild) {
        currChild = child;
        currChildPos = newPos;
        //HACK so i workarounded the above-mentioned problem with this QTimer::singleShot
        QTimer::singleShot(0, q, SLOT(callLayoutChild()));
    } else {
        child->installEventFilter(q);
    }

    emit q->configNeedsSaving();
}

void AbstractGroupPrivate::removeChild(QGraphicsWidget *child)
{
    currChild = child;
    currChildPos = (child->scenePos());

    child->removeEventFilter(q);
    child->setParentItem(containment);

    //HACK like the one in addChild
    QTimer::singleShot(0, q, SLOT(repositionRemovedChild()));
}

//-----------------------------AbstractGroup------------------------------

AbstractGroup::AbstractGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
             : QGraphicsWidget(parent, wFlags),
               d(new AbstractGroupPrivate(this))
{
    setAcceptHoverEvents(true);
    setAcceptDrops(true);
//     setContentsMargins(0, 10, 10, 10);
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

void AbstractGroup::addApplet(Plasma::Applet *applet, bool layoutApplet)
{
    if (!applet || applets().contains(applet)) {
        return;
    }

    kDebug()<<"adding applet"<<applet->id()<<"in group"<<id()<<"of type"<<pluginName();

    d->applets << applet;
    d->addChild(applet, layoutApplet);

    connect(applet, SIGNAL(appletDestroyed(Plasma::Applet*)),
            this, SLOT(appletDestroyed(Plasma::Applet*)));

    emit appletAddedInGroup(applet, this);
}

void AbstractGroup::addSubGroup(AbstractGroup *group, bool layoutGroup)
{
    if (!group || subGroups().contains(group)) {
        return;
    }

    kDebug()<<"adding sub group"<<group->id()<<"in group"<<id()<<"of type"<<pluginName();

    d->subGroups << group;
    d->addChild(group, layoutGroup);

    connect(group, SIGNAL(groupDestroyed(AbstractGroup*)),
            this, SLOT(subGroupDestroyed(AbstractGroup*)));

    emit subGroupAddedInGroup(group, this);
}

Plasma::Applet::List AbstractGroup::applets() const
{
    return d->applets;
}

QList<AbstractGroup *> AbstractGroup::subGroups() const
{
    return d->subGroups;
}

QList<QGraphicsWidget *> AbstractGroup::children() const
{
    QList<QGraphicsWidget *> list;
    foreach (Plasma::Applet *applet, d->applets) {
        list << applet;
    }
    foreach (AbstractGroup *group, d->subGroups) {
        list << group;
    }

    return list;
}

void AbstractGroup::removeApplet(Plasma::Applet *applet)
{
    kDebug()<<"removing applet"<<applet->id()<<"from group"<<id()<<"of type"<<pluginName();

    d->applets.removeAll(applet);
    KConfigGroup appletConfig = applet->config().parent();
    KConfigGroup groupConfig(&appletConfig, QString("GroupInformation"));
    groupConfig.deleteGroup();
    emit configNeedsSaving();

    AbstractGroup *parentGroup = qgraphicsitem_cast<AbstractGroup *>(parentItem());
    if (parentGroup) {
        parentGroup->addApplet(applet);
    } else {
        d->removeChild(applet);
    }

    emit appletRemovedFromGroup(applet, this);
}

void AbstractGroup::removeSubGroup(AbstractGroup *subGroup)
{
    kDebug()<<"removing sub group"<<subGroup->id()<<"from group"<<id()<<"of type"<<pluginName();

    d->subGroups.removeAll(subGroup);
    KConfigGroup subGroupConfig = subGroup->config().parent();
    KConfigGroup groupConfig(&subGroupConfig, QString("GroupInformation"));
    groupConfig.deleteGroup();
    emit configNeedsSaving();

    AbstractGroup *parentGroup = qgraphicsitem_cast<AbstractGroup *>(parentItem());
    if (parentGroup) {
        parentGroup->addSubGroup(subGroup);
    } else {
        d->removeChild(subGroup);
    }

    emit subGroupRemovedFromGroup(subGroup, this);
}

void AbstractGroup::destroy()
{
    kDebug()<<"destroying group"<<id()<<"of type"<<pluginName();

    if (applets().count() == 0) {
        d->destroyGroup();
        return;
    }

    d->destroying = true;

    foreach (AbstractGroup *group, subGroups()) {
        group->destroy();
    }
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
    AbstractGroup *group = qobject_cast<AbstractGroup *>(obj);
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(obj);

    QGraphicsWidget *widget = 0;
    if (applet) {
        widget = applet;
    } else if (group) {
        widget = group;
    }

    if (widget) {
        switch (event->type()) {
            case QEvent::GraphicsSceneMove:
                foreach (AbstractGroup *parentGroup, d->subGroups) {
                    if (!parentGroup->children().contains(widget) && (parentGroup != group)) {
                        QRectF rect = parentGroup->contentsRect();
                        rect.translate(parentGroup->pos());
                        if (rect.contains(widget->geometry())) {
                            if (applet) {
                                d->applets.removeAll(applet);
                                parentGroup->addApplet(applet);
                            } else if (!group->isAncestorOf(parentGroup)) {
                                d->subGroups.removeAll(group);
                                parentGroup->addSubGroup(group);
                            }
                            widget->removeEventFilter(this);
                            d->interestingGroup = 0;
                            break;
                        } else {
                            QRectF intersected(rect.intersected(widget->geometry()));
                            if (intersected.isValid()) {
                                parentGroup->showDropZone(mapToItem(parentGroup, intersected.center()));
                                d->interestingGroup = parentGroup;
                                break;
                            } else {
                                if (parentGroup == d->interestingGroup) {
                                    parentGroup->showDropZone(QPointF());
                                    d->interestingGroup = 0;
                                }
                            }
                        }
                    }
                }
                if (children().contains(widget) && !contentsRect().contains(widget->geometry())) {
                    if (applet) {
                        removeApplet(applet);
                    } else {
                        removeSubGroup(group);
                    }
                }
                break;

            default:
                break;
        }
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

int AbstractGroup::type() const
{
    return Type;
}

void AbstractGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (d->background && (d->containment->formFactor() != Plasma::Vertical) &&
                         (d->containment->formFactor() != Plasma::Horizontal)) {
        d->background->paintFrame(painter);
    } else {
        //TODO draw a halo, something
    }
}

#include "abstractgroup.moc"