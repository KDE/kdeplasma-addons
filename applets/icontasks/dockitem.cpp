/*
 * Icon Task Manager
 *
 * Copyright 2011 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "dockitem.h"
#include "dockitemadaptor.h"
#include "dockmanager.h"
#include "abstracttaskitem.h"
#include <KDE/KDesktopFile>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtCore/QFile>
#include <QtCore/QTimer>

static qulonglong itemCount = 0;

static QIcon getIcon(const QString &file, const QString &name=QString())
{
    if (!file.isEmpty()) {
        if (QFile::exists(file)) {
            return QIcon(file);
        } else if (QIcon::hasThemeIcon(file)) {
            return QIcon::fromTheme(file);
        }
    }

    if (!name.isEmpty() && QIcon::hasThemeIcon(name)) {
        return QIcon::fromTheme(name);
    }

    return QIcon();
}

DockItem::DockItem(const KUrl &desktopFile)
    : m_url(desktopFile)
    , m_timer(0)
    , m_progress(-1)
    , m_menuIdCount(0)
{
    new DockItemAdaptor(this);
    m_path = QLatin1String("/net/launchpad/DockManager/Item") + QString().setNum(itemCount++);
    QDBusConnection::sessionBus().registerObject(m_path, this);
}

DockItem::~DockItem()
{
    foreach (AbstractTaskItem * i, m_tasks) {
        i->setDockItem(0);
    }

    QDBusConnection::sessionBus().unregisterObject(m_path, QDBusConnection::UnregisterTree);
}

QString DockItem::DesktopFile() const
{
    return m_url.toLocalFile();
}

QString DockItem::Uri() const
{
    return m_url.url();
}

QString DockItem::name() const
{
    if (m_name.isEmpty() && m_url.isLocalFile() && KDesktopFile::isDesktopFile(m_url.toLocalFile())) {
        m_name = KDesktopFile(m_url.toLocalFile()).readName();
    }

    return m_name;
}

QList<QAction *> DockItem::menu() const
{
    QList<QAction *> acts;
    QSet<QString> insertedMenus;

    foreach (QAction * act, m_menu.values()) {
        QString title = act->property("container-title").toString();
        if (!title.isEmpty() && m_actionMenus.contains(title)) {
            if (!insertedMenus.contains(title)) {
                insertedMenus.insert(title);
                acts.append(m_actionMenus[title]->menuAction());
            }
        } else {
            acts.append(act);
        }
    }
    return acts;
}

unsigned int DockItem::AddMenuItem(QMap<QString, QVariant> hints)
{
    if (calledFromDBus()) {
        DockManager::self()->itemService(this, message().service());
    }

    QString label,
            iconName,
            iconFile,
            container;

    /*if (hints.contains("uri")) {
    } else*/ {
        label = hints["label"].toString();
        iconName = hints["icon-name"].toString();
        iconFile = hints["icon-file"].toString();
        container = hints["container-title"].toString();
    }

    unsigned int id = m_menuIdCount++;
    QIcon icon = getIcon(iconFile, iconName);
    QAction *action = icon.isNull()
                      ? new QAction(label, this)
                      : new QAction(icon, label, this);
    connect(action, SIGNAL(triggered()), this, SLOT(menuActivated()));
    action->setData(id);
    action->setProperty("container-title", container);
    if (!m_actionMenus.contains(container)) {
        m_actionMenus.insert(container, new QMenu(container, 0));
    }
    m_actionMenus[container]->addAction(action);
    m_menu.insert(id, action);
    return id;
}

void DockItem::RemoveMenuItem(unsigned int id)
{
    if (calledFromDBus()) {
        DockManager::self()->itemService(this, message().service());
    }

    if (m_menu.contains(id)) {
        QAction *act = m_menu[id];
        QString title = act->property("container-title").toString();
        if (!title.isEmpty() && m_actionMenus.contains(title)) {
            m_actionMenus[title]->removeAction(act);
            if (m_actionMenus[title]->actions().isEmpty()) {
                m_actionMenus[title]->deleteLater();
                m_actionMenus.remove(title);
            }
        }
        disconnect(act, SIGNAL(triggered()), this, SLOT(menuActivated()));
        m_menu.remove(id);
    }
}

void DockItem::menuActivated()
{
    QObject *s = sender();
    if (s && qobject_cast<QAction *>(s)) {
        QAction *item = static_cast<QAction *>(s);
        emit MenuItemActivated(item->data().toUInt());
    }
}

void DockItem::UpdateDockItem(QMap<QString, QVariant> hints)
{
    if (calledFromDBus()) {
        DockManager::self()->itemService(this, message().service());
    }

    QMap<QString, QVariant>::ConstIterator it(hints.constBegin()),
                                           end(hints.constEnd());
    int                                    updated = 0;

    for (; it != end; ++it) {
        if (it.key() == "badge") {
            QString badge = it.value().toString();
            if (badge != m_badge) {
                m_badge = badge;
                updated++;
            }
        } else if (it.key() == "progress") {
            int prog = it.value().toInt();
            if (prog != m_progress) {
                m_progress = prog;
                updated++;
            }
        } else if (it.key() == "icon-file") {
            m_icon = getIcon(it.value().toString());
            updated++;
        } else if (it.key() == "x-kde-overlay") {
            m_overlayIcon = getIcon(it.value().toString());
            updated++;
        }
    }

    if (updated) {
        foreach (AbstractTaskItem * i, m_tasks) {
            i->dockItemUpdated();
        }
    }
}

void DockItem::registerTask(AbstractTaskItem *item)
{
    m_tasks.insert(item);
    item->setDockItem(this);
    if (!m_badge.isEmpty() || !m_icon.isNull() || !m_overlayIcon.isNull() || (m_progress >= 0 && m_progress <= 100)) {
        item->dockItemUpdated();
    }
    if (m_timer) {
        m_timer->stop();
    }
}

void DockItem::unregisterTask(AbstractTaskItem *item)
{
    m_tasks.remove(item);

    if (0 == m_tasks.count()) {
        // No current tasks, so set off timer. If nothing registers then we are no longer used...
        if (!m_timer) {
            m_timer = new QTimer(this);
            connect(m_timer, SIGNAL(timeout()), this, SLOT(check()));
        }
        m_timer->start(500);
    }
}

void DockItem::reset()
{
    bool updated = !m_badge.isEmpty() || !m_icon.isNull() || !m_overlayIcon.isNull() || (m_progress >= 0 && m_progress <= 100);

    m_badge = QString();
    m_icon = QIcon();
    m_progress = -1;

    foreach (QAction * mnu, m_menu.values()) {
        mnu->deleteLater();
    }

    m_menu.clear();

    if (updated) {
        foreach (AbstractTaskItem * i, m_tasks) {
            i->dockItemUpdated();
        }
    }
}

void DockItem::check()
{
    if (0 == m_tasks.count()) {
        DockManager::self()->remove(this);
    }
}

#include "dockitem.moc"
