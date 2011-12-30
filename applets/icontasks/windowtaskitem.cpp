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
#include "windowtaskitem.h"
#include "taskgroupitem.h"
#include "taskitemlayout.h"
#include "jobmanager.h"
#include "dockmanager.h"
#include "dockitem.h"
#include "mediabuttons.h"
#include "unity.h"

// Qt
#include <QGraphicsSceneContextMenuEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QVarLengthArray>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KIcon>
#include <KIconLoader>

#include "taskmanager/taskactions.h"
#include "taskmanager/task.h"
#include "taskmanager/taskmanager.h"
#include "taskmanager/taskgroup.h"

#include <Plasma/Theme>
#include <Plasma/FrameSvg>
#include "tooltips/tooltipmanager.h"
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/BusyWidget>

#include "tasks.h"

#ifdef Q_WS_X11

#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

// Busy widget is placed over taskbar icon - therefore need to ignore mouse press/release events!
class WindowTaskItem::BusyWidget : public Plasma::BusyWidget
{
public:

    BusyWidget(QGraphicsWidget *p) : Plasma::BusyWidget(p) { };
    virtual ~BusyWidget() { }

    void mousePressEvent(QGraphicsSceneMouseEvent *event) {
        event->ignore();
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
        event->ignore();
    }
};

WindowTaskItem::WindowTaskItem(QGraphicsWidget *parent, Tasks *applet)
    : AbstractTaskItem(parent, applet),
      m_busyWidget(0)
{
}

WindowTaskItem::~WindowTaskItem()
{
    close(false);
}

void WindowTaskItem::activate()
{
    // the Task class has a method called activateRaiseOrIconify() which
    // should perform the required action here.
    //
    // however it currently does not minimize the task's window if the item
    // is clicked whilst the window is active probably because the active window by
    // the time the mouse is released over the window task button is not the
    // task's window but instead the desktop window
    //
    // TODO: the Kicker panel in KDE 3.x has a feature whereby clicking on it
    // does not take away the focus from the active window (unless clicking
    // in a widget such as a line edit which does accept the focus)
    // this needs to be implemented for Plasma's own panels.
    //kDebug();
    if (m_task && m_task.data()->task()) {
        m_task.data()->task()->activateRaiseOrIconify();
        // emit windowSelected(this);
    }
}

QString WindowTaskItem::appName() const
{
    return m_task ? m_task.data()->taskName() : QString();
}

KUrl WindowTaskItem::launcherUrl() const
{
    return m_task ? m_task.data()->launcherUrl() : KUrl();
}

QString WindowTaskItem::windowClass() const
{
    return m_task && m_task.data()->task() ? m_task.data()->task()->classClass() : QString();
}

void WindowTaskItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::MidButton) {
        if (isGrouped() && parentGroup()) {
            parentGroup()->collapse();
        }
    } else {
        AbstractTaskItem::mousePressEvent(event);
    }

    event->accept();
}

void WindowTaskItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        activate();
    } else {
        QGraphicsWidget::keyPressEvent(event);
    }
}

//destroy this item
void WindowTaskItem::close()
{
    close(true);
}

void WindowTaskItem::close(bool hide)
{
    //kDebug();
    delete m_busyWidget;
    m_busyWidget = 0;
    unregisterFromHelpers();
    if (hide) {
        setVisible(false);
    }
}

void WindowTaskItem::publishIconGeometry() const
{
    if (!m_task || !m_task.data()->task()) {
        return;
    }

    QRect rect = iconGeometry();
    if (QRect(0, 0, 0, 0) != rect) {
        m_task.data()->task()->publishIconGeometry(rect);
    }
}

void WindowTaskItem::publishIconGeometry(const QRect &rect) const
{
    if (m_task && m_task.data()->task() && QRect(0, 0, 0, 0) != rect) {
        m_task.data()->task()->publishIconGeometry(rect);
    }
}

void WindowTaskItem::updateTask(::TaskManager::TaskChanges changes)
{
    if (!m_task) {
        return;
    }

    bool needsUpdate = false;
    TaskFlags flags = m_flags;

    if (changes & TaskManager::StateChanged) {
        if (m_task.data()->isActive()) {
            flags |= TaskHasFocus;
            if (!(m_flags & TaskHasFocus)) {
                emit activated(this);
                // We have focus now, so remove any attention state...
                if (m_task.data()->demandsAttention()) {
                    KWindowSystem::demandAttention(m_task.data()->task()->window(), false);
                }
            }
        } else {
            flags &= ~TaskHasFocus;
        }

        if (m_task.data()->isMinimized()) {
            flags |= TaskIsMinimized;
        } else {
            flags &= ~TaskIsMinimized;
        }

    }

    if (changes & TaskManager::AttentionChanged) {
        if (m_task.data()->demandsAttention()) {
            flags |= TaskWantsAttention;
        } else {
            flags &= ~TaskWantsAttention;
        }
    }

    if (m_flags != flags) {
        needsUpdate = true;
        setTaskFlags(flags);
    }

    // basic title and icon
    if (changes & TaskManager::IconChanged) {
        needsUpdate = true;
    }

    if (changes & TaskManager::NameChanged) {
        needsUpdate = true;
    }

    if (IconTasks::ToolTipManager::self()->isVisible(this) &&
            (changes & TaskManager::IconChanged ||
             changes & TaskManager::NameChanged ||
             changes & TaskManager::DesktopChanged)) {
        updateToolTip();
    }

    if (needsUpdate) {
        //redraw
        //kDebug() << m_task.data()->name();
        queueUpdate();
    }
}

void WindowTaskItem::updateToolTip()
{
    if (!m_task || !m_task.data()->task()) {
        return;
    }

    bool showToolTip = true;
    TaskGroupItem *group = parentGroup();

    if (group) {
        QWidget *groupPopupDialog = parentGroup()->popupDialog();
        QWidget *dialog = m_applet->popupDialog();

        if (dialog && dialog->isVisible()) {
            if (groupPopupDialog && groupPopupDialog == dialog) {
                showToolTip = true;
            } else {
                showToolTip = false;
            }
        }
    }

    if (showToolTip) {
        IconTasks::ToolTipContent data;
        data.setMainText(m_task.data()->name());
        data.setWindowDetailsToPreview(QList<IconTasks::ToolTipContent::Window>()
                                           << IconTasks::ToolTipContent::Window(m_task.data()->task()->window(),
                                       m_task.data()->name(),
                                       icon().pixmap(IconTasks::ToolTipContent::iconSize(), IconTasks::ToolTipContent::iconSize()),
                                       m_task.data()->task()->demandsAttention(),
                                       !m_applet->groupManager().showOnlyCurrentDesktop() || !m_task.data()->isOnCurrentDesktop()
                                           ? m_task.data()->desktop() : 0));
        data.setClickable(true);
#if KDE_IS_VERSION(4, 7, 0)
        data.setInstantPopup(m_applet->instantToolTip());
#endif
        data.setHighlightWindows(m_applet->highlightWindows());
        data.setVertical(Plasma::Vertical == m_applet->formFactor());

        QString key = mediaButtonKey();
        if (!key.isEmpty()) {
            data.setPlayState(MediaButtons::self()->playbackStatus(key, pid()));
        }

        if (group && group->collapsed()) {
            data.setGraphicsWidget(parentWidget());
        }

        IconTasks::ToolTipManager::self()->setContent(this, data);
    } else {
        clearToolTip();
    }
}

void WindowTaskItem::setStartupTask(TaskItem *task)
{
    //kDebug();
    if (!task->startup()) {
        kDebug() << "Error";
        return;
    }

    m_abstractItem = task;

    if (m_abstractItem) {
        connect(m_abstractItem, SIGNAL(destroyed(QObject*)), this, SLOT(clearAbstractItem()));
        connect(task, SIGNAL(gotTaskPointer()), this, SLOT(gotTaskPointer()));

        if (!m_busyWidget) {
            m_busyWidget = new BusyWidget(parentGroup());
            m_busyWidget->hide();
        }
    }
}

void WindowTaskItem::gotTaskPointer()
{
    //kDebug();
    TaskManager::TaskItem *item = qobject_cast<TaskManager::TaskItem*>(sender());
    if (item) {
        bool addToLayout = 0 != m_busyWidget;
        delete m_busyWidget;
        m_busyWidget = 0;

        setWindowTask(item);

        // If we were a busy widget, then we will not have been added to layout. To ensure we are,
        // manually add now...
        TaskGroupItem *pg = 0;
        if (addToLayout && (pg = parentGroup()) && pg->isRootGroup() && pg->tasksLayout()) {
            pg->tasksLayout()->addTaskItem(this);
        }
    }
}

void WindowTaskItem::setWindowTask(TaskManager::TaskItem* taskItem)
{
    if (m_task && m_task.data()->task()) {
        disconnect(m_task.data()->task(), 0, this, 0);
    }
    m_task = taskItem;
    m_abstractItem = taskItem;

    if (m_abstractItem) {
        connect(m_abstractItem, SIGNAL(destroyed(QObject*)), this, SLOT(clearAbstractItem()));
    }

    if (m_task) {
        connect(m_task.data(), SIGNAL(changed(::TaskManager::TaskChanges)),
                this, SLOT(updateTask(::TaskManager::TaskChanges)));
    }

    updateTask(::TaskManager::EverythingChanged);
    publishIconGeometry();
    registerWithHelpers();
    //kDebug() << "Task added, isActive = " << task->isActive();
}

void WindowTaskItem::setTask(TaskManager::TaskItem* taskItem)
{
    if (!taskItem->startup() && !taskItem->task()) {
        kDebug() << "Error";
        return;
    }

    if (!taskItem->task()) {
        setStartupTask(taskItem);
    } else {
        setWindowTask(taskItem);
    }
}

TaskManager::Task *WindowTaskItem::windowTask() const
{
    return m_task ? m_task.data()->task() : 0;
}

void WindowTaskItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    if (!KAuthorized::authorizeKAction("kwin_rmb") || !m_task) {
        QGraphicsWidget::contextMenuEvent(e);
        return;
    }

    showContextMenu(QPoint(), true);
}

void WindowTaskItem::showContextMenu(const QPoint &pos, bool showAppMenu)
{
    QList <QAction*> actionList;
    QAction *a(0);

    QAction *configAction = m_applet->action("configure");
    if (configAction && configAction->isEnabled()) {
        actionList.append(configAction);
    }

    TaskManager::BasicMenu menu(0, m_task.data(), &m_applet->groupManager(), actionList, showAppMenu ? getAppMenu() : QList <QAction*>());
    menu.adjustSize();

    if (m_applet->formFactor() != Plasma::Vertical) {
        menu.setMinimumWidth(size().width());
    }

    Q_ASSERT(m_applet->containment());
    Q_ASSERT(m_applet->containment()->corona());
    stopWindowHoverEffect();
    menu.exec(pos.isNull() ? m_applet->containment()->corona()->popupPosition(this, menu.size()) : pos);
    delete a;
}

int WindowTaskItem::pid() const
{
    return m_task && m_task.data()->task() ? m_task.data()->task()->pid() : 0;
}

void WindowTaskItem::toCurrentDesktop()
{
    if (m_task && m_task.data()->task()) {
        m_task.data()->task()->toCurrentDesktop();
    }
}

bool WindowTaskItem::isWindowItem() const
{
    return true;
}

bool WindowTaskItem::isActive() const
{
    return m_task ? m_task.data()->isActive() : false;
}

void WindowTaskItem::setAdditionalMimeData(QMimeData* mimeData)
{
    if (m_task) {
        m_task.data()->addMimeData(mimeData);
    }
}

QGraphicsWidget *WindowTaskItem::busyWidget() const
{
    return m_busyWidget;
}

#include "windowtaskitem.moc"

