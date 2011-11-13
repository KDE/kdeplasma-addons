/***************************************************************************
 *   Copyright (C) 2010 by Anton Kreuzkamp <akreuzkamp@web.de>             *
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
#include "applauncheritem.h"
#include "taskgroupitem.h"
#include "jobmanager.h"
#include "dockmanager.h"
#include "dockitem.h"
#include "mediabuttons.h"
#include "unity.h"

#include "taskmanager/taskactions.h"
#include "taskmanager/groupmanager.h"

// Qt
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>

// KDE
#include <KAuthorized>
#include <KIconEffect>

#include "tooltips/tooltipmanager.h"
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/PaintUtils>

QString AppLauncherItem::mimetype()
{
    return "taskmanager:/launcher";
}

AppLauncherItem::AppLauncherItem(QGraphicsWidget* parent, Tasks* applet, TaskManager::LauncherItem* launcher)
    : AbstractTaskItem(parent, applet)
{
    m_launcher = launcher;
    m_abstractItem = launcher;
    registerWithHelpers();
}

AppLauncherItem::~AppLauncherItem()
{
    close(false);
}

void AppLauncherItem::close()
{
    close(true);
}

void AppLauncherItem::close(bool hide)
{
    unregisterFromHelpers();
    if (hide) {
        setVisible(false);
    }
}

QString AppLauncherItem::windowClass() const
{
    return m_applet->groupManager().launcherWmClass(launcherUrl());
}

void AppLauncherItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->button() == Qt::LeftButton || (event->button() == Qt::MidButton && Tasks::MC_NewInstance == m_applet->middleClick())) &&
            boundingRect().contains(event->pos())) {
        m_launcher->launch();
    }
}

void AppLauncherItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    if (!KAuthorized::authorizeKAction("kwin_rmb") || !m_launcher) {
        QGraphicsWidget::contextMenuEvent(e);
        return;
    }

    QList <QAction*> actionList;

    QAction *configAction = m_applet->action("configure");
    if (configAction && configAction->isEnabled()) {
        actionList.append(configAction);
    }

    TaskManager::BasicMenu menu(0, m_launcher, &m_applet->groupManager(), actionList, getAppMenu());
    menu.adjustSize();

    if (m_applet->formFactor() != Plasma::Vertical) {
        menu.setMinimumWidth(size().width());
    }

    Q_ASSERT(m_applet->containment());
    Q_ASSERT(m_applet->containment()->corona());
    stopWindowHoverEffect();
    menu.exec(m_applet->containment()->corona()->popupPosition(this, menu.size()));
}


void AppLauncherItem::updateToolTip()
{
    IconTasks::ToolTipContent data(m_launcher->name(), m_launcher->genericName(), m_launcher->icon());
#if KDE_IS_VERSION(4, 7, 0)
    data.setInstantPopup(m_applet->instantToolTip());
#endif
    QString key = mediaButtonKey();
    if (!key.isEmpty()) {
        data.setPlayState(MediaButtons::self()->playbackStatus(key));
        data.setClickable(true);
    }

    IconTasks::ToolTipManager::self()->setContent(this, data);
}

void AppLauncherItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        m_launcher->launch();
    } else {
        QGraphicsWidget::keyPressEvent(event);
    }
}

void AppLauncherItem::setAdditionalMimeData(QMimeData* mimeData)
{
    if (m_launcher) {
        m_launcher->addMimeData(mimeData);

        // Add our own mimetype, so that AbstractTaskItem knows to ignore drag envets of this type,
        // then taskgroup will receive the event, and launchers can be re-ordered!!!
        QByteArray data;
        mimeData->setData(mimetype(), data);
    }
}

#include "applauncheritem.moc"

