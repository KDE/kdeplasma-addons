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
#include <KDebug>

namespace Models {

ApplicationConnector * ApplicationConnector::m_instance = NULL;

ApplicationConnector * ApplicationConnector::instance()
{
    if (m_instance == NULL) {
        m_instance = new ApplicationConnector();
    }
    return m_instance;
}

void ApplicationConnector::search(const QString & search)
{
    emit doSearch(search);
}

bool ApplicationConnector::hide(bool immediate)
{
    return emit doHide(immediate);
}

ApplicationConnector::ApplicationConnector()
{

}

ApplicationConnector::~ApplicationConnector()
{

}


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
    ApplicationConnector::instance()->hide(true);
}

void BaseModel::changeLancelotSearchString(const QString & string)
{
    ApplicationConnector::instance()->search(string);
}

void BaseModel::addServices(const QStringList & serviceNames)
{
    foreach (const QString & serviceAlternatives, serviceNames) {
        foreach (const QString & serviceName, serviceAlternatives.split("|")) {
            if (addService(serviceName)) {
                break;
            }
        }
    }
}

bool BaseModel::addService(const QString & serviceName)
{
    const KService::Ptr service = KService::serviceByStorageId(serviceName);
    return addService(service);
}

bool BaseModel::addService(const KService::Ptr & service)
{
    if (!service) {
        return false;
    }

    QString genericName = service->genericName();
    QString appName = service->name();

    add(
        genericName.isEmpty() ? appName : genericName,
        genericName.isEmpty() ? "" : appName,
        KIcon(service->icon()),
        service->entryPath()
    );
    return true;
}

void BaseModel::addUrls(const QStringList & urls)
{
    foreach (const QString & url, urls) {
        addUrl(url);
    }
}

bool BaseModel::addUrl(const QString & url)
{
    const KUrl kurl(url);
    return addUrl(kurl);
}

bool BaseModel::addUrl(const KUrl & url)
{
    kDebug() << url;
    if (url.isLocalFile() && QFileInfo(url.path()).suffix() == "desktop") {
        // .desktop files may be services (type field == 'Application' or 'Service')
        // or they may be other types such as links.
        //
        // first look in the KDE service database to see if this file is a service,
        // otherwise represent it as a generic .desktop file

        if (addService(url.path())) {
            return true;
        }

        kDebug() << "Local desktop file - Application " << url.path();
        KDesktopFile desktopFile(url.path());
        KUrl desktopUrl(desktopFile.readUrl());
        kDebug() << desktopUrl;

        add(
            QFileInfo(url.path()).baseName(),
            desktopUrl.isLocalFile() ? desktopUrl.path() : desktopUrl.prettyUrl(),
            KIcon(desktopFile.readIcon()),
            // url.path() //desktopFile.readUrl()
            url.url()
        );
    } else {
        kDebug() << "Normal URL";
        add(
            QFileInfo(url.path()).baseName(),
            url.isLocalFile() ? url.path() : url.prettyUrl(),
            KIcon(KMimeType::iconNameForUrl(url)),
            url.url()
        );
    }

    return true;
}

} // namespace Models
