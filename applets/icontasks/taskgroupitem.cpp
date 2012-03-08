/***************************************************************************
*   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
*   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

// Own
#include "taskgroupitem.h"
#include "jobmanager.h"
#include "dockmanager.h"
#include "dockitem.h"
#include "mediabuttons.h"
#include "unity.h"

// Qt
#include <QGraphicsSceneContextMenuEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QInputDialog>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KDesktopFile>

#include "taskmanager/taskactions.h"
#include "taskmanager/taskmanager.h"
#include "taskmanager/taskgroup.h"
#include "taskmanager/abstractgroupingstrategy.h"
#include "taskmanager/taskitem.h"

#include <Plasma/Theme>
#include <Plasma/FrameSvg>
#include "tooltips/tooltipmanager.h"
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/Dialog>
#include <Plasma/WindowEffects>

#include "tasks.h"
#include "taskitemlayout.h"
#include "windowtaskitem.h"
#include "applauncheritem.h"

class DropIndicator : public QGraphicsWidget
{
public:

    DropIndicator(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0)
        : QGraphicsWidget(parent, wFlags)
        , m_size(16)
        , m_orientation(Qt::Horizontal) {
        m_svg = new Plasma::Svg();
        m_svg->setImagePath("icontasks/dropindicators");
        m_svg->setContainsMultipleImages(true);
        m_svg->resize(m_size, m_size);
        setOrientation(Qt::Horizontal);

        m_animation = new QPropertyAnimation(this, "pos", this);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        m_animation->setDuration(50);
    }

    ~DropIndicator() {
    }

    void setOrientation(Qt::Orientation orientation) {
        m_orientation = orientation;
    }

    Qt::Orientation orientation() {
        return m_orientation;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        m_svg->paint(painter, rect().topLeft(), Qt::Horizontal == m_orientation ? "horizontal-dropindicator" : "vertical-dropindicator");
    }

    void setSize(int size) {
        int sz = size;
        if (sz < 16) {
            sz = 16;
        } else if (sz > 32) {
            sz = 32;
        } else {
            sz = (sz / 4) * 4;
        }

        if (m_size != sz) {
            m_size = sz;
            m_svg->resize(m_size, m_size);
        }
    }

    void setPosition(const QRectF &r) {
        QPointF newPos = Qt::Horizontal == m_orientation
                         ? QPointF(r.x() - (m_size / 2.0), r.y() + (r.height() - m_size) / 2.0)
                         : QPointF(r.x() + (r.width() - m_size) / 2.0, r.y() - (m_size / 2.0));

        if (isVisible()) {
            if (m_animation->state() == QAbstractAnimation::Running) {
                m_animation->stop();
            }

            m_animation->setEndValue(newPos);
            m_animation->start();
        } else {
            setVisible(true);
            setPos(newPos);
        }
    }

private:
    int m_size;
    Plasma::Svg *m_svg;
    Qt::Orientation m_orientation;
    QPropertyAnimation *m_animation;
};


TaskGroupItem::TaskGroupItem(QGraphicsWidget *parent, Tasks *applet)
    : AbstractTaskItem(parent, applet),
      m_tasksLayout(0),
      m_popupMenuTimer(0),
      m_lastActivated(-1),
      m_activeTaskIndex(0),
      m_maximumRows(1),
      m_offscreenWidget(0),
      m_offscreenLayout(0),
      m_collapsed(true),
      m_mainLayout(0),
      m_popupDialog(0),
      m_updateTimer(0),
      m_changes(TaskManager::TaskUnchanged),
      m_dropIndicator(0)
{
    setAcceptDrops(true);
    setFlag(ItemClipsChildrenToShape, true);
}


TaskGroupItem::~TaskGroupItem()
{
    if (!m_offscreenLayout && !m_mainLayout) {
        // only delete this if we have neither an offscreen layout or a mainlayout
        // if we do, then they will delete the layout for us.
        delete m_tasksLayout;
    }
    close(false);
}

void TaskGroupItem::activate()
{
}

void TaskGroupItem::activateOrIconify()
{
    bool includesActive = false;
    TaskManager::ItemList items(m_group.data()->members());
    int iconified = 0;
    foreach (AbstractGroupableItem * item, items) {
        TaskManager::TaskItem *task = qobject_cast<TaskManager::TaskItem *>(item);
        if (task) {
            if (task->task()->isIconified()) {
                ++ iconified;
            }

            if (task->task()->isActive()) {
                includesActive = true;
            }
        }
    }

    if (includesActive && items.size() - iconified > iconified) {
        // iconify
        foreach (TaskManager::AbstractGroupableItem * item, items) {
            TaskManager::TaskItem *task = qobject_cast<TaskManager::TaskItem*>(item);
            if (task) {
                task->task()->setIconified(true);
            }
        }
    } else {
        // activate
        QList<WId> winOrder(KWindowSystem::stackingOrder());
        const int winCount = winOrder.size();
        TaskManager::TaskItem* sortedItems[winCount];

        memset(sortedItems, 0, sizeof(TaskManager::TaskItem*) * winCount);

        foreach (TaskManager::AbstractGroupableItem * item, items) {
            TaskManager::TaskItem *task = qobject_cast<TaskManager::TaskItem*>(item);
            if (task) {
                int index = winOrder.indexOf(task->task()->window());
                if (index != -1) {
                    sortedItems[index] = task;
                }
            }
        }

        for (int index = 0; index < winCount; ++ index) {
            TaskManager::TaskItem* task = sortedItems[index];
            if (task) {
                task->task()->activate();
            }
        }
    }
}

void TaskGroupItem::close()
{
    close(true);
}

void TaskGroupItem::close(bool hide)
{
    //kDebug();
    //close the popup if the group is removed
    if (m_popupDialog) {
        m_popupDialog->hide();
        disconnect(m_popupDialog, 0, 0, 0);
        m_popupDialog->deleteLater();
        m_popupDialog = 0;
    }

    if (m_group) {
        disconnect(m_group.data(), 0, this, 0);
    }

    if (m_updateTimer) {
        m_updateTimer->stop();
    }

    if (!isRootGroup()) {
        unregisterFromHelpers();
        if (hide) {
            setVisible(false);
        }
    }
}

bool TaskGroupItem::isRootGroup() const
{
    return m_applet == parentWidget();
}

void TaskGroupItem::updateTask(::TaskManager::TaskChanges changes)
{
    if (!m_group || isRootGroup()) {
        return;
    }

    m_changes |= changes;

    if (!m_updateTimer)  {
        m_updateTimer = new QTimer(this);
        m_updateTimer->setInterval(10);
        m_updateTimer->setSingleShot(true);
        connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
    }

    m_updateTimer->start();
}

void TaskGroupItem::checkUpdates()
{
    if (!m_group) {
        return;
    }

    bool needsUpdate = false;
    TaskFlags flags = m_flags;

    if (m_changes & TaskManager::StateChanged) {
        if (m_group.data()->isActive()) {
            flags |= TaskHasFocus;
            if (!(m_flags & TaskHasFocus)) {
                emit activated(this);
            }
        } else {
            flags &= ~TaskHasFocus;
        }

        if (m_group.data()->isMinimized()) {
            flags |= TaskIsMinimized;
        } else {
            flags &= ~TaskIsMinimized;
        }
    }

    if (m_changes & TaskManager::AttentionChanged) {
        if (m_group.data()->demandsAttention()) {
            flags |= TaskWantsAttention;
        } else {
            flags &= ~TaskWantsAttention;
        }
    }

    if (flags != m_flags) {
        needsUpdate = true;
        setTaskFlags(flags);
    }

    // basic title and icon
    if (m_changes & TaskManager::IconChanged) {
        needsUpdate = true;
    }

    if (m_changes & TaskManager::NameChanged) {
        needsUpdate = true;
    }

    if (IconTasks::ToolTipManager::self()->isVisible(this) &&
            (m_changes & TaskManager::IconChanged ||
             m_changes & TaskManager::NameChanged ||
             m_changes & TaskManager::DesktopChanged)) {
        updateToolTip();
    }

    m_changes = TaskManager::TaskUnchanged;
    if (needsUpdate) {
        //redraw
        queueUpdate();
    }
}

void TaskGroupItem::updateToolTip()
{
    if (!m_group) {
        return;
    }

    QWidget *dialog = m_applet->popupDialog();

    if (dialog && dialog->isVisible()) {
        clearToolTip();
        return;
    }

    IconTasks::ToolTipContent data;

    data.setClickable(true);
#if KDE_IS_VERSION(4, 7, 0)
    data.setInstantPopup(m_applet->instantToolTip());
#endif
    data.setHighlightWindows(m_applet->highlightWindows());
    data.setVertical(Plasma::Vertical == m_applet->formFactor());

    QMap<int, IconTasks::ToolTipContent::Window> map;

    foreach (AbstractGroupableItem * item, m_group.data()->members()) {
        TaskManager::TaskItem *taskItem = qobject_cast<TaskManager::TaskItem *>(item);
        if (taskItem && taskItem->task()) {
            map.insertMulti(taskItem->id(),
                            IconTasks::ToolTipContent::Window(taskItem->task()->window(),
                                    item->name(),
                                    item->icon().pixmap(IconTasks::ToolTipContent::iconSize(), IconTasks::ToolTipContent::iconSize()),
                                    taskItem->task()->demandsAttention(),
                                    !m_applet->groupManager().showOnlyCurrentDesktop() || !taskItem->isOnCurrentDesktop()
                                    ? taskItem->task()->desktop() : 0));
        }
    }

    data.setWindowDetailsToPreview(map.values());

    QString key = mediaButtonKey();
    if (!key.isEmpty()) {
        data.setPlayState(MediaButtons::self()->playbackStatus(key));
    }

    IconTasks::ToolTipManager::self()->setContent(this, data);
}

void TaskGroupItem::reload()
{
    if (!group()) {
        return;
    }

    QHash<AbstractGroupableItem *, AbstractTaskItem*> itemsToRemove = m_groupMembers;
    foreach (AbstractGroupableItem * item, group()->members()) {
        if (!item) {
            kDebug() << "invalid Item";
            continue;
        }

        if (itemsToRemove.contains(item)) {
            itemsToRemove.insert(item, 0);
        }
        itemAdded(item);

        if (item->itemType() == TaskManager::GroupItemType) {
            TaskGroupItem *group = qobject_cast<TaskGroupItem *>(abstractTaskItem(item));
            if (group) {
                group->reload();
            }
        }
    }

    QHashIterator<AbstractGroupableItem *, AbstractTaskItem*> it(itemsToRemove);
    while (it.hasNext()) {
        it.next();
        if (it.key() && it.value()) {
            itemRemoved(it.key());
        }
    }
}

void TaskGroupItem::setGroup(TaskManager::GroupPtr group)
{
    //kDebug();
    if (m_group.data() == group) {
        kDebug() << "already have this group!";
        return;
    }

    if (m_group) {
        disconnect(m_group.data(), 0, this, 0);
    }

    m_group = group;
    m_abstractItem = group;

    if (m_group) {
        connect(m_abstractItem, SIGNAL(destroyed(QObject*)), this, SLOT(clearAbstractItem()));
        connect(group, SIGNAL(destroyed(QObject*)), this, SLOT(clearGroup()));
        connect(group, SIGNAL(itemRemoved(AbstractGroupableItem*)), this, SLOT(itemRemoved(AbstractGroupableItem*)));
        connect(group, SIGNAL(itemAdded(AbstractGroupableItem*)), this, SLOT(itemAdded(AbstractGroupableItem*)));

        //connect(group, SIGNAL(destroyed()), this, SLOT(close()));

        connect(group, SIGNAL(changed(::TaskManager::TaskChanges)), this, SLOT(updateTask(::TaskManager::TaskChanges)));

        connect(group, SIGNAL(itemPositionChanged(AbstractGroupableItem*)), this, SLOT(itemPositionChanged(AbstractGroupableItem*)));
    }

    //Add already existing items
    reload();
    updateTask(::TaskManager::EverythingChanged);
    if (!isRootGroup()) {
        registerWithHelpers();
    }
    //kDebug() << "Task added, isActive = " << task->isActive();
}

TaskManager::GroupPtr TaskGroupItem::group() const
{
    return m_group.data();
}

void TaskGroupItem::clearGroup()
{
    //now it's useless
}

void TaskGroupItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    //kDebug();
    if (!KAuthorized::authorizeKAction("kwin_rmb") || !m_group) {
        QGraphicsWidget::contextMenuEvent(e);
        return;
    }

    Q_ASSERT(m_applet);
    //we are the master group item
    if (isRootGroup()) {
        e->ignore();
        return;
    }

    QList <QAction*> actionList;

    QAction *a = m_applet->action("configure");
    if (a && a->isEnabled()) {
        actionList.append(a);
    }

    TaskManager::BasicMenu menu(qobject_cast<QWidget*>(this), m_group.data(), &m_applet->groupManager(), actionList, getAppMenu());

    menu.adjustSize();

    if (m_applet->formFactor() != Plasma::Vertical) {
        menu.setMinimumWidth(size().width());
    }

    Q_ASSERT(m_applet->containment());
    Q_ASSERT(m_applet->containment()->corona());
    stopWindowHoverEffect();
    menu.exec(m_applet->containment()->corona()->popupPosition(this, menu.size()));
}

QHash<AbstractGroupableItem *, AbstractTaskItem*> TaskGroupItem::members() const
{
    return m_groupMembers;
}

int TaskGroupItem::count() const
{
    return m_groupMembers.count();
}

AbstractTaskItem *TaskGroupItem::createAbstractItem(TaskManager::AbstractGroupableItem *groupableItem)
{
    //kDebug() << "item to create" << groupableItem << endl;
    AbstractTaskItem *item = 0;

    if (groupableItem->itemType() == TaskManager::GroupItemType) {
        TaskGroupItem *groupItem = new TaskGroupItem(this, m_applet);
        groupItem->setGroup(static_cast<TaskManager::TaskGroup*>(groupableItem));
        item = groupItem;
    } else if (groupableItem->itemType() == TaskManager::LauncherItemType) {
        AppLauncherItem *launcherItem = new AppLauncherItem(this, m_applet, static_cast<TaskManager::LauncherItem*>(groupableItem));
        item = launcherItem;
    } else {
        TaskManager::TaskItem * taskItem = static_cast<TaskManager::TaskItem*>(groupableItem);
        //if the taskItem is not either a startup o a task, return 0;
        if (!taskItem->startup() && !taskItem->task()) {
            return item;
        }

        WindowTaskItem *windowItem = new WindowTaskItem(this, m_applet);
        windowItem->setTask(taskItem);
        item = windowItem;
    }

    if (m_collapsed) {
        item->setPreferredOffscreenSize();
    }

    return item;
}

void TaskGroupItem::itemAdded(TaskManager::AbstractGroupableItem * groupableItem)
{
    //kDebug();
    if (!m_applet) {
        kDebug() << "No applet";
        return;
    }

    //returns the corresponding item or creates a new one
    AbstractTaskItem *item = m_groupMembers.value(groupableItem);

    if (!item) {
        item = createAbstractItem(groupableItem);

        if (item) {
            connect(item, SIGNAL(activated(AbstractTaskItem*)),
                    this, SLOT(updateActive(AbstractTaskItem*)));

            TaskGroupItem *group = qobject_cast<TaskGroupItem*>(item);
            if (group) {
                connect(item, SIGNAL(changed()), this, SLOT(relayoutItems()));
            }

            if (!isRootGroup()) {
                item->setVisible(false);
            }
        } else {
            kDebug() << "invalid Item";
            return;
        }
    }

    m_groupMembers[groupableItem] = item;
    item->setParentItem(this);

    if (m_tasksLayout) {  //add to layout either for popup or expanded group
        m_tasksLayout->addTaskItem(item);
    } else { //collapsed and no layout so far
        item->hide();
        QRect rect = iconGeometry();
        item->publishIconGeometry(rect);
    }

    if (item->isActive()) {
        //kDebug() << "item is Active" ;
        m_activeTaskIndex = indexOf(item);
    } else if (!m_group || m_group.data()->members().size() == 1) {
        m_activeTaskIndex = 0;
    }

    if (collapsed()) {
        update();
    }
}

void TaskGroupItem::itemRemoved(TaskManager::AbstractGroupableItem * groupableItem)
{
    //kDebug();
    if (!m_applet) {
        kDebug() << "No Applet";
        return;
    }

    AbstractTaskItem *item = m_groupMembers.take(groupableItem);

    if (!item) {
        kDebug() << "Item not found";
        return;
    }
    //kDebug() << item->text();

    disconnect(item, 0, 0, 0);

    if (m_tasksLayout) {
        m_tasksLayout->removeTaskItem(item);

        if (m_offscreenWidget) {
            m_offscreenWidget->adjustSize();
        }

        if (m_popupDialog && m_popupDialog->isVisible() &&
                m_applet->containment() && m_applet->containment()->corona()) {
            m_popupDialog->move(m_applet->containment()->corona()->popupPosition(this, m_popupDialog->size(), Qt::AlignCenter));
        }
    }

    item->close();
    //item->deleteLater();
    QTimer::singleShot(0, item, SLOT(deleteLater()));
}

bool TaskGroupItem::isWindowItem() const
{
    return false;
}

bool TaskGroupItem::isActive() const
{
    //kDebug() << "Not Implemented";
    return false;
}

bool TaskGroupItem::windowPreviewOpen() const
{
    if (KWindowSystem::compositingActive() && isRootGroup()) {
        QHashIterator<AbstractGroupableItem *, AbstractTaskItem*> it(m_groupMembers);

        while (it.hasNext()) {
            it.next();
            AbstractTaskItem *item = it.value();
            if (!qobject_cast<AppLauncherItem *>(item) && item->isToolTipVisible()) {
                return true;
            }
        }
    }

    return false;
}

QString TaskGroupItem::appName() const
{
    if (isRootGroup()) {
        return text();
    }

    foreach (AbstractTaskItem * member, m_groupMembers) {
        QString n(member->appName());

        if (!n.isEmpty()) {
            return n;
        }
    }

    return QString();
}

KUrl TaskGroupItem::launcherUrl() const
{
    if (isRootGroup()) {
        return KUrl();
    }

    foreach (AbstractTaskItem * member, m_groupMembers) {
        KUrl u(member->launcherUrl());

        if (u.isValid()) {
            return u;
        }
    }

    return KUrl();
}

QString TaskGroupItem::windowClass() const
{
    if (isRootGroup()) {
        return QString();
    }

    foreach (AbstractTaskItem * member, m_groupMembers) {
        QString c(member->windowClass());

        if (!c.isEmpty()) {
            return c;
        }
    }

    return QString();
}

void TaskGroupItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_group) {
        return;
    }

    event->accept();
}

void TaskGroupItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_applet->rootGroupItem() == this || !m_group) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (Tasks::GC_MinMax != m_applet->groupClick() || event->modifiers() & Qt::ControlModifier) {
            bool usePresentWindows=(event->modifiers()&Qt::ControlModifier || Tasks::GC_PresentWindows == m_applet->groupClick()) &&
                    KWindowSystem::compositingActive() && Plasma::WindowEffects::isEffectAvailable(Plasma::WindowEffects::PresentWindowsGroup);

            if (usePresentWindows) {
                // Check all tasks are from this activity!
                foreach (AbstractGroupableItem * groupable, m_group.data()->members()) {
                    TaskItem * item = dynamic_cast<TaskItem*>(groupable);
                    if (item && item->task() && !item->task()->isOnCurrentActivity()) {
                        usePresentWindows=false;
                        break;
                    }
                }
            }

            if (usePresentWindows) {
                QList<WId> ids;
                foreach (AbstractGroupableItem * groupable, m_group.data()->members()) {
                    if (groupable->itemType() == TaskManager::GroupItemType) {
                        //TODO: recurse through sub-groups?
                    } else {
                        TaskItem * item = dynamic_cast<TaskItem*>(groupable);
                        if (item && item->task()) {
                            ids << item->task()->info().win();
                        }
                    }
                }

                Plasma::WindowEffects::presentWindows(m_applet->view()->winId(), ids);
            } else {
                if (m_popupMenuTimer) {
                    m_popupMenuTimer->stop();
                }
                popupMenu();
            }
        } else {
            activateOrIconify();
        }
    }

    AbstractTaskItem::mouseReleaseEvent(event);
}

void TaskGroupItem::handleActiveWindowChanged(WId id)
{
    if (!m_popupDialog) {
        return;
    }

    if (id == m_popupDialog->winId()) {
        return;
    }

    m_popupDialog->hide();

    QRect rect = iconGeometry();
    publishIconGeometry(rect);
}

void TaskGroupItem::popupMenu()
{
    //kDebug();
    if (!m_collapsed) {
        return;
    }

    if (!m_offscreenWidget) {
        foreach (AbstractTaskItem * member, m_groupMembers) {
            member->setPreferredOffscreenSize();
        }

        tasksLayout()->invalidate();
        m_tasksLayout->setOrientation(Plasma::Vertical);
        m_tasksLayout->setMaximumRows(1);
        m_offscreenWidget = new QGraphicsWidget(this);
        m_offscreenLayout = new QGraphicsLinearLayout(m_offscreenWidget);
        m_offscreenLayout->setContentsMargins(0, 0, 0, 0); //default are 4 on each side
        m_offscreenLayout->addItem(tasksLayout());
        m_offscreenWidget->setLayout(m_offscreenLayout);
        m_offscreenWidget->adjustSize();
        m_applet->containment()->corona()->addOffscreenWidget(m_offscreenWidget);
        m_offscreenLayout->activate();
    }

    if (!m_popupDialog) {
        // Initialize popup dialog
        m_popupDialog = new Plasma::Dialog(0, Qt::Popup);
        KWindowSystem::setType(m_popupDialog->winId(), NET::PopupMenu);
        connect(m_popupDialog, SIGNAL(dialogVisible(bool)), this, SLOT(popupVisibilityChanged(bool)));
        connect(m_popupDialog, SIGNAL(dialogVisible(bool)), m_applet, SLOT(setPopupDialog(bool)));
        connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(handleActiveWindowChanged(WId)));
        KWindowSystem::setState(m_popupDialog->winId(), NET::SkipTaskbar | NET::SkipPager);
        m_popupDialog->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

        int left, top, right, bottom;
        m_popupDialog->getContentsMargins(&left, &top, &right, &bottom);
        m_offscreenWidget->setMinimumWidth(size().width() - left - right);
        m_popupDialog->setGraphicsWidget(m_offscreenWidget);
    }

    if (m_popupDialog->isVisible()) {
        m_popupDialog->clearFocus();
        if (m_applet->location() != Plasma::Floating) {
            m_popupDialog->animatedHide(Plasma::locationToInverseDirection(m_applet->location()));
        } else {
            m_popupDialog->hide();
        }
    } else {
        m_tasksLayout->setOrientation(Plasma::Vertical);
        m_tasksLayout->setMaximumRows(1);
        m_offscreenWidget->layout()->activate();

        QSizeF sz(m_offscreenWidget->effectiveSizeHint(Qt::PreferredSize));
        foreach (AbstractTaskItem * member, m_groupMembers) {
            member->setPreferredOffscreenSize();
            QSizeF m = member->preferredSize();
            if (m.width() > sz.width()) {
                sz.setWidth(m.width());
            }
        }

        m_offscreenWidget->resize(sz);
        m_popupDialog->syncToGraphicsWidget();

        if (m_applet->containment() && m_applet->containment()->corona()) {
            m_popupDialog->move(m_applet->containment()->corona()->popupPosition(this, m_popupDialog->size(), Qt::AlignCenter));
        }
        KWindowSystem::setState(m_popupDialog->winId(), NET::SkipTaskbar | NET::SkipPager);
        if (m_applet->location() != Plasma::Floating) {
            m_popupDialog->animatedShow(Plasma::locationToDirection(m_applet->location()));
        } else {
            m_popupDialog->show();
        }

        m_popupDialog->raise();
    }
}

void TaskGroupItem::popupVisibilityChanged(bool visible)
{
    if (!visible) {
        QRect rect = iconGeometry();
        publishIconGeometry(rect);
        update();
    }
}

bool TaskGroupItem::focusNextPrevChild(bool next)
{
    return focusSubTask(next, false);
}

bool TaskGroupItem::focusSubTask(bool next, bool activate)
{
    const int subTasks = totalSubTasks();

    if (subTasks > 0) {
        int index = -1;

        if (subTasks > 1) {
            for (int i = 0; i < subTasks; ++i) {
                if (selectSubTask(i)->taskFlags() & TaskHasFocus) {
                    index = i;
                    break;
                }
            }
        }

        if (next) {
            ++index;

            if (index >= subTasks) {
                index = 0;
            }
        } else {
            --index;

            if (index < 0) {
                index = (subTasks - 1);
            }
        }

        AbstractTaskItem *taskItem = selectSubTask(index);

        if (taskItem) {
            taskItem->setFocus();
            m_activeTaskIndex = index;
        }

        if (activate && taskItem) {
            stopWindowHoverEffect();
            taskItem->activate();
        }

        return true;
    } else {
        return false;
    }
}

void TaskGroupItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (isRootGroup()) {
        return;
    }

    if (QPoint(event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance()) {
        return;
    } //Wait a bit before starting drag

    //kDebug();
    if (m_popupMenuTimer) {
        //kDebug() << "popupTimer stop";
        m_popupMenuTimer->stop();
    } //Wait a bit before starting drag
    AbstractTaskItem::mouseMoveEvent(event);
}

void TaskGroupItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)

    if (m_offscreenWidget && m_popupDialog) {
        int left, top, right, bottom;
        m_popupDialog->getContentsMargins(&left, &top, &right, &bottom);
        m_offscreenWidget->setMinimumWidth(size().width() - left - right);
    }

    AbstractTaskItem::resizeEvent(event);
}

void TaskGroupItem::expand()
{
    if (!collapsed()) {
        //kDebug() << "already expanded";
        return;
    }

    if (m_popupDialog) {
        m_popupDialog->hide();
    }

    if (m_offscreenLayout) {
        m_offscreenLayout->removeItem(tasksLayout());
    }

    if (!m_mainLayout) {  //this layout is needed since we can't take a layout directly from a widget without destroying it
        m_mainLayout = new QGraphicsLinearLayout(this);
        m_mainLayout->setContentsMargins(0, 0, 0, 0); //default are 4 on each side
        setLayout(m_mainLayout);
    }

    //set it back from the popup settings (always vertical and 1 row)
    tasksLayout()->setOrientation(m_applet->formFactor());
    tasksLayout()->setMaximumRows(m_maximumRows);

    m_mainLayout->addItem(tasksLayout());

    disconnect(m_applet, SIGNAL(constraintsChanged(Plasma::Constraints)), this, SLOT(constraintsChanged(Plasma::Constraints)));
    connect(m_applet, SIGNAL(constraintsChanged(Plasma::Constraints)), this, SLOT(constraintsChanged(Plasma::Constraints)));
    //connect(m_tasksLayout, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updatePreferredSize()));
    m_collapsed = false;
    tasksLayout()->layoutItems();
    //kDebug() << tasksLayout()->preferredSize() << preferredSize() << m_groupMembers.count();
    emit changed();
    checkSettings();
    //kDebug() << "expanded";
}

void TaskGroupItem::constraintsChanged(Plasma::Constraints constraints)
{
    //kDebug();
    if (constraints & Plasma::SizeConstraint && m_tasksLayout) {
        m_tasksLayout->layoutItems();
    }

    if (constraints & Plasma::FormFactorConstraint && m_tasksLayout) {
        m_tasksLayout->setOrientation(m_applet->formFactor());
        if (m_dropIndicator) {
            m_dropIndicator->setOrientation(Plasma::Vertical == m_applet->formFactor() ? Qt::Vertical : Qt::Horizontal);
        }
    }
}

void TaskGroupItem::relayoutItems()
{
    if (m_tasksLayout) {
        m_tasksLayout->layoutItems();
    }
}

TaskItemLayout *TaskGroupItem::tasksLayout()
{
    if (!m_tasksLayout) {
        m_tasksLayout = new TaskItemLayout(this, m_applet);
        m_tasksLayout->setMaximumRows(m_maximumRows);
        m_tasksLayout->setForceRows(false); // m_forceRows);
        m_tasksLayout->setOrientation(m_applet->formFactor());
    }

    return m_tasksLayout;
}

void TaskGroupItem::collapse()
{
    //kDebug() << (int)this;
    if (collapsed()) {
        //kDebug() << "already collapsed";
        return;
    }

    m_mainLayout->removeItem(tasksLayout());
    if (m_offscreenLayout) {
        m_offscreenLayout->addItem(tasksLayout());
    } else {
        foreach (AbstractTaskItem * member, m_groupMembers) {
            scene()->removeItem(member);
        }
    }

    //kDebug();
    //delete m_tasksLayout;
    disconnect(m_applet, SIGNAL(constraintsChanged(Plasma::Constraints)), this, SLOT(constraintsChanged(Plasma::Constraints)));
    m_collapsed = true;
    updatePreferredSize();
    //kDebug();
    emit changed();
    checkSettings();
}

bool TaskGroupItem::collapsed() const
{
    return m_collapsed;
}

void TaskGroupItem::toCurrentDesktop()
{
    if (!isRootGroup()) {
        foreach (AbstractTaskItem * member, m_groupMembers) {
            member->toCurrentDesktop();
        }
    }
}

void TaskGroupItem::updatePreferredSize()
{
    if (m_collapsed) {
        foreach (AbstractTaskItem * taskItem, m_groupMembers) {
            taskItem->setPreferredOffscreenSize();
        }

        //FIXME: copypaste from abstracttaskitem: to be done better with proper sizeHint()
        setPreferredSize(basicPreferredSize());
    } else {
        foreach (AbstractTaskItem * taskItem, m_groupMembers) {
            taskItem->setPreferredOnscreenSize();
        }

        layout()->invalidate();
        setPreferredSize(layout()->preferredSize());
        //kDebug() << "expanded group" << layout()->preferredSize();
    }

    //kDebug() << preferredSize();
    emit sizeHintChanged(Qt::PreferredSize);
}

AbstractTaskItem *TaskGroupItem::directMember(AbstractTaskItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(m_group);
    TaskManager::AbstractGroupableItem * directMember = m_group.data()->directMember(item->abstractItem());
    if (!directMember) {
        kDebug() << "Error" << item->abstractItem();
    }
    return abstractTaskItem(directMember);
}

void TaskGroupItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    if (collapsed()) {
        AbstractTaskItem::paint(painter, option, widget);
    }
}

void  TaskGroupItem::itemPositionChanged(AbstractGroupableItem * item)
{
    //kDebug();
    if (!m_tasksLayout) {
        return;
    }

    Q_ASSERT(item);

    AbstractTaskItem *taskItem = abstractTaskItem(item);

    m_tasksLayout->removeTaskItem(taskItem);

    // NOTE: If the grouping strategy is "only when the taskbar is full",
    //       removing the task from the layout might cause this group to
    //       split, and so the task might be removed from the group.
    //       This will cause deleteLater() to be called on taskItem, and
    //       we are in danger of inserting a pointer that will soon be
    //       invalid into the layout.  So check that the task item is
    //       still in the group.
    taskItem = abstractTaskItem(item);
    if (m_group && taskItem) {
        m_tasksLayout->insert(m_group.data()->members().indexOf(item), taskItem);
    }
}


void TaskGroupItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    bool shouldIgnore = shouldIgnoreDragEvent(event);
    if ((collapsed() && shouldIgnore) || (isRootGroup() && !shouldIgnore)) {
        event->ignore();
        //kDebug()<<"Drag enter accepted";
    } else {
        event->accept();
        if (!m_popupMenuTimer) {
            m_popupMenuTimer = new QTimer(this);
            m_popupMenuTimer->setSingleShot(true);
            m_popupMenuTimer->setInterval(500);
            connect(m_popupMenuTimer, SIGNAL(timeout()), this, SLOT(popupMenu()));
        }
        m_popupMenuTimer->start();
    }
}

void TaskGroupItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *)
{
    if (m_popupMenuTimer) {
        m_popupMenuTimer->stop();
    }

    if (m_dropIndicator && m_dropIndicator->isVisible()) {
        m_dropIndicator->setVisible(false);
    }
}

void TaskGroupItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (isRootGroup()) {
        //#ifndef ICON_TASKS_SHOW_DROP_INDICATOR_FOR_MOVE
        int sourceIndex = event->mimeData()->property("icontasks-item-ptr").isValid()
                          ? m_applet->rootGroupItem()->indexOf((AbstractTaskItem *)(event->mimeData()->property("icontasks-item-ptr").toULongLong()), false)
                          : -1;
        //#else
        //int sourceIndex = event->mimeData()->property("icontasks-item-index").isValid() ? event->mimeData()->property("icontasks-item-index").toInt() : -1;
        //#endif
        bool isDesktopFile = -1 == sourceIndex && event->mimeData()->hasFormat("text/uri-list");
        bool isLauncher = isDesktopFile || (-1 != sourceIndex && sourceIndex < m_applet->groupManager().launcherCount());

        if (isLauncher && m_applet->groupManager().launchersLocked()) {
            return;
        }

        if (sourceIndex > -1 || isDesktopFile) {
            TaskItemLayout::Insert destIndex = m_tasksLayout->insertionIndexAt(event->pos());
            bool visible = destIndex.index != sourceIndex && destIndex.index >= 0 && (destIndex.geom.x() > 0 || destIndex.geom.y() > 0 || 0 == destIndex.index);
            bool moveRight = sourceIndex > -1 && destIndex.index > sourceIndex;

//             qDebug() << event->pos() << sourceIndex << destIndex.index << destIndex.geom;
            if (visible && ((isLauncher && destIndex.index - (moveRight ? 1 : 0) < m_applet->groupManager().launcherCount()) ||
                            (!isLauncher && destIndex.index - (moveRight ? 1 : 0) >= m_applet->groupManager().launcherCount()))) {
                //#ifndef ICON_TASKS_SHOW_DROP_INDICATOR_FOR_MOVE
                if (!isDesktopFile) {
                    dropEvent(event);
                    return;
                }
                //#endif

                if (!m_dropIndicator) {
                    m_dropIndicator = new DropIndicator(parentItem());
                    m_dropIndicator->setOrientation(Plasma::Vertical == m_applet->formFactor() ? Qt::Vertical : Qt::Horizontal);
                    m_dropIndicator->setVisible(false);
                }

                if (!m_dropIndicator->isVisible()) {
                    m_dropIndicator->setSize((Plasma::Vertical == m_applet->formFactor() ? destIndex.geom.height() : destIndex.geom.width()) / 3.0);
                }

                m_dropIndicator->setPosition(QRectF(mapToParent(destIndex.geom.topLeft()), destIndex.geom.size()));
                return;
            }
        }

        if (m_dropIndicator && m_dropIndicator->isVisible()) {
            m_dropIndicator->setVisible(false);
        }
    }
}

AbstractTaskItem *TaskGroupItem::taskItemForWId(WId id)
{
    QHashIterator<AbstractGroupableItem *, AbstractTaskItem*> it(m_groupMembers);

    while (it.hasNext()) {
        it.next();
        AbstractTaskItem *item = it.value();
        TaskGroupItem *group = qobject_cast<TaskGroupItem*>(item);

        if (group) {
            item = group->taskItemForWId(id);
            if (item) {
                return item;
            }
        } else {
            TaskManager::TaskItem *task = qobject_cast<TaskManager::TaskItem*>(it.key());
            if (task && task->task() && task->task()->window() == id) {
                return item;
            }
        }
    }

    return 0;
}

static QString agiName(TaskManager::AbstractGroupableItem *i)
{
    if (i->itemType() == TaskManager::TaskItemType && !i->isStartupItem()) {
        return static_cast<TaskItem *>(i)->taskName().toLower();
    } else {
        return i->name().toLower();
    }
}

AbstractTaskItem *TaskGroupItem::matchingItem(TaskManager::AbstractGroupableItem *from)
{
    QHashIterator<AbstractGroupableItem *, AbstractTaskItem*> it(m_groupMembers);
    AbstractTaskItem *itm = 0L;
    QString n = agiName(from);
    KUrl launcherUrl = from->launcherUrl();

    while (it.hasNext()) {
        it.next();
        AbstractGroupableItem *item = it.key();
        AbstractTaskItem *taskItem = it.value();
        QString name = agiName(item);

        if ((name == n || (!launcherUrl.isEmpty() && item->launcherUrl() == launcherUrl)) &&
                (qobject_cast<TaskGroupItem*>(taskItem) || !taskItem->busyWidget())) {
            itm = taskItem;
        }
    }

    return itm;
}

void TaskGroupItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (m_dropIndicator && m_dropIndicator->isVisible()) {
        m_dropIndicator->setVisible(false);
    }

    //kDebug() << "TaskItemLayout dropEvent";
    if (event->mimeData()->hasFormat(TaskManager::Task::mimetype()) ||
            event->mimeData()->hasFormat(TaskManager::Task::groupMimetype())) {
        bool ok;
        QList<WId> ids = TaskManager::Task::idsFromMimeData(event->mimeData(), &ok);

        if (!ok) {
            //kDebug() << "FAIL!";
            event->ignore();
            return;
        }

        AbstractTaskItem *targetTask = dynamic_cast<AbstractTaskItem *>(scene()->itemAt(mapToScene(event->pos())));
        //  kDebug() << "Pos: " << event->pos() << mapToScene(event->pos()) << "item" << scene()->itemAt(mapToScene(event->pos())) << "target Task " << dynamic_cast<QGraphicsItem *>(targetTask);

        //kDebug() << "got" << ids.count() << "windows";
        foreach (WId id, ids) {
            handleDroppedId(id, targetTask, event);
        }

        //kDebug() << "TaskItemLayout dropEvent done";
        event->acceptProposedAction();
    } else if (!m_applet->groupManager().launchersLocked() && event->mimeData()->hasFormat("text/uri-list")) {
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
        foreach (const KUrl & url, urls) {
            const bool exists = m_applet->groupManager().launcherExists(url);
            if (exists) {
                // it exists; if we are doing manual sorting, make sure it is in the right location if
                // it is in this group .. otherwise, we can do nothing.
                if (m_applet->groupManager().sortingStrategy() == TaskManager::GroupManager::ManualSorting) {
                    QHashIterator<AbstractGroupableItem *, AbstractTaskItem*> it(m_groupMembers);
                    while (it.hasNext()) {
                        it.next();
                        if (it.key()->itemType() == TaskManager::LauncherItemType &&
                                it.key()->launcherUrl() == url) {
                            layoutTaskItem(it.value(), event->pos());
                            break;
                        }
                    }
                }
            } else {
                m_applet->groupManager().addLauncher(url, QIcon(), QString(), QString(), QString(), m_tasksLayout->insertionIndexAt(event->pos()).index);
            }
        }
    } else {
        event->ignore();
    }
}

void TaskGroupItem::handleDroppedId(WId id, AbstractTaskItem *, QGraphicsSceneDragDropEvent *event)
{
    AbstractTaskItem *taskItem = m_applet->rootGroupItem()->taskItemForWId(id);

    if (!taskItem) {
        //kDebug() << "Invalid TaskItem";
        return;
    }

    if (!taskItem->parentGroup()) {
        //kDebug() << "group invalid";
        return;
    }

    TaskManager::GroupPtr group = taskItem->parentGroup()->group();

    //kDebug() << id << taskItem->text() << (QObject*)targetTask;

    // kDebug() << "first item: " << dynamic_cast<QGraphicsItem*>(m_taskItems.first()) << "layout widget" << dynamic_cast<QGraphicsItem*>(this);
    if (m_applet->groupManager().sortingStrategy() == TaskManager::GroupManager::ManualSorting) {
        //Move action
        if (group == m_group.data()) {  //same group
            //kDebug() << "Drag within group";
            layoutTaskItem(taskItem, event->pos());
        } else if (m_group) {  //task item was dragged outside of group -> group move
            AbstractTaskItem *directMember = abstractTaskItem(m_group.data()->directMember(group));
            if (directMember) {
                layoutTaskItem(directMember, event->pos()); //we need to get the group right under the receiver group
            }
        }
    }
}

void TaskGroupItem::layoutTaskItem(AbstractTaskItem* item, const QPointF &pos)
{
    if (!m_tasksLayout || !item->abstractItem()) {
        return;
    }

    int insertIndex = m_tasksLayout->insertionIndexAt(pos).index;
    // kDebug() << "Item inserting at: " << insertIndex << "of: " << numberOfItems();
    m_applet->groupManager().manualSortingRequest(item->abstractItem(), insertIndex);
}


void TaskGroupItem::updateActive(AbstractTaskItem *task)
{
    if (!m_tasksLayout) {
        return;
    }

    m_activeTaskIndex = indexOf(task);
}

int TaskGroupItem::indexOf(AbstractTaskItem *task, bool descendGroups)
{
    if (!m_group || !task) {
        //kDebug() << "Error";
        return -1;
    }

    int index = 0;

    foreach (AbstractGroupableItem * item, m_group.data()->members()) {
        AbstractTaskItem *taskItem = abstractTaskItem(item);
        if (!taskItem) {
            continue;
        }

        if (task == taskItem) {
            if (descendGroups) {
                TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(taskItem);
                if (groupItem) {
                    int subIndex = groupItem->indexOf(groupItem->activeSubTask());
                    if (subIndex == -1) {
                        index += groupItem->count();
                    } else {
                        return index + subIndex;
                    }
                }
            }

            return index;
        }

        if (descendGroups) {
            TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(taskItem);
            if (groupItem) {
                int subIndex = groupItem->indexOf(task);
                if (subIndex == -1) {
                    index += groupItem->count();
                } else {
                    return index + subIndex;
                }
            } else {
                ++index;
            }
        } else {
            ++index;
        }
    }

    return -1;
}

AbstractTaskItem * TaskGroupItem::activeSubTask()
{
    if (!m_group) {
        return 0;
    }

    foreach (AbstractGroupableItem * item, m_group.data()->members()) {
        AbstractTaskItem *taskItem = abstractTaskItem(item);
        if (taskItem && taskItem->isActive()) {
            TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(taskItem);
            if (groupItem) {
                return groupItem->activeSubTask();
            }
            return taskItem;
        }
    }

    return 0;
}

int TaskGroupItem::totalSubTasks()
{
    int count = 0;

    foreach (AbstractGroupableItem * item, group()->members()) {
        AbstractTaskItem *taskItem = abstractTaskItem(item);
        if (taskItem) {
            TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(taskItem);
            if (groupItem) {
                count += groupItem->count();
            } else if (!qobject_cast<AppLauncherItem *>(taskItem)) {
                count++;
            }
        }
    }
    return count;
}

AbstractTaskItem * TaskGroupItem::selectSubTask(int index)
{
    foreach (AbstractGroupableItem * item, group()->members()) {
        AbstractTaskItem *taskItem = abstractTaskItem(item);
        if (taskItem) {
            TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(taskItem);
            if (groupItem) {
                if (index < groupItem->count()) {
                    return groupItem->abstractTaskItem(groupItem->group()->members().at(index));
                } else {
                    index -= groupItem->count();
                }
            } else if (qobject_cast<AppLauncherItem *>(taskItem)) {
                continue;
            } else if (index == 0) {
                return taskItem;
            } else {
                --index;
            }
        }
    }
    return NULL;
}

void TaskGroupItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    focusSubTask((event->delta() < 0), true);
}

int TaskGroupItem::maxRows()
{
    return m_maximumRows;
}

void TaskGroupItem::setMaxRows(int rows)
{
    m_maximumRows = rows;
    if (m_tasksLayout) {
        m_tasksLayout->setMaximumRows(m_maximumRows);
    }
}

int TaskGroupItem::optimumCapacity()
{
    if (m_tasksLayout) {
        return m_tasksLayout->maximumRows() * m_tasksLayout->preferredColumns();
    }

    return 1;
}

AbstractTaskItem* TaskGroupItem::abstractTaskItem(AbstractGroupableItem * item)
{
    if (!item) {
        return 0;
    }

    AbstractTaskItem *abstractTaskItem = m_groupMembers.value(item);
    if (!abstractTaskItem) {
        foreach (AbstractTaskItem * taskItem, m_groupMembers) {
            TaskGroupItem *group = qobject_cast<TaskGroupItem*>(taskItem);
            if (group) {
                abstractTaskItem = group->abstractTaskItem(item);
                if (abstractTaskItem) {
                    break;
                }
            }
        }
    }

    //kDebug() << "item not found";
    return abstractTaskItem;
}

void TaskGroupItem::setAdditionalMimeData(QMimeData* mimeData)
{
    if (m_group) {
        m_group.data()->addMimeData(mimeData);
    }
}

void TaskGroupItem::publishIconGeometry() const
{
    // only do this if we are a collapsed group, with a GroupPtr and members
    if (!collapsed() || !m_group || m_groupMembers.isEmpty()) {
        return;
    }

    QRect rect = iconGeometry();
    publishIconGeometry(rect);
}

void TaskGroupItem::publishIconGeometry(const QRect &rect) const
{
    foreach (AbstractTaskItem * item, m_groupMembers) {
        WindowTaskItem *windowItem = qobject_cast<WindowTaskItem *>(item);
        if (windowItem) {
            windowItem->publishIconGeometry(rect);
            continue;
        }

        TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(item);
        if (groupItem) {
            groupItem->publishIconGeometry(rect);
        }
    }
}

QWidget *TaskGroupItem::popupDialog() const
{
    return m_popupDialog;
}

#include "taskgroupitem.moc"

