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

#include "unityitem.h"
#include "unity.h"
#include "abstracttaskitem.h"
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <KDE/KDesktopFile>
#include <KDE/KRun>
#include <KDE/KIcon>
#include <dbusmenuimporter.h>

UnityItem::UnityItem(const QString &i, const QString &desktopFile)
    : m_id(i)
    , m_desktopFile(KDesktopFile::isDesktopFile(desktopFile) ? desktopFile : QString())
    , m_staticDirty(true)
    , m_progressVisible(false)
    , m_countVisible(false)
    , m_count(0)
    , m_progress(0)
    , m_timer(0)
    , m_separator(0)
    , m_dbusMenu(0)
{
}

UnityItem::~UnityItem()
{
    foreach (AbstractTaskItem * i, m_tasks) {
        i->setUnityItem(0);
    }
}

QList<QAction *> UnityItem::menu()
{
    readStaticMenu();

    QList<QAction *> mnu = m_staticMenu;

    if (m_dbusMenu) {
        m_dbusMenu->updateMenu();

        if (m_dbusMenu->menu()) {
            QList<QAction *> dbusMnu = m_dbusMenu->menu()->actions();

            if (!mnu.isEmpty() && !dbusMnu.isEmpty()) {
                if (!m_separator) {
                    m_separator = new QAction("Separator", this);
                    m_separator->setSeparator(true);
                }
                mnu.append(m_separator);
            }
            mnu.append(dbusMnu);
        }
    }

    return mnu;
}

void UnityItem::reset()
{
    bool updated = m_countVisible || m_progressVisible;
    m_countVisible = m_progressVisible = false;
    m_progress = m_count = 0;
    m_dbusMenu->deleteLater();
    m_dbusMenu = 0;
    if (updated) {
        foreach (AbstractTaskItem * task, m_tasks) {
            task->unityItemUpdated();
        }
    }
}

void UnityItem::updateStaticMenu()
{
    m_staticDirty = true;
}

void UnityItem::update(QMap<QString, QVariant> properties, const QString &sender)
{
    bool updated = false;

    if (properties.contains("count")) {
        unsigned int count = properties["count"].toUInt();
        if (count != m_count) {
            m_count = count;
            updated = true;
        }
    }
    if (properties.contains("count-visible")) {
        bool countVisible = properties["count-visible"].toBool();
        if (countVisible != m_countVisible) {
            m_countVisible = countVisible;
            updated = true;
        }
    }
    if (properties.contains("progress")) {
        int progress = (int)((properties["progress"].toDouble() * 100.0) + 0.5);
        if (progress != m_progress) {
            m_progress = progress;
            updated = true;
        }
    }
    if (properties.contains("progress-visible")) {
        bool progressVisible = properties["progress-visible"].toBool();
        if (progressVisible != m_progressVisible) {
            m_progressVisible = progressVisible;
            updated = true;
        }
    }
    if (properties.contains("quicklist")) {
        QString quicklist = properties["quicklist"].toString();
        if (quicklist != m_dbusMenuPath || sender != m_dbusMenuService) {
            if (quicklist.isEmpty() && !m_dbusMenuPath.isEmpty()) {
                m_dbusMenu->deleteLater();
                m_dbusMenu = 0;
            } else if (!quicklist.isEmpty()) {
                if (!m_dbusMenuPath.isEmpty()) {
                    m_dbusMenu->deleteLater();
                    m_dbusMenu = 0;
                }
                m_dbusMenuPath = quicklist;
                m_dbusMenu = new DBusMenuImporter(sender, quicklist, this);
            }
        }
        m_dbusMenuService = sender;
    }

    if (updated) {
        foreach (AbstractTaskItem * task, m_tasks) {
            task->unityItemUpdated();
        }
    }
}

void UnityItem::registerTask(AbstractTaskItem *item)
{
    m_tasks.insert(item);
    item->setUnityItem(this);

    if (m_progressVisible) {
        item->unityItemUpdated();
    }

    if (m_timer) {
        m_timer->stop();
    }
}

void UnityItem::unregisterTask(AbstractTaskItem *item)
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

void UnityItem::check()
{
    if (0 == m_tasks.count()) {
        Unity::self()->remove(this);
    }
}

void UnityItem::menuActivated()
{
    QObject *s = sender();
    if (s && qobject_cast<QAction *>(s)) {
        QAction *item = static_cast<QAction *>(s);
        QString exec = item->data().toString();

        if (!exec.isEmpty()) {
            KRun::run(exec, KUrl::List(), 0, QString(), QString(), "0");
        }
    }
}

void UnityItem::readStaticMenu()
{
    if (m_staticDirty) {
        if (!m_desktopFile.isEmpty()) {
            foreach (QAction * act, m_staticMenu) {
                delete act;
            }
            m_staticMenu.clear();

            KDesktopFile df(m_desktopFile);
            KConfigGroup de(&df, "Desktop Entry");

            QStringList shortCutsFdo = de.readEntry("Actions", QString()).split(';');

            foreach (QString shortcut, shortCutsFdo) {
                parseDesktopAction(KConfigGroup(&df, "Desktop Action " + shortcut));
            }

            QStringList shortCutsAyatana = de.readEntry("X-Ayatana-Desktop-Shortcuts", QString()).split(';');

            foreach (QString shortcut, shortCutsAyatana) {
                parseDesktopAction(KConfigGroup(&df, shortcut + " Shortcut Group"));
            }
        }
        m_staticDirty = false;
    }
}

void UnityItem::parseDesktopAction(const KConfigGroup& grp)
{
    QString name = grp.readEntry("Name", QString());
    QString exec = grp.readEntry("Exec", QString());

    if (!name.isEmpty() && !exec.isEmpty()) {
        QString icon = grp.readEntry("Icon", QString()); // ???
        QAction *action = icon.isEmpty() ? new QAction(name, this) : new QAction(KIcon(icon), name, this);
        action->setData(exec);
        m_staticMenu.append(action);
        connect(action, SIGNAL(triggered()), this, SLOT(menuActivated()));
    }
}

#include "unityitem.moc"
