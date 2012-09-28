/*
 *   Copyright 2012 by Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "wallpapersmodel.h"
#include <KIcon>
#include <KStandardDirs>
#include <plasma/package.h>
#include <qdir.h>

WallpapersModel::WallpapersModel(QObject* parent)
    : QAbstractListModel(parent)
{
    QStringList dirs(KGlobal::dirs()->findDirs("data", "plasma/wallpapers"));
    foreach (const QString &dir, dirs) {
        foreach (const QString &package, Plasma::Package::listInstalled(dir)) {
            addPackage(dir, package);
        }
    }
}

void WallpapersModel::addPackage(const QString& packageRoot, const QString& packageName)
{
    beginInsertRows(QModelIndex(), m_packages.count(), m_packages.count());
    Plasma::PackageStructure::Ptr str = Plasma::PackageStructure::load("Plasma/Generic");
    Plasma::Package* p = new Plasma::Package(packageRoot, packageName, str);
    if (p->isValid() && p->metadata().serviceType()=="Plasma/DeclarativeWallpaper") {
        m_packages += p;
    } else {
        delete p;
    }
    endInsertRows();
}

QVariant WallpapersModel::data(const QModelIndex& index, int role) const
{
    Plasma::Package* p = m_packages[index.row()];
    switch(role) {
        case PackageNameRole:
            return KUrl(p->path()).fileName(KUrl::IgnoreTrailingSlash);
        case Qt::DisplayRole:
            return p->metadata().name();
        case Qt::ToolTipRole:
            return p->metadata().description();
        case Qt::DecorationRole:
            return KIcon(p->metadata().icon());
    }
    return QVariant();
}

int WallpapersModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid()? 0 : m_packages.count();
}

QModelIndex WallpapersModel::indexForPackagePath(const QString& path)
{
    for (int i = 0; i<rowCount(); i++) {
        if (m_packages[i]->path()==path) {
            return index(i,0);
        }
    }
    return QModelIndex();
}
