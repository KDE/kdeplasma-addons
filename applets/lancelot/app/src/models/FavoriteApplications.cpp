/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "FavoriteApplications.h"

#include <KConfig>
#include <KIcon>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KTemporaryFile>
#include "../Serializator.h"

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
    if (addUrl(url)) {
        save();
        return true;
    }
    return false;
}

FavoriteApplications * FavoriteApplications::instance()
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
        addUrls(favoriteList);
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
    : ActionTreeModelProxy(Models::FavoriteApplications::instance(),
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

    /* We don't need this hack anymore in 4.2
     * (the code is now a part of plasma shell)
    KTemporaryFile file;
    file.setAutoRemove(false);
    file.setSuffix(".lancelotpart");

    if (!file.open()) {
        return NULL;
    }

    QTextStream out(&file);
    out << Serializator::serialize(map).toAscii();
    out.flush();

    QMimeData * data = new QMimeData();
    QByteArray urlData = KUrl(file.fileName()).url().toAscii();
    data->setData("text/uri-list", urlData);
    data->setData("text/plain", urlData);
    file.close();

    return data;*/
}

} // namespace Models
