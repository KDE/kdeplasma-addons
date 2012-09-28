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

#include "wallpapersqml.h"
#include "wallpapersmodel.h"
#include <plasma/package.h>
#include <KStandardDirs>
#include <kdeclarative.h>
#include <QGraphicsScene>
#include <QDeclarativeEngine>
#include <QDeclarativeItem>
#include <QDeclarativeComponent>
#include <QPainter>
#include <QListView>
#include <qdir.h>

K_EXPORT_PLASMA_WALLPAPER(wallpaper-qml, WallpaperQml)

WallpaperQml::WallpaperQml(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args)
    , m_scene(new QGraphicsScene(this))
    , m_item(0)
    , m_package(0)
{
    QDeclarativeEngine* engine = new QDeclarativeEngine(this);
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine);
    kdeclarative.initialize();
    kdeclarative.setupBindings();
    
    m_component = new QDeclarativeComponent(engine);
    connect(m_component, SIGNAL(statusChanged(QDeclarativeComponent::Status)), SLOT(componentStatusChanged(QDeclarativeComponent::Status)));
    connect(this, SIGNAL(renderHintsChanged()), SLOT(resizeWallpaper()));
    connect(m_scene, SIGNAL(changed(QList<QRectF>)), SLOT(shouldRepaint(QList<QRectF>)));
}

void WallpaperQml::setPackageName(const QString& packageName)
{
    if (m_package)
        delete m_package;
    
    kDebug() << "loading package..." << packageName;
    m_structure = Plasma::PackageStructure::load("Plasma/Generic");
    QStringList dirs(KGlobal::dirs()->findDirs("data", "plasma/wallpapers"));
    foreach (const QString &dir, dirs) {
        m_package = new Plasma::Package(dir, packageName, m_structure);
        if (m_package->isValid() && !m_package->filePath("mainscript").isEmpty()) {
            break;
        }
    }

    Q_ASSERT(m_package->isValid());
    QUrl scriptUrl(m_package->filePath("mainscript"));
    if (scriptUrl.isValid()) {
        m_component->loadUrl(scriptUrl);
    } else {
        m_component->setData("import QtQuick 1.1\n Text { text: 'wrong wallpaper'}", QDir::tempPath());
    }
}

void WallpaperQml::componentStatusChanged(QDeclarativeComponent::Status s)
{
    if (s==QDeclarativeComponent::Ready) {
        if (m_item) {
            m_scene->removeItem(m_item);
            delete m_item;
        }
        
        m_item = qobject_cast<QDeclarativeItem *>(m_component->create());
        m_item->setSize(targetSizeHint());
        Q_ASSERT(m_item);
        m_scene->addItem(m_item);
        
        emit update(QRectF());
    }
    if (!m_component->errors().isEmpty())
        kDebug() << "wallpaper errors:" << m_component->errors();
}

void WallpaperQml::paint(QPainter *painter, const QRectF& exposedRect)
{
    m_scene->render(painter, exposedRect, exposedRect, Qt::IgnoreAspectRatio);
}

void WallpaperQml::resizeWallpaper()
{
    m_scene->setSceneRect(QRectF(QPointF(0,0), targetSizeHint()));
    if (m_item) {
        m_item->setSize(targetSizeHint());
    }
}

void WallpaperQml::shouldRepaint(const QList<QRectF> &rects)
{
    QRectF repaintRect(0,0,0,0);
    foreach (const QRectF& rect, rects) {
        repaintRect = repaintRect.united(rect);
    }
    
    if (!repaintRect.isEmpty()) {
        emit update(repaintRect);
    }
}

QWidget* WallpaperQml::createConfigurationInterface(QWidget* parent)
{
    QListView* view = new QListView(parent);
    WallpapersModel* m = new WallpapersModel(view);
    
    view->setModel(m);
    if (m_package) {
        view->setCurrentIndex(m->indexForPackagePath(m_package->path()));
    }
    connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(changeWallpaper(QModelIndex)));
    connect(this, SIGNAL(changed(bool)), parent, SLOT(settingsChanged(bool)));
    return view;
}

void WallpaperQml::changeWallpaper(const QModelIndex& idx)
{
    QString name = idx.data(WallpapersModel::PackageNameRole).toString();
    emit changed(true);
    setPackageName(name);
}

void WallpaperQml::init(const KConfigGroup& config)
{
    setPackageName(config.readEntry("packageName", "org.kde.animals"));
	emit changed(false);
}

void WallpaperQml::save(KConfigGroup& config)
{
    config.writeEntry("packageName", KUrl(m_package->path()).fileName(KUrl::IgnoreTrailingSlash));
	emit changed(false);
}
