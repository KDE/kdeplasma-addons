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
#include "ui_viewconfig.h"
#include <QtGui/QGraphicsScene>

K_EXPORT_PLASMA_WALLPAPER(org.kde.wallpaper-qml, WallpaperQml)

WallpaperQml::WallpaperQml(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args)
    , m_scene(new QGraphicsScene(this))
    , m_item(0)
    , m_package(0)
{
    m_engine = new QDeclarativeEngine(this);
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(m_engine);
    kdeclarative.initialize();
    kdeclarative.setupBindings();

    m_component = new QDeclarativeComponent(m_engine);
    connect(m_component, SIGNAL(statusChanged(QDeclarativeComponent::Status)), SLOT(componentStatusChanged(QDeclarativeComponent::Status)));
    connect(this, SIGNAL(renderHintsChanged()), SLOT(resizeWallpaper()));
    connect(m_scene, SIGNAL(changed(QList<QRectF>)), SLOT(shouldRepaint(QList<QRectF>)));
}

void WallpaperQml::setPackageName(const QString& packageName)
{
    if (m_package) {
        delete m_package;
    }

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

        resizeWallpaper();
    } else if (s==QDeclarativeComponent::Error) {
        delete m_component;
        m_component = new QDeclarativeComponent(m_engine);
        connect(m_component, SIGNAL(statusChanged(QDeclarativeComponent::Status)), SLOT(componentStatusChanged(QDeclarativeComponent::Status)));
    }
    if (!m_component->errors().isEmpty())
        kDebug() << "wallpaper errors:" << m_component->errors();
}

void WallpaperQml::paint(QPainter *painter, const QRectF& exposedRect)
{
    painter->drawPixmap(exposedRect, m_pixmap, exposedRect.translated(-boundingRect().topLeft()));
}

void WallpaperQml::resizeWallpaper()
{
    m_scene->setSceneRect(QRectF(QPointF(0,0), targetSizeHint()));
    if (m_item) {
        m_item->setSize(targetSizeHint());
    }
    m_pixmap = QPixmap(targetSizeHint().toSize());
    m_pixmap.fill(m_scene->backgroundBrush().color());
    QPainter p(&m_pixmap);
    m_scene->render(&p, QRectF(), QRectF(), Qt::IgnoreAspectRatio);
    p.end();
    emit update(QRectF());
}

void WallpaperQml::shouldRepaint(const QList<QRectF> &rects)
{
    QRectF repaintRect(0,0,0,0);
    foreach (const QRectF& rect, rects) {
        repaintRect = repaintRect.united(rect);
    }

    if (!repaintRect.isEmpty()) {
        QPainter p(&m_pixmap);
        m_scene->render(&p, repaintRect, repaintRect, Qt::IgnoreAspectRatio);
        p.end();
        emit update(repaintRect);
    }
}

QWidget* WallpaperQml::createConfigurationInterface(QWidget* parent)
{
    QWidget* w = new QWidget;
    Ui::ViewConfig v;
    v.setupUi(w);
    
    WallpapersModel* m = new WallpapersModel(w);
    v.m_view->setModel(m);
    if (m_package) {
        v.m_view->setCurrentIndex(m->indexForPackagePath(m_package->path()));
    }
    connect(v.m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(changeWallpaper(QModelIndex)));
    connect(v.m_color, SIGNAL(changed(QColor)), this, SLOT(setBackgroundColor(QColor)));
    connect(this, SIGNAL(changed(bool)), parent, SLOT(settingsChanged(bool)));
    return w;
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
    setBackgroundColor(config.readEntry("color", QColor(Qt::transparent)));
    emit changed(false);
}

void WallpaperQml::save(KConfigGroup& config)
{
    config.writeEntry("packageName", KUrl(m_package->path()).fileName(KUrl::IgnoreTrailingSlash));
    config.writeEntry("color", m_scene->backgroundBrush().color());
    emit changed(false);
}

void WallpaperQml::setBackgroundColor(const QColor& color)
{
    m_scene->setBackgroundBrush(color);
    emit changed(false);
}
