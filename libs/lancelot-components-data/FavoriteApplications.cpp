/*
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <KDebug>
#include <KConfig>
#include <KIcon>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KTemporaryFile>

FavoriteApplications::FavoriteApplications()
    : BaseModel(i18n("Favorites"), KIcon("favorites"))
{
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

void FavoriteApplications::load()
{
    kDebug() << "Getting the list of applications";

    KConfig cfg(KStandardDirs::locate("config", "lancelotrc"));
    KConfigGroup favoritesGroup = cfg.group("Favorites");

    QStringList favoriteList = favoritesGroup.readEntry("FavoriteURLs", QStringList());

    if (favoriteList.empty()) {
        loadDefaultApplications();
        save();
    } else {
        foreach (const QString & favoriteItem, favoriteList) {
            kDebug() << favoriteItem;

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
    for (int i = 0; i < count(); i++) {
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

#include "FavoriteApplications.moc"

