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
#include <QGraphicsScene>
#include <QDeclarativeEngine>
#include <QDeclarativeItem>
#include <QDeclarativeComponent>
#include <QPainter>

K_EXPORT_PLASMA_WALLPAPER(wallpaper-qml, WallpaperQml)

WallpaperQml::WallpaperQml(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args)
{
    m_scene = new QGraphicsScene(this);
    QDeclarativeEngine* engine = new QDeclarativeEngine(this);
    QDeclarativeComponent* component = new QDeclarativeComponent(engine);
    component->loadUrl(QUrl::fromLocalFile("/home/kde-devel/tmp/wp.qml"));
    m_item = qobject_cast<QDeclarativeItem *>(component->create());
    Q_ASSERT(m_item);
    m_scene->addItem(m_item);
    
    connect(this, SIGNAL(renderHintsChanged()), SLOT(resizeWallpaper()));
    connect(m_scene, SIGNAL(changed(QList<QRectF>)), SLOT(shouldRepaint(QList<QRectF>)));
}

void WallpaperQml::paint(QPainter *painter, const QRectF& exposedRect)
{
    painter->setBrush(Qt::green);
    painter->drawRect(exposedRect);
    m_scene->render(painter, exposedRect, exposedRect, Qt::IgnoreAspectRatio);
}

void WallpaperQml::resizeWallpaper()
{
    m_scene->setSceneRect(QRectF(QPointF(0,0), targetSizeHint()));
    m_item->setSize(targetSizeHint());
}

void WallpaperQml::shouldRepaint(const QList<QRectF> &rects)
{
    QRectF repaintRect(0,0,0,0);
    foreach(const QRectF& rect, rects)
        repaintRect = repaintRect.united(rect);
    
    if(!repaintRect.isEmpty())
        emit update(repaintRect);
}
