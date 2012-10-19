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

#ifndef WALLPAPERSMODEL_H
#define WALLPAPERSMODEL_H

#include <QAbstractItemModel>
#include <QDeclarativeComponent>
#include <QDeclarativeItem>
#include <plasma/packagestructure.h>
#include "backgrounddelegate.h"

namespace Plasma { class Package; }

class WallpapersModel : public QAbstractListModel
{
    Q_OBJECT
    public:
        enum Roles {
            PackageNameRole = BackgroundDelegate::ResolutionRole+1
        };

        explicit WallpapersModel(QObject* parent = 0);
        ~WallpapersModel();

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        QModelIndex indexForPackagePath(const QString& path);
        void clearCache();
        void reload();
        void setWallpaperSize(const QSize& size);

    private:
        void addPackage(const QString& root, const QString& name);

        QList<Plasma::Package*> m_packages;
        QMap<Plasma::Package*, QDeclarativeItem*> m_packagesItems;
        QDeclarativeEngine* m_engine;
        QGraphicsScene* m_scene;
        QSize m_size;
};

#endif // WALLPAPERSMODEL_H
