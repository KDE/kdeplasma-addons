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

#include "BaseModel.h"

#include <QFileInfo>
#include <QApplication>

#include <KRun>
#include <KLocalizedString>
#include <KDesktopFile>
#include <KDebug>
#include <KFileItem>
#include <KIcon>
#include <KGlobal>
#include <KMimeType>
#include <KUrl>

class BaseModel::Private {
public:
    Private()
    {
    }

    QString title;
    QIcon icon;
    QList < BaseModel::Item > items;
    bool sendEmits : 1;
};

BaseModel::BaseModel(const QString & title, const QIcon & icon)
    : d(new Private())
{
    d->title = title;
    d->icon  = icon;

    QHash <int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole]    = "display";
    roleNames[Qt::StatusTipRole]  = "description";
    roleNames[Qt::DecorationRole] = "decoration";
    setRoleNames(roleNames);
}

BaseModel::~BaseModel()
{
    delete d;
}

// void BaseModel::activate(int index)
// {
//     QString data = itemAt(index).data.toString();
//
//     int result = -1;
//
//     kDebug() << "starting:" << data;
//
//     if (result != 0) {
//         new KRun(KUrl(data), 0);
//     }
//
//     hideApplicationWindow();
// }

int BaseModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return d->items.count();
}

int BaseModel::count() const
{
    return d->items.count();
}

QVariant BaseModel::data(const QModelIndex & index, int role) const
{
    int row = index.row();

    if (row >= d->items.count() || row < 0) {
        return QVariant();
    }

    switch (role) {
        case Qt::DisplayRole:
            return d->items.at(row).title;

        case Qt::StatusTipRole:
            return d->items.at(row).description;

        case Qt::DecorationRole:
            return d->items.at(row).icon;

        default:
            return QVariant();
    }
}

//

void BaseModel::add(const Item & item)
{
    beginInsertRows(index(0, 0), d->items.count(), d->items.count());
    d->items.append(item);
    endInsertRows();
}

void BaseModel::add(const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    add(Item(title, description, icon, data));
}

void BaseModel::insert(int where, const Item & item)
{
    beginInsertRows(index(0, 0), where, where);
    d->items.insert(where, item);
    endInsertRows();
}

void BaseModel::insert(int where, const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    insert(where, Item(title, description, icon, data));
}

void BaseModel::set(int where, const Item & item)
{
    if (where < 0 || where >= d->items.count()) return;

    d->items[where] = item;

    emit dataChanged(index(where), index(where));
}

void BaseModel::set(int where, const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    set(where, Item(title, description, icon, data));
}

void BaseModel::removeAt(int where)
{
    beginRemoveRows(index(0, 0), where, where);
    d->items.removeAt(where);
    endRemoveRows();
}

const BaseModel::Item & BaseModel::itemAt(int index)
{
    return d->items[index];
}

void BaseModel::clear()
{
    beginResetModel();
    d->items.clear();
    endResetModel();
}





//

int BaseModel::addServices(const QStringList & serviceNames)
{
    int result = 0;
    foreach (const QString & serviceAlternatives, serviceNames) {
        foreach (const QString & serviceName, serviceAlternatives.split('|')) {
            if (addService(serviceName)) {
                ++result;
                break;
            }
        }
    }
    return result;
}

bool BaseModel::addService(const QString & serviceName)
{
    const KService::Ptr service = KService::serviceByStorageId(serviceName);

    // qDebug() << "BaseModel::addService Applic:" << service->isValid() << " " << serviceName;

    return addService(service);
}

bool BaseModel::addService(const KService::Ptr & service)
{
    if (!service || !service->isValid()) {
        return false;
    }

    // kDebug() << service->name()
    //     << service->isApplication() << service->exec()
    //     << service->isValid();

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

int BaseModel::addUrls(const QStringList & urls)
{
    int result = 0;
    foreach (const QString & url, urls) {
        if (addUrl(url)) {
            ++result;
        }
    }
    return result;
}

bool BaseModel::addUrl(const QString & url)
{
    const KUrl kurl(url);
    return addUrl(kurl);
}

bool BaseModel::addUrl(const KUrl & url)
{
    // kDebug() << url;
    KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, url);

    if (url.isLocalFile() && QFileInfo(url.path()).suffix() == "desktop") {
        // .desktop files may be services (type field == 'Application' or 'Service')
        // or they may be other types such as links.
        //
        // first look in the KDE service database to see if this file is a service,
        // otherwise represent it as a generic .desktop file

        KDesktopFile desktopFile(url.path());

        if ((desktopFile.readType() == "Service" || desktopFile.readType() == "Application")
                && addService(url.path())) {
            return true;
        }

        KUrl desktopUrl(desktopFile.readUrl());

        add(
            QFileInfo(url.path()).baseName(),
            desktopUrl.isLocalFile() ? desktopUrl.path() : desktopUrl.prettyUrl(),
            KIcon(desktopFile.readIcon()),
            // url.path() //desktopFile.readUrl()
            url.url()
        );
    } else {
        add(
            fileItem.text(),
            url.isLocalFile() ? url.path() : url.prettyUrl(),
            KIcon(fileItem.iconName()),
            url.url()
        );
    }

    return true;
}

// inserts
int BaseModel::insertServices(int where, const QStringList & serviceNames)
{
    int result = 0;
    foreach (const QString & serviceAlternatives, serviceNames) {
        foreach (const QString & serviceName, serviceAlternatives.split('|')) {
            if (insertService(where + result, serviceName)) {
                ++result;
                break;
            }
        }
    }
    return result;
}

bool BaseModel::insertService(int where, const QString & serviceName)
{
    const KService::Ptr service = KService::serviceByStorageId(serviceName);
    return insertService(where, service);
}

bool BaseModel::insertService(int where, const KService::Ptr & service)
{
    if (!service) {
        return false;
    }

    QString genericName = service->genericName();
    QString appName = service->name();

    insert(
        where,
        genericName.isEmpty() ? appName : genericName,
        genericName.isEmpty() ? "" : appName,
        KIcon(service->icon()),
        service->entryPath()
    );
    return true;
}

int BaseModel::insertUrls(int where, const QStringList & urls)
{
    int result = 0;
    foreach (const QString & url, urls) {
        if (insertUrl(where + result, url)) {
            ++result;
        }
    }
    return result;
}

bool BaseModel::insertUrl(int where, const QString & url)
{
    const KUrl kurl(url);
    return insertUrl(where, kurl);
}

bool BaseModel::insertUrl(int where, const KUrl & url)
{
    KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, url);

    if (url.isLocalFile() && QFileInfo(url.path()).suffix() == "desktop") {
        // .desktop files may be services (type field == 'Application' or 'Service')
        // or they may be other types such as links.
        //
        // first look in the KDE service database to see if this file is a service,
        // otherwise represent it as a generic .desktop file

        KDesktopFile desktopFile(url.path());

        if ((desktopFile.readType() == "Service" || desktopFile.readType() == "Application")
                && insertService(where, url.path())) {
            return true;
        }

        KUrl desktopUrl(desktopFile.readUrl());

        insert(
            where,
            QFileInfo(url.path()).baseName(),
            desktopUrl.isLocalFile() ? desktopUrl.path() : desktopUrl.prettyUrl(),
            KIcon(desktopFile.readIcon()),
            // url.path() //desktopFile.readUrl()
            url.url()
        );
    } else {
        insert(
            where,
            fileItem.text(),
            url.isLocalFile() ? url.path() : url.prettyUrl(),
            KIcon(fileItem.iconName()),
            url.url()
        );
    }

    return true;
}

QString BaseModel::selfTitle() const
{
    return d->title;
}

QIcon BaseModel::selfIcon() const
{
    return d->icon;
}

void BaseModel::setSelfTitle(const QString & title)
{
    d->title = title;
}

void BaseModel::setSelfIcon(const QIcon & icon)
{
    d->icon = icon;
}

#include "BaseModel.moc"

