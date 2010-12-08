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

#include <cmath>
#include <math.h>

AbstractGroupPrivate::AbstractGroupPrivate(AbstractGroup *group)
    : q(group),
      parentGroup(0),
      destroying(false),
      background(0),
      immutability(Plasma::Mutable),
      groupType(AbstractGroup::FreeGroup),
      interestingGroup(0),
      isMainGroup(false),
      backgroundHints(AbstractGroup::NoBackground),
      isLoading(true),
      hasInterface(false),
      simplerBackgroundChildren(false),
      m_mainConfig(0)
{

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

    KConfigGroup containmentGroup = q->containment()->config();
    KConfigGroup groupsConfig = KConfigGroup(&containmentGroup, "Groups");
    m_mainConfig = new KConfigGroup(&groupsConfig, QString::number(id));

    return m_mainConfig;
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
        KConfigGroup subGroupConfig = subGroup->config().parent();
        KConfigGroup groupConfig(&subGroupConfig, QString("GroupInformation"));
        groupConfig.deleteGroup();

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
    if (groupType == AbstractGroup::ConstrainedGroup) {
        child->setTransform(QTransform());
    } else {
        QTransform t(child->itemTransform(q));
        if (t.m11() != 0) {
            qreal angle = (t.m12() > 0 ? acos(t.m11()) : -acos(t.m11()));
            QTransform at;
            QSizeF size(child->size());
            at.translate(size.width() / 2, size.height() / 2);
            at.rotateRadians(angle);
            at.translate(-size.width() / 2, -size.height() / 2);
            child->setTransform(at);
            newPos -= QPointF(at.dx(), at.dy());
        }
    }
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
    QGraphicsItem *parent = q->parentItem();
    QTransform t(child->itemTransform(parent));
    if (t.m11() != 0) {
        qreal angle = (t.m12() > 0 ? acos(t.m11()) : -acos(t.m11()));
        QTransform at;
        QSizeF size(child->size());
        at.translate(size.width() / 2, size.height() / 2);
        at.rotateRadians(angle);
        at.translate(-size.width() / 2, -size.height() / 2);
        child->setTransform(at);
    }
    child->setParentItem(parent);
    child->setPos(parent->mapFromScene(newPos));

    child->disconnect(q);
}

void AbstractGroupPrivate::restoreChildren()
{
    q->restoreChildren();

    isLoading = false;
    setBackground();
    q->update();
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

void AbstractGroupPrivate::setChildBorders(Plasma::Applet *a, bool added)
{
    if (added) {
        savedAppletsHints.insert(a, a->backgroundHints());
        a->setBackgroundHints(Plasma::Applet::NoBackground);
    } else {
        if (savedAppletsHints.contains(a)) {
            a->setBackgroundHints(savedAppletsHints.value(a));
        }
    }
}

void AbstractGroupPrivate::setChildBorders(AbstractGroup *g, bool added)
{
    if (added) {
        savedGroupsHints.insert(g, g->backgroundHints());
        g->setBackgroundHints(AbstractGroup::PlainBackground);
    } else {
        if (savedGroupsHints.contains(g)) {
            g->setBackgroundHints(savedGroupsHints.value(g));
        }
    }
}

void AbstractGroupPrivate::setBackground()
{
    if ((backgroundHints & AbstractGroup::StandardBackground) ||
        (backgroundHints & AbstractGroup::PlainBackground)) {
        if (!background) {
            background = new Plasma::FrameSvg(q);
            background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        }

        if (backgroundHints & AbstractGroup::StandardBackground) {
            background->setImagePath("widgets/translucentbackground");
            background->setElementPrefix(QString());
        } else {
            background->setImagePath("widgets/frame");
            background->setElementPrefix("sunken");
        }

        qreal left, top, right, bottom;
        background->getMargins(left, top, right, bottom);
        q->setContentsMargins(left, right, top, bottom);
        background->resizeFrame(q->boundingRect().size());
    } else if (background) {
        delete background;
        background = 0;
        q->setContentsMargins(0, 0, 0, 0);
    }
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
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(20, 20);
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

AbstractGroup *AbstractGroup::parentGroup() const
{
    return d->parentGroup;
}

void AbstractGroup::addApplet(Plasma::Applet *applet, bool layoutApplet)
{
    if (!applet) {
        return;
    }

    if (applets().contains(applet)) {
        if (applet->parentItem() != this) {
            applet->setParentItem(this);
        }

        return;
    }

    QVariant pGroup = applet->property("group");
    if (pGroup.isValid()) {
        pGroup.value<AbstractGroup *>()->removeApplet(applet);
    }

    kDebug()<<"adding applet"<<applet->id()<<"in group"<<id()<<"of type"<<pluginName();

    if (d->simplerBackgroundChildren) {
        d->setChildBorders(applet, true);
    }

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
            group->setParentItem(this);
        }

        return;
    }

    AbstractGroup *parent = group->parentGroup();
    if (parent) {
        parent->removeSubGroup(group);
    }

    kDebug()<<"adding sub group"<<group->id()<<"in group"<<id()<<"of type"<<pluginName();

    if (d->simplerBackgroundChildren) {
        d->setChildBorders(group, true);
    }

    group->d->parentGroup = this;
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

    if (d->simplerBackgroundChildren) {
        d->setChildBorders(applet, false);
    }

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

    if (d->simplerBackgroundChildren) {
        d->setChildBorders(subGroup, false);
    }

    d->subGroups.removeAll(subGroup);
    KConfigGroup subGroupConfig = subGroup->config().parent();
    KConfigGroup groupConfig(&subGroupConfig, QString("GroupInformation"));
    groupConfig.deleteGroup();

    if (newGroup) {
        newGroup->addSubGroup(subGroup);
    } else {
        d->removeChild(subGroup);
        subGroup->d->parentGroup = 0;
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
    QGraphicsItem *parent = parentItem();
    GroupingContainment *c = 0;

    while (parent) {
        GroupingContainment *possibleC = dynamic_cast<GroupingContainment *>(parent);
        if (possibleC) {
            c = possibleC;
            break;
        }
        parent = parent->parentItem();
    }

    return c;
}

KConfigGroup AbstractGroup::config() const
{
    KConfigGroup config = KConfigGroup(d->mainConfigGroup(), "Configuration");

    return config;
}

void AbstractGroup::save(KConfigGroup &group) const
{
    if (d->isLoading) {
        return;
    }

    if (!group.isValid()) {
        group = *d->mainConfigGroup();
    }

    group.writeEntry("zvalue", zValue());
    group.writeEntry("plugin", pluginName());
    group.writeEntry("geometry", geometry());

    if (transform() == QTransform()) {
        group.deleteEntry("transform");
    } else {
        QList<qreal> m;
        QTransform t = transform();
        m << t.m11() << t.m12() << t.m13() << t.m21() << t.m22() << t.m23() << t.m31() << t.m32() << t.m33();
        group.writeEntry("transform", m);
    }
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
    QList<qreal> m = group.readEntry("transform", QList<qreal>());
    if (m.count() == 9) {
        QTransform t(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
        setTransform(t);
    }

    qreal z = group.readEntry("zvalue", 0);
    QRectF geom = group.readEntry("geometry", QRectF());
    setGeometry(geom);

    if (z > 0) {
        setZValue(z);
    }
}

void AbstractGroup::restoreChildren()
{
    foreach (Plasma::Applet *applet, d->applets) {
        KConfigGroup appletConfig = applet->config().parent();
        KConfigGroup groupConfig(&appletConfig, QString("GroupInformation"));
        restoreChildGroupInfo(applet, groupConfig);
    }
    foreach (AbstractGroup *subGroup, d->subGroups) {
        KConfigGroup subGroupConfig = subGroup->config().parent();
        KConfigGroup groupConfig(&subGroupConfig, QString("GroupInformation"));
        restoreChildGroupInfo(subGroup, groupConfig);
    }
}

bool AbstractGroup::showDropZone(const QPointF &pos)
{
    Q_UNUSED(pos)

    //base implementation does nothing

    return true;
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

void AbstractGroup::setIsMainGroup()
{
    d->isMainGroup = true;
    setBackgroundHints(AbstractGroup::NoBackground);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setZValue(0);
    setContentsMargins(0, 0, 0, 0);
}

bool AbstractGroup::isMainGroup() const
{
    return d->isMainGroup;
}

void AbstractGroup::setBackgroundHints(BackgroundHints hints)
{
    if (d->backgroundHints == hints) {
        return;
    }

    d->backgroundHints = hints;
    if (!d->isLoading) {
        d->setBackground();
        update();
    }
}

AbstractGroup::BackgroundHints AbstractGroup::backgroundHints() const
{
    return d->backgroundHints;
}

void AbstractGroup::setUseSimplerBackgroundForChildren(bool use)
{
    if (d->simplerBackgroundChildren != use) {
        foreach (Plasma::Applet *applet, d->applets) {
            d->setChildBorders(applet, use);
        }
        foreach (AbstractGroup *group, d->subGroups) {
            d->setChildBorders(group, use);
        }

        d->simplerBackgroundChildren = use;
    }
}

bool AbstractGroup::useSimplerBackgroundForChildren() const
{
    return d->simplerBackgroundChildren;
}

void AbstractGroup::dragLeaveEvent(QGraphicsSceneDragDropEvent *)
{
    showDropZone(QPointF());
}

void AbstractGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    if (d->background) {
        d->background->resizeFrame(event->newSize());
    }

    if (!d->isLoading && !d->destroying) {
        emit geometryChanged();

        save(*(d->mainConfigGroup()));
        emit configNeedsSaving();
    }
}

void AbstractGroup::constraintsEvent(Plasma::Constraints)
{

}

int AbstractGroup::type() const
{
    return Type;
}

QVariant AbstractGroup::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
        case ItemPositionHasChanged:
            emit geometryChanged();

        case ItemTransformHasChanged: {
            // invalid group, will result in save using the default group
            KConfigGroup cg;
            save(cg);
            emit configNeedsSaving();
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

    if (d->backgroundHints == NoBackground || !d->background) {
        return;
    }

    if (sceneTransform().isRotating()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->setRenderHint(QPainter::Antialiasing);
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

void AbstractGroup::updateConstraints(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor f = containment()->formFactor();
        if (d->background && (f == Plasma::Vertical || f == Plasma::Horizontal)) {
            if (backgroundHints() == AbstractGroup::StandardBackground) {
                setBackgroundHints(AbstractGroup::PlainBackground);
            }
        }
    }

    constraintsEvent(constraints);
}

void AbstractGroup::releaseChild(QGraphicsWidget *)
{

}

QString AbstractGroup::mimeType()
{
    return QString("plasma/group");
}

AbstractGroup *AbstractGroup::load(const QString &name, QGraphicsItem *parent)
{
    return GroupFactory::instance()->load(name, parent);
}

QStringList AbstractGroup::availableGroups()
{
    QStringList groups;

    foreach (const GroupInfo &gi, GroupFactory::instance()->groupInfos()) {
        groups << gi.name();
    }

    return groups;
}

GroupInfo AbstractGroup::groupInfo(const QString &name)
{
    foreach (const GroupInfo &gi, GroupFactory::instance()->groupInfos()) {
        if (gi.name() == name) {
            return gi;
        }
    }

    return GroupInfo("");
}

#include "abstractgroup.moc"
