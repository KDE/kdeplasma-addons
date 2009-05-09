/*
Copyright 2008-2009 by Frederik Gladhorn <gladhorn@kde.org>
Copyright 2008-2009 by Sascha Peilicke <sasch.pe@gmx.de>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MARBLE_WALLPAPER_H
#define MARBLE_WALLPAPER_H

#include "ui_config.h"

#include <Plasma/Wallpaper>
#include <marble/global.h>

namespace Marble {

class MarbleMap;

class MarbleWallpaper : public Plasma::Wallpaper
{
Q_OBJECT
public:
    enum Movement {
        Interactive = 0,        //< Allow user interactive navigation
        FollowSun,              //< Rotate by following the sun
        Rotate,                 //< Rotate by configured values
        DontMove
    };

    MarbleWallpaper(QObject *parent, const QVariantList &args);
    ~MarbleWallpaper();

    /// Show config
    QWidget * createConfigurationInterface(QWidget *parent);
    /// Save config
    void save(KConfigGroup &config);
    /// Paint the wallpaper
    void paint(QPainter *painter, const QRectF &exposedRect);

signals:
    void settingsChanged(bool);

protected:
    void init(const KConfigGroup &config);

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

private slots:
    void updateGlobe();                 //< Called by the timer
    void updateSettings();              //< Changes from the settings dialog
    void changeTheme(int index);        //< Change the map theme
    void updateConfigScreen(int);       //< Updates config screen

private:
    Ui_MarbleSettingsWidget m_ui;

    QTimer *m_timer;                    // Update timer
    MarbleMap *m_map;                   // Marble parameters for GeoPainter
    Projection m_projection;            // Type of projection
    MapQuality m_quality;               // Painting quality
    Movement m_movement;                // Current movement/rotation scheme
    QString m_mapTheme;                 // Theme of the map
    bool m_showPlacemarks;              // Visibility flag for all placemarks

    qreal m_zoom;                       // Last saved zoom / distance
    qreal m_positionLon;                // Last saved position lon
    qreal m_positionLat;                // Last saved position lat
    qreal m_rotationLon;                // Used for 'follow sun'
    qreal m_rotationLat;                // Used for 'follow sun'
    int m_rotationTimeout;              // Used for 'follow sun' and 'rotate'

    int m_dragStartPositionX;           // position of last button press X
    int m_dragStartPositionY;           // position of last button press Y
    qreal m_leftPressedTranslationX;
    qreal m_leftPressedTranslationY;

    QPixmap m_pixmap;                   // Cached wallpaper
};

K_EXPORT_PLASMA_WALLPAPER(marble, MarbleWallpaper)

} // namspace Marble

#endif
