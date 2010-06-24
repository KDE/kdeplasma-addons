/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "FavoriteApplications.h"

#include <QDebug>
#include <KConfig>
#include <KIcon>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KTemporaryFile>
#include "Serializator.h"

namespace Lancelot {
namespace Models {

FavoriteApplications * FavoriteApplications::m_instance = NULL;

FavoriteApplications::FavoriteApplications()
    : BaseModel(true)
{
    setSelfTitle(i18n("Favorites"));
    setSelfIcon(KIcon("favorites"));
    load();
}

FavoriteApplications::~FavoriteApplications()
{
}

bool FavoriteApplications::addFavorite(QString url)
{
    if (url.endsWith(".desktop")) {
        addService(url);
        save();

    } else if (addUrl(url)) {
        save();
        return true;
    }
    return false;
}

FavoriteApplications * FavoriteApplications::self()
{
    if (!m_instance) {
        m_instance = new FavoriteApplications();
    }
    return m_instance;
}

void FavoriteApplications::load()
{
    KConfig cfg(KStandardDirs::locate("config", "lancelotrc"));
    KConfigGroup favoritesGroup = cfg.group("Favorites");

    QStringList favoriteList = favoritesGroup.readEntry("FavoriteURLs", QStringList());

    if (favoriteList.empty()) {
        loadDefaultApplications();
        save();
    } else {
        foreach (const QString & favoriteItem, favoriteList) {
            if (favoriteItem.endsWith(".desktop")) {
                addService(favoriteItem);

            } else {
                addUrl(favoriteItem);

            }
        }
    }
}

void FavoriteApplications::save()
{
    KConfig cfg(KStandardDirs::locate("config", "lancelotrc"));
    KConfigGroup favoritesGroup = cfg.group("Favorites");

    QStringList favoriteList;
    for (int i = 0; i < size(); i++) {
        favoriteList << itemAt(i).data.toString();
    }

    favoritesGroup.writeEntry("FavoriteURLs", favoriteList);
    favoritesGroup.sync();
}

void FavoriteApplications::loadDefaultApplications()
{
    QStringList result;

    // First, we check whether Kickoff is already set up,
    // if it is, we are using its Favorites
    KConfig cfg(KStandardDirs::locate("config", "kickoffrc"));
    KConfigGroup favoritesGroup = cfg.group("Favorites");
    result = favoritesGroup.readEntry("FavoriteURLs", QStringList());
    if (!result.empty()) {
        addUrls(result);
        return;
    }

    QStringList applications;
    applications
        << "firefox|konqbrowser"
        << "kmail|thunderbird"
        << "kopete|gaim"
        << "kate|gvim|kedit"
        << "konsole|xterm";
    addServices(applications);
}

bool FavoriteApplications::hasContextActions(int index) const
{
    Q_UNUSED(index);
    return true;
}

void FavoriteApplications::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    Q_UNUSED(index);
    menu->addAction(KIcon("list-remove"), i18n("Remove From Favorites"))
        ->setData(QVariant(0));
}

void FavoriteApplications::contextActivate(int index, QAction * context)
{
    if (!context) {
        return;
    }

    if (context->data().toInt() == 0) {
        removeAt(index);
        save();
    }
}

FavoriteApplications::PassagewayViewProxy::PassagewayViewProxy()
    : ActionTreeModelProxy(Models::FavoriteApplications::self(),
            i18n("Favorites"), KIcon("favorites"))
{
}

FavoriteApplications::PassagewayViewProxy::~PassagewayViewProxy()
{
}

QMimeData * FavoriteApplications::PassagewayViewProxy::selfMimeData() const
{
    QMap < QString , QString > map;
    map["version"] = "1.0";
    map["type"]    = "list";
    map["model"]   = "FavoriteApplications";

    QMimeData * data = new QMimeData();
    data->setData("text/x-lancelotpart", Serializator::serialize(map).toAscii());
    return data;
}

bool FavoriteApplications::dataDropAvailable(int where, const QMimeData * mimeData)
{
    Q_UNUSED(where);
    return (mimeData->formats().contains("text/uri-list"));
}

void FavoriteApplications::dataDropped(int where, const QMimeData * mimeData)
{
    if (mimeData->formats().contains("text/uri-list")) {
        int from = -1;

        KUrl url = KUrl(QString(mimeData->data("text/uri-list")));

        for (int i = 0; i < size(); i++) {
            if (url.path() == itemAt(i).data) {
                from = i;
                break;
            }
        }

        if (from != -1) {
            removeAt(from);
        }
        insertUrl(where, url);

        save();
    }
}

} // namespace Models
} // namespace Lancelot
