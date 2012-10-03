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

#ifndef WALLPAPERQML_H
#define WALLPAPERQML_H

#include <plasma/wallpaper.h>
#include <QtDeclarative/QDeclarativeComponent>

class QModelIndex;
class QDeclarativeItem;

class WallpaperQml : public Plasma::Wallpaper
{
    Q_OBJECT
    public:
        WallpaperQml(QObject* parent, const QVariantList& args);

        virtual void paint(QPainter* painter, const QRectF& exposedRect);
        virtual QWidget* createConfigurationInterface(QWidget* parent);
        virtual void save(KConfigGroup& config);
        virtual void init(const KConfigGroup& config);

    private slots:
        void resizeWallpaper();
        void shouldRepaint(const QList< QRectF >& rects);
        void componentStatusChanged(QDeclarativeComponent::Status s);
        void setPackageName(const QString& name);
        void changeWallpaper(const QModelIndex& idx);
        void setBackgroundColor(const QColor& color);

    signals:
        void changed(bool hasChanged=true);

    private:
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
        virtual void wheelEvent(QGraphicsSceneWheelEvent* event);
        
        QGraphicsScene* m_scene;
        QDeclarativeItem* m_item;
        QDeclarativeComponent* m_component;
        Plasma::Package* m_package;
        Plasma::PackageStructure::Ptr m_structure;
        QPixmap m_pixmap;
        QDeclarativeEngine* m_engine;
        QString m_packageName;
};

#endif
