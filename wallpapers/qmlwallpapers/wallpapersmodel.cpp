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
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/qdeclarativeitem.h>
#include <QtGui/qpixmap.h>
#include <QtGui/QPainter>
#include <QGraphicsScene>
#include <KIcon>
#include <KStandardDirs>
#include <plasma/package.h>
#include <qdir.h>
#include <kdeclarative.h>

Q_DECLARE_METATYPE(Plasma::Package*);

WallpapersModel::WallpapersModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_scene = new QGraphicsScene(this);
    m_engine = new QDeclarativeEngine(this);
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(m_engine);
    kdeclarative.initialize();
    kdeclarative.setupBindings();
}

WallpapersModel::~WallpapersModel()
{
    clearCache();
}

void WallpapersModel::clearCache()
{
    qDeleteAll(m_packagesItems);
}

void WallpapersModel::addPackage(const QString& packageRoot, const QString& packageName)
{
    beginInsertRows(QModelIndex(), m_packages.count(), m_packages.count());
    Plasma::PackageStructure::Ptr str = Plasma::PackageStructure::load("Plasma/Generic");
    Plasma::Package* p = new Plasma::Package(packageRoot, packageName, str);
    
    if (p->isValid() && p->metadata().serviceType()=="Plasma/DeclarativeWallpaper") {
        m_packages += p;
        QDeclarativeComponent* component = new QDeclarativeComponent(m_engine);
        component->loadUrl(QUrl(p->filePath("mainscript")));
        if (component->isReady()) {
            QDeclarativeItem* item = qobject_cast<QDeclarativeItem *>(component->create());
            Q_ASSERT(item);
            m_packagesItems[p] = item;
            item->setWidth(m_scene->width());
            item->setHeight(m_scene->height());
            item->setVisible(false);
            m_scene->addItem(item);
        }
        delete component;
    } else {
        delete p;
    }
    endInsertRows();
}

QVariant WallpapersModel::data(const QModelIndex& index, int role) const
{
    Plasma::Package* p = m_packages[index.row()];
    switch (role) {
        case PackageNameRole:
            return KUrl(p->path()).fileName(KUrl::IgnoreTrailingSlash);
        case Qt::DisplayRole:
            return p->metadata().name();
        case Qt::ToolTipRole:
            return p->metadata().description();
        case Qt::DecorationRole:
            return KIcon(p->metadata().icon());
        case BackgroundDelegate::AuthorRole:
            return p->metadata().author();
        case BackgroundDelegate::ScreenshotRole:
            QDeclarativeItem* it = m_packagesItems.value(p);
            if(it) {
                QPixmap pix(m_scene->sceneRect().size().toSize());
                pix.fill(Qt::transparent);
                QPainter painter(&pix);
                it->setVisible(true);
                m_scene->render(&painter, QRectF(), QRectF(), Qt::KeepAspectRatio);
                it->setVisible(false);
                return pix;
            }
            break;
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

void WallpapersModel::setWallpaperSize(const QSize& size)
{
    float newHeight = ((float)size.height() / (float)size.width()) * BackgroundDelegate::SCREENSHOT_SIZE;

    m_size = QSize(BackgroundDelegate::SCREENSHOT_SIZE, newHeight);

    m_size.scale(BackgroundDelegate::SCREENSHOT_SIZE, BackgroundDelegate::SCREENSHOT_SIZE/1.6, Qt::KeepAspectRatio);

    reload();
}

void WallpapersModel::reload(){
    m_scene->setSceneRect(0,0, m_size.width(), m_size.height());
    QStringList dirs(KGlobal::dirs()->findDirs("data", "plasma/wallpapers"));
    foreach (const QString &dir, dirs) {
        foreach (const QString &package, Plasma::Package::listInstalled(dir)) {
            addPackage(dir, package);
        }
    }
}
