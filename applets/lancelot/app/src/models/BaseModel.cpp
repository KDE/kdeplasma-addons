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

#include "BaseModel.h"
#include <QFileInfo>

#include <KRun>
#include <KLocalizedString>
#include <KDesktopFile>
#include <KIcon>
#include <KGlobal>
#include <KMimeType>
#include <KUrl>

namespace Models {

BaseModel::BaseModel()
{
}

BaseModel::~BaseModel()
{
}

void BaseModel::activate(int index)
{
    new KRun(KUrl(m_items.at(index).data.toString()), 0);
    hideLancelotWindow();
}

void BaseModel::hideLancelotWindow()
{
    LancelotApplication::hide(true);
}

void BaseModel::changeLancelotSearchString(const QString & string)
{
    LancelotApplication::search(string);
}

void BaseModel::addService(const QString & serviceName)
{
    const KService::Ptr service = KService::serviceByStorageId(serviceName);
    addService(service);
}

void BaseModel::addService(const KService::Ptr & service)
{
    if (service) {
        QString genericName = service->genericName();
        QString appName = service->name();

        add(
            genericName.isEmpty() ? appName : genericName,
            genericName.isEmpty() ? "" : appName,
            KIcon(service->icon()),
            service->entryPath()
        );
    }
}

void BaseModel::addUrl(const QString & url)
{
    const KUrl kurl(url);
    addUrl(kurl);
}

void BaseModel::addUrl(const KUrl & url)
{
    if (url.isLocalFile() && QFileInfo(url.path()).suffix() == "desktop") {

        // .desktop files may be services (type field == 'Application' or 'Service')
        // or they may be other types such as links.
        //
        // first look in the KDE service database to see if this file is a service,
        // otherwise represent it as a generic .desktop file

        const KService::Ptr service = KService::serviceByDesktopPath(url.path());
        if (service) {
            return addService(service);
        }

        KDesktopFile desktopFile(url.path());
        KUrl desktopUrl(desktopFile.readUrl());

        add(
            QFileInfo(url.path()).baseName(),
            desktopUrl.isLocalFile() ? desktopUrl.path() : desktopUrl.prettyUrl(),
            KIcon(desktopFile.readIcon()),
            desktopFile.readUrl()
        );
    } else {
        add(
            QFileInfo(url.path()).baseName(),
            url.isLocalFile() ? url.path() : url.prettyUrl(),
            KIcon(KMimeType::iconNameForUrl(url)),
            url.url()
        );
    }
}

} // namespace Models
