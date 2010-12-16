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

#include "groupingcontainment.h"
#include "groupingcontainment_p.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QAction>
#include <QtGui/QGraphicsView>
#include <QtCore/QTimer>

#include <KDebug>
#include <KMenu>
#include <KIcon>

#include <kwindowsystem.h>
#include <netwm.h>

#include <Plasma/Corona>
#include <Plasma/Animator>
#include <Plasma/Animation>
#include <Plasma/WindowEffects>

#include "abstractgroup.h"
#include "abstractgroup_p.h"
#include "handle.h"

#include "groupexplorer/explorerwindow.h"

//----------------------GroupingContainmentPrivate-----------------------

int GroupingContainmentPrivate::s_maxZValue = 0;
unsigned int GroupingContainmentPrivate::s_groupId = 0;

GroupingContainmentPrivate::GroupingContainmentPrivate(GroupingContainment *containment)
                           : q(containment),
                             mainGroup(0),
                             mainGroupId(0),
                             layout(0),
                             loading(true),
                             movingWidget(0),
                             interestingWidget(0),
                             movementHelperWidget(new QGraphicsWidget(q)),
                             widgetToBeSetMoving(0),
                             blockSceneEventFilter(false)
{
    newGroupAction = new QAction(i18n("Add Groups"), q);
    newGroupAction->setIcon(KIcon("list-add"));

    deleteGroupAction = new QAction(q);
    deleteGroupAction->setIcon(KIcon("edit-delete"));
    deleteGroupAction->setVisible(false);

    configureGroupAction = new QAction(q);
    configureGroupAction->setIcon(KIcon("configure"));
    configureGroupAction->setVisible(false);

    separator = new QAction(q);
    separator->setSeparator(true);

    q->connect(newGroupAction, SIGNAL(triggered()), q, SLOT(newGroupClicked()));
    q->connect(deleteGroupAction, SIGNAL(triggered()), q, SLOT(deleteGroup()));
    q->connect(configureGroupAction, SIGNAL(triggered()), q, SLOT(configureGroup()));
}

GroupingContainmentPrivate::~GroupingContainmentPrivate()
{
    //ensure we don't remain with an instance of ExplorerWindow when we don't have
    //grouping containments anymore. Not the best way to do it but it won't cause
    //any harm because it will be recreated the next time it's needed.
    delete ExplorerWindow::instance();
}

AbstractGroup *GroupingContainmentPrivate::createGroup(const QString &plugin, const QPointF &pos,
                                                       unsigned int id, bool delayInit)
{
    foreach (AbstractGroup *group, groups) {
        if (group->id() == id) {
            return 0;
        }
    }

    AbstractGroup *group = AbstractGroup::load(plugin, q);

    if (!group) {
        return 0;
    }

    if (groups.contains(group)) {
        delete group;
        return 0;
    }

    if (id == 0) {
        id = ++s_groupId;
    } else if (id > s_groupId) {
        s_groupId = id;
    }
    group->d->id = id;

    groups << group;

    q->addGroup(group, pos);

    if (!delayInit) {
        group->init();

        if (!loading) {
            group->d->restoreChildren();
        }
        group->save(*(group->d->mainConfigGroup()));
        emit q->configNeedsSaving();
    }

    return group;
}

void GroupingContainmentPrivate::handleDisappeared(Handle *handle)
{
    if (handles.contains(handle->widget())) {
        handles.remove(handle->widget());
        handle->detachWidget();
        if (q->scene()) {
            q->scene()->removeItem(handle);
        }
        handle->deleteLater();
    }
}

void GroupingContainmentPrivate::onGroupRemoved(AbstractGroup *group)
{
    kDebug()<<"Removed group"<<group->id();

    groups.removeAll(group);
    group->removeEventFilter(q);
    group->removeSceneEventFilter(q);

    if (handles.contains(group)) {
        Handle *handle = handles.value(group);
        handles.remove(group);
        delete handle;
    }

    emit q->groupRemoved(group);
    emit q->configNeedsSaving();
}

void GroupingContainmentPrivate::onAppletRemoved(Plasma::Applet *applet)
{
    kDebug()<<"Removed applet"<<applet->id();

    applet->removeEventFilter(q);
    applet->removeSceneEventFilter(q);

    if (handles.contains(applet)) {
        Handle *handle = handles.value(applet);
        handles.remove(applet);
        delete handle;
    }
}

QList<AbstractGroup *> GroupingContainmentPrivate::groupsAt(const QPointF &pos, QGraphicsWidget *uppermostItem)
{
    QList<AbstractGroup *> groups;

    if (pos.isNull()) {
        return groups;
    }

    QList<QGraphicsItem *> items = q->scene()->items(q->mapToScene(pos),
                                                     Qt::IntersectsItemShape,
                                                     Qt::DescendingOrder);

    if (items.isEmpty()) {
        return groups;
    }

    bool goOn;
    if (uppermostItem && items.contains(uppermostItem)) {
        do {
            if (items.isEmpty()) {
                return groups;
            }
            goOn = items.first() != uppermostItem;
            items.removeFirst();
        } while (goOn);
    }

    for (int i = 0; i < items.size(); ++i) {
        AbstractGroup *group = qgraphicsitem_cast<AbstractGroup *>(items.at(i));
        if (group && group->contentsRect().contains(q->mapToItem(group, pos))) {
            groups << group;
        }
    }

    return groups;
}

AbstractGroup *GroupingContainmentPrivate::groupAt(const QPointF &pos, QGraphicsWidget *uppermostItem)
{
    QList<AbstractGroup *> groups = groupsAt(pos, uppermostItem);
    if (groups.isEmpty()) {
        return 0;
    }

    return groups.first();
}

void GroupingContainmentPrivate::manageApplet(Plasma::Applet *applet, const QPointF &pos)
{
    int z = applet->zValue();
    if (GroupingContainmentPrivate::s_maxZValue < z) {
        GroupingContainmentPrivate::s_maxZValue = z;
    } else {
        applet->setZValue(GroupingContainmentPrivate::s_maxZValue);
    }

    AbstractGroup *group = 0;
    if (interestingGroup) {
        group = interestingGroup.data();
        interestingGroup.clear();
    } else {
        group = groupAt(pos.x() < 0 || pos.y() < 0 ? QPointF(10, 10) : pos);
    }

    if (group) {
        group->addApplet(applet);
    }

    applet->installEventFilter(q);
    applet->installSceneEventFilter(q);

    q->connect(applet, SIGNAL(appletDestroyed(Plasma::Applet*)), q, SLOT(onAppletRemoved(Plasma::Applet*)));
}

void GroupingContainmentPrivate::manageGroup(AbstractGroup *subGroup, const QPointF &pos)
{
    q->raise(subGroup);

    AbstractGroup *group = 0;
    if (interestingGroup) {
        group = interestingGroup.data();
        interestingGroup.clear();
    } else {
        group = groupAt(pos, subGroup);
    }

    if (group && (group != subGroup)) {
        group->addSubGroup(subGroup);
    }
}

void GroupingContainmentPrivate::newGroupClicked()
{
    ExplorerWindow *w = ExplorerWindow::instance();
    if (w->isHidden()) {
        w->setContainment(q);
        w->setLocation(q->location());
        w->showGroupExplorer();
        w->resize(w->sizeHint());

        bool moved = false;
        if (q->containmentType() == Plasma::Containment::PanelContainment ||
            q->containmentType() == Plasma::Containment::CustomPanelContainment) {
            // try to align it with the appropriate panel view
            QGraphicsView *view = q->view();
            if (view) {
                w->move(w->positionForPanelGeometry(view->geometry()));
                moved = true;
            }
        }

        if (!moved) {
            // set it to the bottom of the screen as we have no better hints to go by
            QRect geom = q->corona()->screenGeometry(q->screen());
            w->setGeometry(geom.x(), geom.bottom() - w->height(), geom.width(), w->height());
        }

        w->show();
        Plasma::WindowEffects::slideWindow(w, Plasma::BottomEdge);
        KWindowSystem::setOnAllDesktops(w->winId(), true);
        KWindowSystem::activateWindow(w->winId());
        KWindowSystem::setState(w->winId(), NET::SkipTaskbar | NET::SkipPager | NET::Sticky | NET::KeepAbove);
    }
}

void GroupingContainmentPrivate::deleteGroup()
{
    int id = deleteGroupAction->data().toInt();

    foreach (AbstractGroup *group, groups) {
        if ((int)group->id() == id) {
            group->destroy();

            return;
        }
    }
}

void GroupingContainmentPrivate::configureGroup()
{
    int id = configureGroupAction->data().toInt();

    foreach (AbstractGroup *group, groups) {
        if ((int)group->id() == id) {
            group->showConfigurationInterface();

            return;
        }
    }
}

void GroupingContainmentPrivate::onAppletRemovedFromGroup(Plasma::Applet *applet, AbstractGroup *group)
{
    Q_UNUSED(group)

    if (applet->parentItem() == q) {
        applet->installEventFilter(q);
    }
}

void GroupingContainmentPrivate::onSubGroupRemovedFromGroup(AbstractGroup *subGroup, AbstractGroup *group)
{
    Q_UNUSED(group)

    if (subGroup->parentItem() == q) {
        subGroup->installEventFilter(q);
    }
}

void GroupingContainmentPrivate::widgetMovedAnimationComplete()
{
    blockSceneEventFilter = false;
}

void GroupingContainmentPrivate::onWidgetMoved(QGraphicsWidget *widget)
{
    if (movingWidget != widget) {
        return;
    }

    movingWidget = 0;
    interestingWidget = 0;
    movementHelperWidget->setZValue(0);

    if (interestingGroup) {
        AbstractGroup *interesting = interestingGroup.data();

        QGraphicsItem *parent = widget->parentItem();
        QPointF initialPos(widget->pos());

        blockSceneEventFilter = true;

        //removing the handle if changing group, because the new group could provide a different type.
        //would like to find a way to know if it is the case, but don't know how.
        if (interesting != widget->property("group").value<AbstractGroup *>()) {
            Handle *h = handles.value(widget);
            if (h) {
                h->deleteLater();
                handles.remove(widget);
            }
        }

        if (q->corona()->immutability() == Plasma::Mutable) {
            Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);
            AbstractGroup *group = static_cast<AbstractGroup *>(widget);
            if (applet) {
                interesting->addApplet(applet, false);
            } else if (!group->isAncestorOf(interesting) && interesting != group) {
                interesting->addSubGroup(group, false);
            } else {
                interestingGroup.clear();
                return;
            }
        }

        QRectF geom(widget->boundingRect());

        QRectF rect(interesting->contentsRect());
        QPointF pos = interesting->mapFromItem(parent, initialPos);
        if (pos.x() < rect.left()) {
            pos.setX(rect.left());
        }
        if (pos.y() < rect.top()) {
            pos.setY(rect.top());
        }
        interesting->layoutChild(widget, pos);
        interesting->save(*(interesting->d->mainConfigGroup()));
        interesting->saveChildren();

        geom.translate(widget->parentItem()->mapFromItem(parent, initialPos));
        QRectF newGeom(widget->geometry());

        if (geom != newGeom) {
            Plasma::Animation *anim = Plasma::Animator::create(Plasma::Animator::GeometryAnimation);
            if (anim) {
                q->connect(anim, SIGNAL(finished()), q, SLOT(widgetMovedAnimationComplete()));
                anim->setTargetWidget(widget);
                anim->setProperty("startGeometry", geom);
                anim->setProperty("targetGeometry", newGeom);
                anim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        } else {
            blockSceneEventFilter = false;
        }

        interestingGroup.clear();

    } else {
        widget->setParentItem(q);
    }

    emit q->configNeedsSaving();
}

void GroupingContainmentPrivate::onImmutabilityChanged(Plasma::ImmutabilityType immutability)
{
    newGroupAction->setVisible(immutability == Plasma::Mutable);

    if (immutability != Plasma::Mutable) {
        foreach (Handle *handle, handles) {
            handleDisappeared(handle);
        }
    }
}

void GroupingContainmentPrivate::restoreGroups()
{
    KConfigGroup groupsConfig = q->config("Groups");
    foreach (AbstractGroup *group, groups) {
        KConfigGroup groupConfig(&groupsConfig, QString::number(group->id()));
        KConfigGroup groupInfoConfig(&groupConfig, "GroupInformation");

        if (groupInfoConfig.isValid() && groupInfoConfig.exists()) {
            int groupId = groupInfoConfig.readEntry("Group", -1);

            if (groupId != -1) {
                AbstractGroup *parentGroup = 0;
                foreach (AbstractGroup *g, groups) {
                    if ((int)g->id() == groupId) {
                        parentGroup = g;
                        break;
                    }
                }
                if (parentGroup) {
                    QTransform t = group->transform();
                    parentGroup->addSubGroup(group, false);
                    group->setTransform(t);
                } else {
                    groupInfoConfig.deleteGroup();
                    manageGroup(group, group->pos());
                }
            }
        } else if (group != mainGroup) {
            manageGroup(group, group->pos());
        }
    }

    KConfigGroup appletsConfig = q->config("Applets");
    foreach (Plasma::Applet *applet, q->applets()) {
        KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
        KConfigGroup groupConfig(&appletConfig, "GroupInformation");

        if (groupConfig.isValid() && groupConfig.exists()) {
            int groupId = groupConfig.readEntry("Group", -1);

            if (groupId != -1) {
                AbstractGroup *group = 0;
                foreach (AbstractGroup *g, groups) {
                    if ((int)g->id() == groupId) {
                        group = g;
                        break;
                    }
                }
                if (group) {
                    QTransform t = applet->transform();
                    group->addApplet(applet, false);
                    applet->setTransform(t);
                } else {
                    groupConfig.deleteGroup();
                    manageApplet(applet, applet->pos());
                }
            }
        } else { //happens when changing a desktop activity to GroupingDesktop
            manageApplet(applet, applet->pos());
        }
    }

    foreach (AbstractGroup *group, groups) {
        group->d->restoreChildren();
    }

    //since a Main Group won't have any modification that would cause it to save its settings
    //we make sure here it saves all it has to save.
    AbstractGroup *group = mainGroup;
    if (group) {
        KConfigGroup cg;
        group->save(cg);
        group->saveChildren();
        emit q->configNeedsSaving();
    }
}

void GroupingContainmentPrivate::prepareWidgetToMove()
{
    q->raise(widgetToBeSetMoving);
    q->raise(movementHelperWidget);

    //need to do do this because when you have, e.g a flow group in a flow group in a flow group,
    //when you move the top one outside of the mid one boundaries appears the
    //lower one's spacer that causes the mid one to move and to propagate the move to its children,
    //so the upper one would move too. It could be that after the move the lower one decides
    //that the spacer has to go, causing the mid one to return to its original position and
    //causing the spacer to flicker. setting the third one's parent to movementHelperWidget
    //resolves this. i use that widget and not "q" or others because, setting its position equal
    //to the parentItem's one, it doesn't break the movement via ItemIsMovable.
    if (q->immutability() == Plasma::Mutable) {
        movementHelperWidget->setTransform(QTransform());
        QGraphicsItem *parent = widgetToBeSetMoving->parentItem();
        QTransform t(parent->itemTransform(movementHelperWidget));
        QTransform tr(t.m11(), t.m12(), t.m21(), t.m22(), 0, 0);
        movementHelperWidget->setTransform(tr);
        QPointF p(q->mapFromItem(parent, QPointF(0, 0)));
        movementHelperWidget->setPos(p);
        movementHelperWidget->setMinimumSize(widgetToBeSetMoving->size());
        widgetToBeSetMoving->setParentItem(movementHelperWidget);
    }

    interestingGroup = widgetToBeSetMoving->property("group").value<AbstractGroup *>();
    movingWidget = widgetToBeSetMoving;

    if (q->immutability() != Plasma::Mutable) {
        onWidgetMoved(widgetToBeSetMoving);
    }

    widgetToBeSetMoving = 0;
}

//------------------------GroupingContainment------------------------------

GroupingContainment::GroupingContainment(QObject* parent, const QVariantList& args)
               : Containment(parent, args),
                 d(new GroupingContainmentPrivate(this))
{
    setContainmentType(Plasma::Containment::NoContainmentType);
    useMainGroup("floating");
}

GroupingContainment::~GroupingContainment()
{
    delete d;
}

void GroupingContainment::init()
{
    Plasma::Containment::init();

    d->newGroupAction->setVisible(immutability() == Plasma::Mutable);
    addToolBoxAction(d->newGroupAction);

    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(manageApplet(Plasma::Applet*,QPointF)));
    connect(this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            this, SLOT(onImmutabilityChanged(Plasma::ImmutabilityType)));
}

void GroupingContainment::constraintsEvent(Plasma::Constraints constraints)
{
    foreach (AbstractGroup *g, d->groups) {
        g->updateConstraints(constraints);
    }
}

AbstractGroup *GroupingContainment::addGroup(const QString &plugin, const QPointF &pos, int id)
{
    return d->createGroup(plugin, pos, id);
}

void GroupingContainment::addGroup(AbstractGroup *group, const QPointF &pos)
{
    if (!group) {
        return;
    }

    kDebug()<<"adding group"<<group->id();
    connect(this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            group, SLOT(setImmutability(Plasma::ImmutabilityType)));
    connect(group, SIGNAL(groupDestroyed(AbstractGroup*)),
            this, SLOT(onGroupRemoved(AbstractGroup*)));
    connect(group, SIGNAL(appletRemovedFromGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletRemovedFromGroup(Plasma::Applet*,AbstractGroup*)));
    connect(group, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)));
    connect(group, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
    group->setPos(pos);
    group->setImmutability(immutability());
    group->updateConstraints();

    int z = group->zValue();
    if (GroupingContainmentPrivate::s_maxZValue < z) {
        GroupingContainmentPrivate::s_maxZValue = z;
    }

    emit groupAdded(group, pos);

    if (!d->loading && !pos.isNull()) {
        d->manageGroup(group, pos);
        Plasma::Animation *anim = Plasma::Animator::create(Plasma::Animator::AppearAnimation);
        if (anim) {
            anim->setTargetWidget(group);
            anim->setDirection(QAbstractAnimation::Backward);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    group->installEventFilter(this);
    group->installSceneEventFilter(this);
}

QList<AbstractGroup *> GroupingContainment::groups() const
{
    return d->groups;
}

QList<QAction *> GroupingContainment::contextualActions()
{
    QList<QAction *> list;
    list << d->newGroupAction << d->separator << d->configureGroupAction << d->deleteGroupAction;
    return list;
}

void GroupingContainment::useMainGroup(const QString &name)
{
    if (!name.isEmpty()) {
        d->mainGroupPlugin = name;
    }
}

void GroupingContainment::setMainGroup(AbstractGroup *group)
{
    if (!group) {
        return;
    }

    d->mainGroup = group;

    layoutMainGroup();
    group->setIsMainGroup();

    config().writeEntry("mainGroup", group->id());
    emit configNeedsSaving();
}

void GroupingContainment::layoutMainGroup()
{
    if (!d->layout) {
        d->layout = new QGraphicsLinearLayout(this);
        d->layout->setContentsMargins(0, 0, 0, 0);
    }
    d->layout->addItem(d->mainGroup);
}

AbstractGroup *GroupingContainment::mainGroup() const
{
    return d->mainGroup;
}

bool GroupingContainment::sceneEventFilter(QGraphicsItem* watched, QEvent* event)
{
    if (d->blockSceneEventFilter) {
        return true;
    }

    Plasma::Applet *applet = qgraphicsitem_cast<Plasma::Applet *>(watched);
    AbstractGroup *group = qgraphicsitem_cast<AbstractGroup *>(watched);

    QGraphicsWidget *widget = 0;
    if (applet) {
        widget = applet;
    } else if (group) {
        widget = group;
    }

    if (event->type() == QEvent::GraphicsSceneHoverEnter || event->type() == QEvent::GraphicsSceneHoverMove) {
        QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent *>(event);
        if (immutability() == Plasma::Mutable && ((group && !group->isMainGroup()) || applet)) {
            if (d->handles.contains(widget)) {
                Handle *handle = d->handles.value(widget);
                if (handle) {
                    handle->setHoverPos(he->pos());
                }
            } else {
//              kDebug() << "generated group handle";
                AbstractGroup *parent = widget->property("group").value<AbstractGroup *>();
                if (parent) {
                    Handle *handle = parent->createHandleForChild(widget);
                    if (handle) {
                        handle->setHoverPos(he->pos());
                        d->handles[widget] = handle;
                        connect(handle, SIGNAL(disappearDone(Handle*)),
                                this, SLOT(handleDisappeared(Handle*)));
                        connect(widget, SIGNAL(geometryChanged()),
                                handle, SLOT(widgetResized()));
                        connect(handle, SIGNAL(widgetMoved(QGraphicsWidget*)),
                                this, SLOT(onWidgetMoved(QGraphicsWidget*)));
                    }
                }
            }
        }

        foreach (Handle *handle, d->handles) {
            QGraphicsWidget *w = d->handles.key(handle);
            if (w != widget && handle) {
                handle->setHoverPos(w->mapFromScene(he->scenePos()));
            }
        }
    }

    return false;
}

bool GroupingContainment::eventFilter(QObject *obj, QEvent *event)
{
    if (immutability() != Plasma::Mutable) {
        return false;
    }

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
            case QEvent::GraphicsSceneMousePress:
                if (static_cast<QGraphicsSceneMouseEvent *>(event)->button() == Qt::LeftButton &&
                    immutability() == Plasma::Mutable) {
                    d->interestingWidget = widget;
                }
            break;

            case QEvent::GraphicsSceneMove: {
                if (d->movingWidget != widget && widget == d->interestingWidget) {
                    setMovingWidget(widget);
                }
                if (widget == d->movingWidget) {
                    //use the center of the widget to find the possible groups, but pass to
                    //their showDropZone the actual position of the widget, modulo an
                    //eventual shift to make it fit into their contentsRect
                    QPointF p = mapFromScene(widget->scenePos());
                    QRectF rect(widget->contentsRect());
                    QList<AbstractGroup *> groups = d->groupsAt(p + rect.center(), widget);
                    foreach (AbstractGroup *parentGroup ,groups) {
                        if (d->interestingGroup && d->interestingGroup.data() != parentGroup) {
                            d->interestingGroup.data()->showDropZone(QPointF());
                            d->interestingGroup.clear();
                        }
                        QPointF pos = mapToItem(parentGroup, p + rect.topLeft());
                        if (pos.x() < rect.left()) {
                            pos.setX(rect.left());
                        }
                        if (pos.y() < rect.top()) {
                            pos.setY(rect.top());
                        }
                        if (parentGroup->showDropZone(pos)) {
                            d->interestingGroup = parentGroup;
                            break;
                        }
                    }
                }
            }

            break;

            case QEvent::GraphicsSceneDragMove: {
                QGraphicsSceneDragDropEvent *e = static_cast<QGraphicsSceneDragDropEvent *>(event);
                bool ok = true;
                const QMimeData *mime = e->mimeData();
                if (mime->hasFormat(AbstractGroup::mimeType())) {
                    QString name = mime->data(AbstractGroup::mimeType());
                    GroupInfo gi = AbstractGroup::groupInfo(name);
                    if (!gi.formFactors().contains(formFactor())) {
                        ok = false;
                    }
                }

                if (ok) {
                    QPointF pos(mapFromScene(e->scenePos()));
                    QList<AbstractGroup *> groups = d->groupsAt(pos);
                    foreach (AbstractGroup *group, groups) {
                        if (d->interestingGroup && d->interestingGroup.data() != group) {
                            d->interestingGroup.data()->showDropZone(QPointF());
                            d->interestingGroup.clear();
                        }
                        if (group->showDropZone(mapToItem(group, pos))) {
                            d->interestingGroup = group;
                            break;
                        }
                    }
                } else {
                    e->ignore();
                }
            }
            break;

            case QEvent::GraphicsSceneDrop: {
                if (group) {
                    QGraphicsSceneDragDropEvent *e = static_cast<QGraphicsSceneDragDropEvent *>(event);
                    e->setPos(mapFromScene(e->scenePos()));
                    dropEvent(e);
                }
                if (d->interestingGroup) {
                    d->interestingGroup.data()->showDropZone(QPointF());
                    d->interestingGroup.clear();
                }
            }
            break;

            case QEvent::GraphicsSceneMouseRelease:
                if (d->movingWidget) {
                    d->onWidgetMoved(widget);
                }
                d->interestingWidget = 0;

            break;

            default:
            break;
        }
    }

    return Plasma::Containment::eventFilter(obj, event);
}

void GroupingContainment::saveContents(KConfigGroup &group) const
{
    Plasma::Containment::saveContents(group);

    KConfigGroup groupsConfig(&group, "Groups");
    foreach (AbstractGroup *g, d->groups) {
        g->save(*(g->d->mainConfigGroup()));
        g->saveChildren();
    }
}

void GroupingContainment::restoreContents(KConfigGroup &group)
{
    Plasma::Containment::restoreContents(group);

    d->mainGroupId = group.readEntry("mainGroup", 0);

    KConfigGroup groupsConfig(&group, "Groups");
    foreach (const QString &groupId, groupsConfig.groupList()) {
        int id = groupId.toInt();
        KConfigGroup groupConfig(&groupsConfig, groupId);
        QString plugin = groupConfig.readEntry("plugin", QString());

        AbstractGroup *group = d->createGroup(plugin, QPointF(), id);
        if (group) {
            group->restore(groupConfig);
        }
    }

    if (d->mainGroupId != 0 && !d->mainGroup) {
        foreach (AbstractGroup *group, d->groups) {
            if (group->id() == d->mainGroupId) {
                setMainGroup(group);
            }
        }
    }

    if (!d->mainGroupPlugin.isEmpty() && !d->mainGroup) {
        AbstractGroup *group = d->createGroup(d->mainGroupPlugin, QPointF(), 0, true);
        setMainGroup(group);
        group->init();
    } else if (d->mainGroup && d->mainGroupPlugin != d->mainGroup->pluginName()) {
        //switching activity between containments with different main groups
        AbstractGroup *old = d->mainGroup;
        AbstractGroup *group = d->createGroup(d->mainGroupPlugin, QPointF(), 0, true);
        setMainGroup(group);
        group->init();

        //don't call destroy() because it will wait for the anim to finish before actually destroying it
        //and we don't have time to wait here
        old->d->destroyGroup();
    }
    if (!d->mainGroup) {
        kWarning()<<"You have not set a Main Group! This will really cause troubles! You *must* set a Main Group!";
    }

    //delay so to allow the applets and subGroup to prepare themselves.
    //without this PopupApplets in GridGroups would be restored always expanded,
    //even if they were iconified the last session.
    QTimer::singleShot(0, this, SLOT(restoreGroups()));

    d->loading = false;
}

void GroupingContainment::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    d->deleteGroupAction->setVisible(false);
    d->configureGroupAction->setVisible(false);
    d->lastClick = event->pos();

    AbstractGroup *group = d->groupAt(event->pos());

    while (group && group->isMainGroup()) {
        group = group->parentGroup();
        if (!group) {
            break;
        }
    }

    if (group && (immutability() == Plasma::Mutable) && (group->immutability() == Plasma::Mutable) && !group->isMainGroup()) {
        d->deleteGroupAction->setVisible(true);
        d->deleteGroupAction->setData(group->id());
        d->deleteGroupAction->setText(i18n("Remove this %1",
                                           AbstractGroup::groupInfo(group->pluginName()).prettyName()));
        if (group->hasConfigurationInterface()) {
            d->configureGroupAction->setVisible(true);
            d->configureGroupAction->setText(i18n("Configure this %1",
                                                  AbstractGroup::groupInfo(group->pluginName()).prettyName()));
            d->configureGroupAction->setData(group->id());
        }
    }

    event->ignore();

    Plasma::Containment::contextMenuEvent(event);
}

void GroupingContainment::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (mime->hasFormat(AbstractGroup::mimeType())) {
        QString name = mime->data(AbstractGroup::mimeType());
        GroupInfo gi = AbstractGroup::groupInfo(name);
        if (gi.formFactors().contains(formFactor())) {
            d->createGroup(name, event->pos(), 0);
        }
    } else {
        Plasma::Containment::dropEvent(event);
    }
}

void GroupingContainment::setMovingWidget(QGraphicsWidget *widget)
{
    d->interestingWidget = 0;

    if (d->movingWidget) {
        if (d->movingWidget == widget) {
            return;
        }
        d->onWidgetMoved(d->movingWidget);
    }

    AbstractGroup *group = widget->property("group").value<AbstractGroup *>();
    if (group) {
        group->releaseChild(widget);
    }

    d->widgetToBeSetMoving = widget;
    //delay so to allow the widget to receive and react to the events caused by the changes
    //its group may do in releaseChild
    QTimer::singleShot(0, this, SLOT(prepareWidgetToMove()));
}

QGraphicsWidget *GroupingContainment::movingWidget() const
{
    return d->movingWidget;
}

void GroupingContainment::raise(QGraphicsWidget *widget)
{
    widget->setZValue(++GroupingContainmentPrivate::s_maxZValue);
}

#include "groupingcontainment.moc"
