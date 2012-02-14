/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include "Applications_p.h"

#include <KIcon>
#include <KRun>
#include <KServiceGroup>
#include <KStandardDirs>
#include <KSycoca>
#include <KUrl>
#include <KToolInvocation>
#include <KDebug>

#include "FavoriteApplications.h"
#include "Logger.h"

// Applications

namespace Lancelot {
namespace Models {

Applications::Private::Private(Applications * parent)
    : q(parent)
{
    lastIndex = -1;
}

Applications::Private::~Private()
{
    clear();
}

void Applications::Private::sycocaUpdated(const QStringList & resources)
{
    if (resources.contains("services") || resources.contains("apps")) {
        load();
    }
}

void Applications::Private::load()
{
    KServiceGroup::Ptr services = KServiceGroup::group(root);
    if (!services || !services->isValid())
        return;

    if (title.isEmpty() || icon.isNull()) {
        title = services->caption();
        icon = KIcon(services->icon());
    }

    // KServiceGroup::List list = services->entries();
    const KServiceGroup::List list =
            services->entries(true  /* sorted */,
                          true  /* exclude no display entries */,
                          false /* allow separators */,
                          false /* sort by generic name */);
    items.clear();
    QList < Applications * > submodelsOld = submodels;
    submodels.clear();

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
            // data.name = service->name().replace('&', "&&");
            data.name = service->name();
            data.description = service->genericName();
            data.desktopFile = service->entryPath();

            items.append(data);
        } else if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr serviceGroup =
                    KServiceGroup::Ptr::staticCast(p);

            if (serviceGroup->noDisplay() || serviceGroup->childCount() == 0)
                continue;

            bool found = false;
            Applications * model;
            foreach (model, submodelsOld) {
                if (serviceGroup->relPath() == model->d->root) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                submodels.append(new Applications(
                    serviceGroup->relPath(),
                    serviceGroup->caption().replace('&', "&&"),
                    KIcon(serviceGroup->icon())
                ));
            } else {
                submodelsOld.removeAll(model);
                submodels.append(model);
                model->d->load();
                // TODO: Find a way to delete the remaining
                // items in submodelsOld - can't delete now
                // because some action list could use the model
            }

            // appName = serviceGroup->comment();
        }
    }
    emit q->updated();
}

void Applications::Private::clear()
{
    foreach(Applications * applist, submodels) {
        delete applist;
    }
}

Applications::Applications(QString root, QString title, QIcon icon, bool flat)
    : d(new Private(this))
{
    d->root = root;
    d->title = title;
    d->icon = icon;
    d->flat = flat;

    connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)),
            d, SLOT(sycocaUpdated(QStringList)));
    d->load();
}

Applications::~Applications()
{
    delete d;
}

QString Applications::Private::data(int index, Applications::Private::Data what)
{
    if (index >= items.size() + submodels.size()) return "";

    if (index != lastIndex) {
        lastTitle = (index < submodels.size()) ?
                    submodels.at(index)->selfTitle() :
                    items.at(index - submodels.size()).name;
        lastDescription =
                    (index < submodels.size()) ? QString() :
                    items.at(index - submodels.size()).description;

        if (!ApplicationConnector::self()->applicationNameFirst()) {
            QString tmp = lastTitle;
            lastTitle = lastDescription;
            lastDescription = tmp;
        }

        if (lastTitle.isEmpty()) {
            lastTitle = lastDescription;
        }

        if (lastTitle == lastDescription) {
            lastDescription.clear();
        }
    }

    return (what == Applications::Private::Title) ? lastTitle : lastDescription;
}

QString Applications::title(int index) const
{
    return d->data(index, Applications::Private::Title);
}

QString Applications::description(int index) const
{
    return d->data(index, Applications::Private::Description);
}

QIcon Applications::icon(int index) const
{
    if (index >= size()) return QIcon();
    return
        (index < d->submodels.size()) ?
            d->submodels.at(index)->selfIcon() :
            d->items.at(index - d->submodels.size()).icon;
}

bool Applications::isCategory(int index) const
{
    //Q_UNUSED(index);
    //return false;
    if (d->flat) {
        return false;
    }
    return (index < d->submodels.size());
}

int Applications::size() const
{
    return d->submodels.size() + d->items.size();
}

void Applications::activate(int index)
{
    if (index >= size() || index < 0) return;

    if (index < d->submodels.size()) {
        if (d->flat) {
            // opening the dir in external viewer
            new KRun(KUrl("applications:/" + d->submodels[index]->d->root), 0);
        }
        return;
    }

    QString data = d->items.at(index - d->submodels.size()).desktopFile;

    int result = KToolInvocation::startServiceByDesktopPath(data, QStringList(), 0, 0, 0, "", true);

    Logger::self()->log("applications-model", data);

    if (result != 0) {
        new KRun(KUrl(data), 0);
    }

    ApplicationConnector::self()->hide(true);
}

QMimeData * Applications::mimeData(int index) const
{
    if (index >= size()) return NULL;
    if (index < d->submodels.size()) {
        return BaseModel::mimeForUrl("applications:/" +
            d->submodels.at(index)->d->root);
    }

    return BaseModel::mimeForUrl(d->items.at(index - d->submodels.size()).desktopFile);
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
    if (index >= d->submodels.size())
        return NULL;
    return d->submodels.at(index);
}

QString Applications::selfTitle() const
{
    return d->title;
}

QIcon Applications::selfIcon() const
{
    return d->icon;
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

    int appIndex = index - d->submodels.size();
    if (context->data().toInt() == 0) {
        FavoriteApplications::self()
            ->addFavorite(d->items.at(appIndex).desktopFile);
    }
}

QMimeData * Applications::selfMimeData() const
{
    return BaseModel::mimeForUrl("applications:/" + d->root);
}

} // namespace Models
} // namespace Lancelot
