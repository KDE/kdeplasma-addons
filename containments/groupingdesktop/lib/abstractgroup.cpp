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

#include <KDE/KConfigDialog>
#include <KDE/KWindowSystem>
#include <kservice.h>
#include <kservicetypetrader.h>

#include <Plasma/Containment>
#include <Plasma/FrameSvg>
#include <Plasma/Animator>
#include <Plasma/Animation>

#include "groupingcontainment.h"
#include "freehandle.h"

AbstractGroupPrivate::AbstractGroupPrivate(AbstractGroup *group)
    : q(group),
      destroying(false),
      containment(0),
      immutability(Plasma::Mutable),
      groupType(AbstractGroup::FreeGroup),
      interestingGroup(0),
      isMainGroup(false),
      backgroundHints(AbstractGroup::NoBackground),
      isLoading(true),
      hasInterface(false),
      m_mainConfig(0)
{
    background = new Plasma::FrameSvg(q);
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

    q->scene()->removeItem(q);
    delete q;
}

void AbstractGroupPrivate::startDestroyAnimation()
{
    Plasma::Animation *zoomAnim = Plasma::Animator::create(Plasma::Animator::ZoomAnimation);
    q->connect(zoomAnim, SIGNAL(finished()), q, SLOT(destroyGroup()));
    zoomAnim->setTargetWidget(q);
    zoomAnim->start();
}

void AbstractGroupPrivate::appletDestroyed(Plasma::Applet *applet)
{
    if (applets.contains(applet)) {
        kDebug()<<"removed applet"<<applet->id()<<"from group"<<id<<"of type"<<q->pluginName();

        applets.removeAll(applet);

        emit q->appletRemovedFromGroup(applet, q);

        q->saveChildren();
        emit q->configNeedsSaving();

        if (destroying && (q->children().count() == 0)) {
            startDestroyAnimation();
            destroying = false;
        }
    }
}

void AbstractGroupPrivate::subGroupDestroyed(AbstractGroup *subGroup)
{
    if (subGroups.contains(subGroup)) {
        kDebug()<<"removed sub group"<<subGroup->id()<<"from group"<<id<<"of type"<<q->pluginName();

        subGroups.removeAll(subGroup);

        emit q->subGroupRemovedFromGroup(subGroup, q);

        q->saveChildren();
        emit q->configNeedsSaving();

        if (destroying && (q->children().count() == 0)) {
            startDestroyAnimation();
            destroying = false;
        }
    }
}

void AbstractGroupPrivate::addChild(QGraphicsWidget *child)
{
    QPointF newPos = q->mapFromItem(child->parentItem(), child->pos());
    child->setParentItem(q);
    child->setProperty("group", QVariant::fromValue(q));
    child->setPos(newPos);

    if (groupType == AbstractGroup::FreeGroup) {
        q->connect(child, SIGNAL(geometryChanged()), q, SLOT(onChildGeometryChanged()));
    }
}

void AbstractGroupPrivate::removeChild(QGraphicsWidget *child)
{
    QPointF newPos = child->scenePos();
    child->setParentItem(q->parentItem());
    child->setPos(child->parentItem()->mapFromScene(newPos));

    child->disconnect(q);
}

void AbstractGroupPrivate::setIsMainGroup()
{
    isMainGroup = true;
    q->setBackgroundHints(AbstractGroup::NoBackground);
    q->setFlag(QGraphicsItem::ItemIsMovable, false);
    q->setZValue(0);
    q->setContentsMargins(0, 0, 0, 0);
}

void AbstractGroupPrivate::onInitCompleted()
{
    isLoading = false;

    if (background && (containment->formFactor() == Plasma::Vertical ||
                       containment->formFactor() == Plasma::Horizontal)) {
        q->setBackgroundHints(AbstractGroup::PlainBackground);
    }
}

void AbstractGroupPrivate::onChildGeometryChanged()
{
    q->saveChildren();
    emit q->configNeedsSaving();
}

QString AbstractGroupPrivate::configDialogId()
{
    return QString("%1settings").arg(id);
}

//-----------------------------AbstractGroup------------------------------

AbstractGroup::AbstractGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
             : QGraphicsWidget(parent, wFlags),
               d(new AbstractGroupPrivate(this))
{

    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setContentsMargins(10, 10, 10, 10);
    setBackgroundHints(StandardBackground);

    connect(this, SIGNAL(initCompleted()), this, SLOT(onInitCompleted()));
}

AbstractGroup::~AbstractGroup()
{
    emit groupDestroyed(this);

    delete KConfigDialog::exists(d->configDialogId());
    delete d;
}

void AbstractGroup::init()
{

}

void AbstractGroup::setImmutability(Plasma::ImmutabilityType immutability)
{
    if (!isMainGroup()) {
        setFlag(QGraphicsItem::ItemIsMovable, immutability == Plasma::Mutable);
    }
    d->immutability = immutability;

    foreach (Plasma::Applet *applet, applets()) {
        applet->setImmutability(immutability);
    }
    foreach (AbstractGroup *group, subGroups()) {
        group->setImmutability(immutability);
    }

    emit immutabilityChanged(immutability);
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
    if (!applet) {
        return;
    }

    if (applets().contains(applet)) {
        if (applet->parentItem() != this) {
            QPointF p(mapFromScene(applet->scenePos()));
            applet->setParentItem(this);
            applet->setPos(p);
        }

        return;
    }

    QVariant pGroup = applet->property("group");
    if (pGroup.isValid()) {
        pGroup.value<AbstractGroup *>()->removeApplet(applet);
    }

    kDebug()<<"adding applet"<<applet->id()<<"in group"<<id()<<"of type"<<pluginName();

    d->applets << applet;
    d->addChild(applet);

    emit appletAddedInGroup(applet, this);

    if (layoutApplet) {
        layoutChild(applet, applet->pos());

        save(*(d->mainConfigGroup()));
        saveChildren();
        emit configNeedsSaving();
    }

    connect(applet, SIGNAL(appletDestroyed(Plasma::Applet*)),
            this, SLOT(appletDestroyed(Plasma::Applet*)));
}

void AbstractGroup::addSubGroup(AbstractGroup *group, bool layoutGroup)
{
    if (!group) {
        return;
    }

    if (subGroups().contains(group)) {
        if (group->parentItem() != this) {
            QPointF p(mapFromScene(group->scenePos()));
            group->setParentItem(this);
            group->setPos(p);
        }

        return;
    }

    QVariant pGroup = group->property("group");
    if (pGroup.isValid()) {
        pGroup.value<AbstractGroup *>()->removeSubGroup(group);
    }

    kDebug()<<"adding sub group"<<group->id()<<"in group"<<id()<<"of type"<<pluginName();

    d->subGroups << group;
    d->addChild(group);

    emit subGroupAddedInGroup(group, this);

    if (layoutGroup) {
        layoutChild(group, group->pos());

        save(*(d->mainConfigGroup()));
        saveChildren();
        emit configNeedsSaving();
    }

    connect(group, SIGNAL(groupDestroyed(AbstractGroup*)),
            this, SLOT(subGroupDestroyed(AbstractGroup*)));
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

void AbstractGroup::removeApplet(Plasma::Applet *applet, AbstractGroup *newGroup)
{
    kDebug()<<"removing applet"<<applet->id()<<"from group"<<id()<<"of type"<<pluginName();

    d->applets.removeAll(applet);
    KConfigGroup appletConfig = applet->config().parent();
    KConfigGroup groupConfig(&appletConfig, QString("GroupInformation"));
    groupConfig.deleteGroup();

    if (newGroup) {
        newGroup->addApplet(applet);
    } else {
        d->removeChild(applet);
    }

    emit appletRemovedFromGroup(applet, this);

    saveChildren();
    emit configNeedsSaving();
}

void AbstractGroup::removeSubGroup(AbstractGroup *subGroup, AbstractGroup *newGroup)
{
    kDebug()<<"removing sub group"<<subGroup->id()<<"from group"<<id()<<"of type"<<pluginName();

    d->subGroups.removeAll(subGroup);
    KConfigGroup subGroupConfig = subGroup->config().parent();
    KConfigGroup groupConfig(&subGroupConfig, QString("GroupInformation"));
    groupConfig.deleteGroup();

    if (newGroup) {
        newGroup->addSubGroup(subGroup);
    } else {
        d->removeChild(subGroup);
    }

    emit subGroupRemovedFromGroup(subGroup, this);

    saveChildren();
    emit configNeedsSaving();
}

void AbstractGroup::destroy()
{
    kDebug()<<"destroying group"<<id()<<"of type"<<pluginName();

    d->destroying = true;

    if (children().count() == 0) {
        d->startDestroyAnimation();
        return;
    }

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

    group.writeEntry("zvalue", zValue());
    group.writeEntry("plugin", pluginName());
    QRectF rect = boundingRect();
    rect.translate(containment()->mapFromScene(scenePos()));
    group.writeEntry("geometry", rect);
}

void AbstractGroup::saveChildren() const
{
    foreach (Plasma::Applet *applet, d->applets) {
        KConfigGroup appletConfig = applet->config().parent();
        KConfigGroup groupConfig(&appletConfig, QString("GroupInformation"));
        groupConfig.writeEntry("Group", id());
        saveChildGroupInfo(applet, groupConfig);
    }
    foreach (AbstractGroup *subGroup, d->subGroups) {
        KConfigGroup subGroupConfig = subGroup->config().parent();
        KConfigGroup groupConfig(&subGroupConfig, QString("GroupInformation"));
        groupConfig.writeEntry("Group", id());
        saveChildGroupInfo(subGroup, groupConfig);
    }
}

void AbstractGroup::restore(KConfigGroup &group)
{
    qreal z = group.readEntry("zvalue", 0);

    if (z > 0) {
        setZValue(z);
    }
}

void AbstractGroup::showDropZone(const QPointF &pos)
{
    Q_UNUSED(pos)

    //base implementation does nothing
}

void AbstractGroup::raise()
{
    containment()->raise(this);
}

Handle *AbstractGroup::createHandleForChild(QGraphicsWidget *child)
{
    if (!children().contains(child)) {
        return 0;
    }

    Plasma::Applet *a = qobject_cast<Plasma::Applet *>(child);
    if (a) {
        return new FreeHandle(containment(), a);
    }

    return new FreeHandle(containment(), static_cast<AbstractGroup *>(child));
}

void AbstractGroup::setGroupType(AbstractGroup::GroupType type)
{
    d->groupType = type;
}

AbstractGroup::GroupType AbstractGroup::groupType() const
{
    return d->groupType;
}

bool AbstractGroup::isMainGroup() const
{
    return d->isMainGroup;
}

void AbstractGroup::setBackgroundHints(BackgroundHints hints)
{
    d->backgroundHints = hints;
    switch (hints) {
        case StandardBackground:
            d->background->setImagePath("widgets/translucentbackground");
            d->background->setElementPrefix(QString());
        break;

        case PlainBackground:
            d->background->setImagePath("widgets/frame");
            d->background->setElementPrefix("sunken");
        break;

        default:
        break;
    }

    update();
}

AbstractGroup::BackgroundHints AbstractGroup::backgroundHints() const
{
    return d->backgroundHints;
}

void AbstractGroup::dragLeaveEvent(QGraphicsSceneDragDropEvent *)
{
    showDropZone(QPointF());
}

void AbstractGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->background->resizeFrame(event->newSize());

    if (!d->isLoading && !d->destroying) {
        emit geometryChanged();

        save(*(d->mainConfigGroup()));
        emit configNeedsSaving();
    }
}

void AbstractGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        event->accept();
    }
}

void AbstractGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseMoveEvent(event);

    if (event->button() == Qt::LeftButton) {
        containment()->setMovingWidget(this);
    }
}

int AbstractGroup::type() const
{
    return Type;
}

QVariant AbstractGroup::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
        case ItemPositionChange:
            if (isMainGroup() || immutability() != Plasma::Mutable) {
                return pos();
            }
        break;

        case ItemPositionHasChanged:
            emit geometryChanged();

        break;

        case ItemTransformChange:
            if (immutability() != Plasma::Mutable) {
                return transform();
            }
        break;

        default:
        break;
    }

    return QGraphicsWidget::itemChange(change, value);
}

void AbstractGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (d->backgroundHints == NoBackground) {
        return;
    }

    d->background->paintFrame(painter);
}

void AbstractGroup::showConfigurationInterface()
{
    KConfigDialog *dlg = KConfigDialog::exists(d->configDialogId());

    if (dlg) {
        KWindowSystem::setOnDesktop(dlg->winId(), KWindowSystem::currentDesktop());
        dlg->show();
        KWindowSystem::activateWindow(dlg->winId());
        return;
    }

    KConfigSkeleton *nullManager = new KConfigSkeleton(0);
    KConfigDialog *dialog = new KConfigDialog(0, d->configDialogId(), nullManager);
    dialog->setFaceType(KPageDialog::Auto);
    dialog->setWindowTitle(i18n("Group Configuration"));
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    createConfigurationInterface(dialog);
    //TODO: Apply button does not correctly work for now, so do not show it
    dialog->showButton(KDialog::Apply, false);
    dialog->showButton(KDialog::Default, false);
    QObject::connect(dialog, SIGNAL(finished()), nullManager, SLOT(deleteLater()));

    dialog->show();
}

void AbstractGroup::createConfigurationInterface(KConfigDialog *)
{

}

bool AbstractGroup::hasConfigurationInterface() const
{
    return d->hasInterface;
}

void AbstractGroup::setHasConfigurationInterface(bool hasInterface)
{
    d->hasInterface = hasInterface;
}

#include "abstractgroup.moc"
