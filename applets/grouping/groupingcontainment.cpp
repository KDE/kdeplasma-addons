/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "groupingcontainment.h"

#include <QDebug>
#include <QMenu>
#include <QQueue>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QStandardItemModel>

#include <KActionCollection> // Applet::actions

#include <Plasma/Corona>

GroupingContainment::GroupingContainment(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Containment(parent, data, args)
{
    setHasConfigurationInterface(true);
}

void GroupingContainment::init()
{
    migrateFromContainer();
}

void GroupingContainment::migrateFromContainer()
{
    // Search the old grouping containment containment config group
    KConfigGroup rootCg(corona()->config(), QStringLiteral("Containments"));
    // NOTE: this function is called from the constructor, so we can't use config() yet
    KConfigGroup ownCg = KConfigGroup(config());
    // old Configuration group of the old grouping containment applet
    KConfigGroup oldAppletCg(&ownCg, QStringLiteral("Configuration"));
    const uint oldContId = oldAppletCg.readEntry(QStringLiteral("ContainmentId"), 0);

    if (oldContId == 0) {
        return;
    }

    // Config group of the old grouping containment containment
    KConfigGroup oldContCg = KConfigGroup(&rootCg, QString::number(oldContId));

    // Copy everything all the applets inside, all the subgroup of any applet
    QQueue<QPair<KConfigGroup, KConfigGroup>> queue;
    queue.enqueue(qMakePair(oldContCg, ownCg));
    // Iterative Tree traversal of the grouping containment configuration
    while (!queue.isEmpty()) {
        QPair<KConfigGroup, KConfigGroup> current = queue.dequeue();
        const KConfigGroup &currentSource = current.first;
        KConfigGroup &currentDest = current.second;

        // Copy all entries in the current group
        const QMap<QString, QString> entries = currentSource.entryMap();
        for (auto it = entries.constBegin(); it != entries.constEnd(); ++it) {
            currentDest.writeEntry(it.key(), currentSource.readEntry(it.key(), QString()));
        }

        const QStringList groups = currentSource.groupList();
        for (const QString &group : groups) {
            KConfigGroup sourceSubGroup = currentSource.group(group);
            KConfigGroup destSubGroup = currentDest.group(group);
            queue.enqueue(qMakePair(sourceSubGroup, destSubGroup));
        }
    }

    // Delete the the old grouping containment
    bool oldContainmentInstantiated = false;
    for (Plasma::Containment *cont : corona()->containments()) {
        if (cont->id() == oldContId) {
            delete cont;
            break;
        }
    }
    // To make sure the old containment is really gone from the config first delete then
    // remove the group, as destroy() might fail
    rootCg.deleteGroup(QString::number(oldContId));

    // Delete the old unused key ContainmentId in this grouping containment
    oldAppletCg.deleteEntry(QStringLiteral("ContainmentId"));

    for (Applet *a : applets()) {
        a->configChanged();
    }
    rootCg.sync();
}

void GroupingContainment::newTask(const QString &task)
{
    createApplet(task, QVariantList{QStringLiteral("org.kde.plasma:force-create")});
}

void GroupingContainment::cleanupTask(const QString &task)
{
    const auto appletList = applets();
    for (Plasma::Applet *applet : appletList) {
        if (!applet->pluginMetaData().isValid() || task == applet->pluginMetaData().pluginId()) {
            applet->destroy();
        }
    }
}

void GroupingContainment::showPlasmoidMenu(QQuickItem *appletInterface, int x, int y)
{
    if (!appletInterface) {
        return;
    }

    Plasma::Applet *applet = appletInterface->property("_plasma_applet").value<Plasma::Applet *>();

    QPointF pos = appletInterface->mapToScene(QPointF(x, y));

    if (appletInterface->window() && appletInterface->window()->screen()) {
        pos = appletInterface->window()->mapToGlobal(pos.toPoint());
    } else {
        pos = QPoint();
    }

    QMenu *desktopMenu = new QMenu;
    connect(this, &QObject::destroyed, desktopMenu, &QMenu::close);
    desktopMenu->setAttribute(Qt::WA_DeleteOnClose);

    Q_EMIT applet->contextualActionsAboutToShow();
    const QList<QAction *> actions = applet->contextualActions();
    for (QAction *action : actions) {
        if (action) {
            desktopMenu->addAction(action);
        }
    }

    // add run associated action/ remove / alternatives
    desktopMenu->addActions(applet->internalActions());

    if (desktopMenu->isEmpty()) {
        delete desktopMenu;
        return;
    }

    desktopMenu->adjustSize();

    if (QScreen *screen = appletInterface->window()->screen()) {
        const QRect geo = screen->availableGeometry();

        pos =
            QPoint(qBound(geo.left(), (int)pos.x(), geo.right() - desktopMenu->width()), qBound(geo.top(), (int)pos.y(), geo.bottom() - desktopMenu->height()));
    }

    desktopMenu->popup(pos.toPoint());
}

QPointF GroupingContainment::popupPosition(QQuickItem *visualParent, int x, int y)
{
    if (!visualParent) {
        return QPointF(0, 0);
    }

    QPointF pos = visualParent->mapToScene(QPointF(x, y));

    if (visualParent->window() && visualParent->window()->screen()) {
        pos = visualParent->window()->mapToGlobal(pos.toPoint());
    } else {
        return QPoint();
    }
    return pos;
}

void GroupingContainment::reorderItemBefore(QQuickItem *before, QQuickItem *after)
{
    if (!before || !after) {
        return;
    }

    before->setVisible(false);
    before->setParentItem(after->parentItem());
    before->stackBefore(after);
    before->setVisible(true);
}

void GroupingContainment::reorderItemAfter(QQuickItem *after, QQuickItem *before)
{
    if (!before || !after) {
        return;
    }

    after->setVisible(false);
    after->setParentItem(before->parentItem());
    after->stackAfter(before);
    after->setVisible(true);
}

K_PLUGIN_CLASS_WITH_JSON(GroupingContainment, "metadata.json")

#include "groupingcontainment.moc"
