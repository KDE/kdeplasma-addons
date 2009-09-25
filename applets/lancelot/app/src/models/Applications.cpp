/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
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
#include <KIcon>
#include <KSycoca>

#include "logger/Logger.h"
// Applications

namespace Models {

Applications::Applications(QString root, QString title, QIcon icon, bool flat):
    m_root(root), m_title(title), m_icon(icon), m_flat(flat)
{
    connect(KSycoca::self(), SIGNAL(databaseChanged(const QStringList &)),
            this, SLOT(sycocaUpdated(const QStringList &)));
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
    KServiceGroup::Ptr root = KServiceGroup::group(m_root);
    if (!root || !root->isValid())
        return;

    if (m_title == QString() || m_icon.isNull()) {
        m_title = root->caption();
        m_icon = KIcon(root->icon());
    }

    // KServiceGroup::List list = root->entries();
    const KServiceGroup::List list =
            root->entries(true  /* sorted */,
                          true  /* exclude no display entries */,
                          false /* allow separators */,
                          false /* sort by generic name */);
    m_items.clear();
    QList < Applications * > submodelsOld = m_submodels;
    m_submodels.clear();

    // application name <-> service map for detecting duplicate entries
    QHash<QString,KService::Ptr> existingServices;
    for (KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        ApplicationData data;

        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KService)) {
            const KService::Ptr service = KService::Ptr::staticCast(p);

            if (service->noDisplay())
                continue;

            data.icon = KIcon(service->icon());
            data.name = service->name().replace("&", "&&");
            data.description = service->genericName();
            data.desktopFile = service->entryPath();

            m_items.append(data);
        } else if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr serviceGroup =
                    KServiceGroup::Ptr::staticCast(p);

            if (serviceGroup->noDisplay() || serviceGroup->childCount() == 0)
                continue;

            bool found = false;
            Applications * model;
            foreach (model, submodelsOld) {
                if (serviceGroup->relPath() == model->m_root) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                m_submodels.append(new Applications(
                    serviceGroup->relPath(),
                    serviceGroup->caption().replace("&", "&&"),
                    KIcon(serviceGroup->icon())
                ));
            } else {
                submodelsOld.removeAll(model);
                m_submodels.append(model);
                model->load();
                // TODO: Find a way to delete the remaining
                // items in submodelsOld - can't delete now
                // because some action list could use the model
            }

            // appName = serviceGroup->comment();
        }
    }
    emit updated();
}

QString Applications::title(int index) const
{
    if (index >= size()) return "";
    return
        (index < m_submodels.size()) ?
            m_submodels.at(index)->selfTitle() :
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
            m_submodels.at(index)->selfIcon() :
            m_items.at(index - m_submodels.size()).icon;
}

bool Applications::isCategory(int index) const
{
    //Q_UNUSED(index);
    //return false;
    if (m_flat) {
        return false;
    }
    return (index < m_submodels.size());
}

int Applications::size() const
{
    return m_submodels.size() + m_items.size();
}

void Applications::activate(int index)
{
    if (index >= size() || index < 0) return;

    if (index < m_submodels.size()) {
        if (m_flat) {
            // opening the dir in external viewer
            new KRun(KUrl("applications:/" + m_submodels[index]->m_root), 0);
        }
        return;
    }

    QString data = m_items.at(index - m_submodels.size()).desktopFile;
    Logger::instance()->log("applications-model", data);
    new KRun(KUrl(data), 0);
    ApplicationConnector::instance()->hide(true);
}

QMimeData * Applications::mimeData(int index) const
{
    if (index >= size()) return NULL;
    if (index < m_submodels.size()) {
        return BaseModel::mimeForUrl("applications:/" +
            m_submodels.at(index)->m_root);
    }

    return BaseModel::mimeForUrl(m_items.at(index - m_submodels.size()).desktopFile);
}

void Applications::setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    Q_UNUSED(index);
    actions = Qt::CopyAction;
    defaultAction = Qt::CopyAction;
}

Lancelot::ActionTreeModel * Applications::child(int index)
{
    if (index >= m_submodels.size())
        return NULL;
    return m_submodels.at(index);
}

QString Applications::selfTitle() const
{
    return m_title;
}

QIcon Applications::selfIcon() const
{
    return m_icon;
}

bool Applications::hasContextActions(int index) const
{
    return !isCategory(index);
}

void Applications::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    if (isCategory(index)) {
        return;
    }

    menu->addAction(KIcon("list-add"), i18n("Add to Favorites"))
        ->setData(QVariant(0));
}

void Applications::contextActivate(int index, QAction * context)
{
    if (!context) {
        return;
    }

    int appIndex = index - m_submodels.size();
    if (context->data().toInt() == 0) {
        FavoriteApplications::instance()
            ->addFavorite(m_items.at(appIndex).desktopFile);
    }
}

void Applications::sycocaUpdated(const QStringList & resources)
{
    if (resources.contains("services")) {
        load();
    }
}

QMimeData * Applications::selfMimeData() const
{
    return BaseModel::mimeForUrl("applications:/" + m_root);
}

} // namespace Models
