/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Applications.h"
#include "FavoriteApplications.h"

#include <kstandarddirs.h>
#include <kservicegroup.h>

#include <KRun>
#include <KUrl>
#include <KDebug>
#include <KIcon>
// Applications

namespace Models {

Applications::Applications(QString root, QString title, QIcon icon):
    m_root(root), m_title(title), m_icon(icon), m_loaded(false)
{
    load();
}

Applications::~Applications()
{
    clear();
}

void Applications::clear()
{
    foreach(Applications * applist, m_submodels) {
        delete applist;
    }
}

void Applications::load()
{
    if (m_loaded) clear();
    m_loaded = true;

    KServiceGroup::Ptr root = KServiceGroup::group(m_root);
    if (!root || !root->isValid())
        return;

    KServiceGroup::List list = root->entries();

    // application name <-> service map for detecting duplicate entries
    QHash<QString,KService::Ptr> existingServices;
    for (KServiceGroup::List::ConstIterator it = list.begin(); it != list.end(); ++it) {
        ApplicationData data;

        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KService)) {
            const KService::Ptr service = KService::Ptr::staticCast(p);

            if (service->noDisplay())
                continue;

            data.icon = KIcon(service->icon());
            data.name = service->name();
            data.description = service->genericName();
            data.desktopFile = service->entryPath();
            kDebug() << data.desktopFile;

            m_items.append(data);
        } else if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr serviceGroup =
                    KServiceGroup::Ptr::staticCast(p);

            if (serviceGroup->noDisplay() || serviceGroup->childCount() == 0)
                continue;

            m_submodels.append(new Applications(
                serviceGroup->relPath(),
                serviceGroup->caption(),
                KIcon(serviceGroup->icon())
            ));

            // appName = serviceGroup->comment();
        }
    }
}

QString Applications::title(int index) const
{
    if (index >= size()) return "";
    return
        (index < m_submodels.size()) ?
            m_submodels.at(index)->modelTitle() :
            m_items.at(index - m_submodels.size()).name;
}

QString Applications::description(int index) const
{
    if (index >= size()) return "";
    if (index < m_submodels.size()) return "";
    return m_items.at(index - m_submodels.size()).description;
}

QIcon Applications::icon(int index) const
{
    if (index >= size()) return QIcon();
    return
        (index < m_submodels.size()) ?
            m_submodels.at(index)->modelIcon() :
            m_items.at(index - m_submodels.size()).icon;
}

bool Applications::isCategory(int index) const
{
    //Q_UNUSED(index);
    //return false;
    return (index < m_submodels.size());
}

int Applications::size() const
{
    return m_submodels.size() + m_items.size();
}

void Applications::activate(int index)
{
    kDebug() << " activated ";

    if (index >= size()) return;
    if (index < m_submodels.size()) return;
    kDebug() << " activated passed ";

    new KRun(KUrl(m_items.at(index - m_submodels.size()).desktopFile), 0);
    LancelotApplication::hide(true);
}

Lancelot::PassagewayViewModel * Applications::child(int index)
{
    if (index >= m_submodels.size())
        return NULL;
    return m_submodels.at(index);
}

QString Applications::modelTitle() const
{
    return m_title;
}

QIcon Applications::modelIcon() const
{
    return m_icon;
}

bool Applications::hasContextActions(int index) const
{
    return !isCategory(index);
}

void Applications::setContextActions(int index, QMenu * menu)
{
    if (isCategory(index)) {
        return;
    }

    menu->addAction(KIcon("list-add"), i18n("Add to favorites"))
        ->setData(QVariant(0));
}

void Applications::contextActivate(int index, QAction * context)
{
    if (!context) {
        return;
    }

    int appIndex = index - m_submodels.size();
    kDebug() << appIndex << m_items.size();
    if (context->data().toInt() == 0) {
        FavoriteApplications::instance()
            ->addFavorite(m_items.at(appIndex).desktopFile);
    }
}

} // namespace Models
