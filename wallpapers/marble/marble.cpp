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

#include "marble.h"

#include <marble/GeoPainter.h>
#include <marble/MapThemeManager.h>
#include <marble/MarbleMap.h>
#include <marble/RenderPlugin.h>
#include <marble/SunLocator.h>
#include <marble/ViewParams.h>
#include <marble/ViewportParams.h>

#include <QGraphicsSceneMouseEvent>
#include <QStandardItemModel>
#include <QTimer>

// Distance (pixel) before starting drag updates
#define DRAGTHRESHOLD 3

// Type of movement (follow sun, auto rotate, user)
#define MOVEMENT_CONFIG_KEY "movement"
// Update timeout when auto rotating
#define ROTATION_TIMEOUT_CONFIG_KEY "rotationTimeout"
// Amount of rotation (degrees/second)
#define ROTATION_LON_CONFIG_KEY "rotateLongitude"
#define ROTATION_LAT_CONFIG_KEY "rotateLatitude"

// Last position
#define POSITION_LON_CONFIG_KEY "positionLongitude"
#define POSITION_LAT_CONFIG_KEY "positionLatitude"
#define POSITION_DISTANCE_CONFIG_KEY "positionDistance"

// Marble settings
#define MAP_THEME_CONFIG_KEY "mapTheme"
#define PROJECTION_CONFIG_KEY "projection"
#define QUALITY_CONFIG_KEY "quality"
#define SHOW_PLACEMARKS_CONFIG_KEY "showPlacemarks"

namespace Marble {

MarbleWallpaper::MarbleWallpaper(QObject * parent, const QVariantList & args )
    : Plasma::Wallpaper(parent, args), m_map(0), m_rotationTimer(0)
{
    KGlobal::locale()->insertCatalog("marble");
}

MarbleWallpaper::~MarbleWallpaper()
{
    delete m_map;
}

void MarbleWallpaper::init(const KConfigGroup &config)
{
    m_map = new MarbleMap();

    // Get default position from marble to initialize on first startup (empty config)
    qreal lon;
    qreal lat;
    int zoom;
    m_map->home(lon, lat, zoom);

    // Read settings
    m_movement = static_cast<Movement>(config.readEntry(MOVEMENT_CONFIG_KEY, static_cast<int>(DontMove)));
    m_mapTheme = config.readEntry(MAP_THEME_CONFIG_KEY, QString::fromLatin1("earth/bluemarble/bluemarble.dgml"));
    m_positionDist = config.readEntry(POSITION_DISTANCE_CONFIG_KEY, 4500);
    m_positionLon = config.readEntry(POSITION_LON_CONFIG_KEY, lon);
    m_positionLat = config.readEntry(POSITION_LAT_CONFIG_KEY, lat);
    m_projection = static_cast<Projection>(config.readEntry(PROJECTION_CONFIG_KEY, static_cast<int>(Spherical)));
    m_quality = static_cast<MapQuality>(config.readEntry(QUALITY_CONFIG_KEY, static_cast<int>(Normal)));
    m_rotationLat = config.readEntry(ROTATION_LAT_CONFIG_KEY, 0.0);
    m_rotationLon = config.readEntry(ROTATION_LON_CONFIG_KEY, 0.025);
    m_rotationTimeout = config.readEntry(ROTATION_TIMEOUT_CONFIG_KEY, 10000);
    m_showPlacemarks = config.readEntry(SHOW_PLACEMARKS_CONFIG_KEY, true);

    // Set up the map
    m_map->setMapThemeId(m_mapTheme);
    m_map->setShowCompass(false);
    m_map->setShowGrid(false);
    m_map->setShowScaleBar(false);
    m_map->sunLocator()->setCitylights(true);
    m_map->sunLocator()->setShow(true);
    m_map->centerOn(m_positionLon, m_positionLat);
    m_map->setDistance(m_positionDist);

    // Disable all render plugins except the "stars" plugin
    foreach (RenderPlugin *item, m_map->renderPlugins()) {
        if (item->nameId() == "stars") {
            item->setVisible(true);
            item->setEnabled(true);
        } else {
            item->setVisible(false);
            item->setEnabled(false);
        }
    }

    widgetChanged();
}

QWidget *MarbleWallpaper::createConfigurationInterface(QWidget *parent)
{
    QWidget *configWidget = new QWidget(parent);
    m_ui.setupUi(configWidget);
    m_ui.movement->setCurrentIndex(static_cast<int>(m_movement));
    m_ui.projection->setCurrentIndex(static_cast<int>(m_projection));
    // The first MapQuality value is wireframe, which we don't show in the list
    m_ui.quality->setCurrentIndex(static_cast<int>(m_quality) - 1);
    m_ui.rotationLon->setValue(m_rotationLon);
    m_ui.rotationLat->setValue(m_rotationLat);
    m_ui.showPlacemarks->setChecked(m_showPlacemarks);
    m_ui.timeout->setValue(m_rotationTimeout / 1000);

    MapThemeManager themeManager;
    themeManager.updateMapThemeModel();
    // FIXME: Going manually through the model is ugly as hell, but plugging the model into the view didn't work for me
    for (int i = 0; i < themeManager.mapThemeModel()->rowCount(); i++) {
        QModelIndex index = themeManager.mapThemeModel()->index(i, 0, QModelIndex());
        QString theme = themeManager.mapThemeModel()->data(index, Qt::DisplayRole).toString();
        QIcon icon = qvariant_cast<QIcon>(themeManager.mapThemeModel()->data(index, Qt::DecorationRole));
        QModelIndex fileIndex = themeManager.mapThemeModel()->index(i, 1, QModelIndex());
        QString themeFile = themeManager.mapThemeModel()->data(fileIndex, Qt::DisplayRole).toString();
        m_ui.themeList->addItem(icon, theme, themeFile);
        if (m_mapTheme == themeFile) {
            m_ui.themeList->setCurrentIndex(i);
        }
    }

    // Trigger initial visual movement setup
    updateConfigScreen(static_cast<int>(m_movement));

    connect(m_ui.movement, SIGNAL(currentIndexChanged(int)), SLOT(updateConfigScreen(int)));
    connect(m_ui.projection, SIGNAL(currentIndexChanged(int)), SLOT(updateSettings()));
    connect(m_ui.quality, SIGNAL(currentIndexChanged(int)), SLOT(updateSettings()));
    connect(m_ui.rotationLon, SIGNAL(valueChanged(double)), SLOT(updateSettings()));
    connect(m_ui.rotationLat, SIGNAL(valueChanged(double)), SLOT(updateSettings()));
    connect(m_ui.showPlacemarks, SIGNAL(stateChanged(int)), SLOT(updateSettings()));
    connect(m_ui.themeList, SIGNAL(currentIndexChanged(int)), SLOT(changeTheme(int)));
    connect(m_ui.timeout, SIGNAL(valueChanged(int)), SLOT(updateSettings()));

    return configWidget;
}

void MarbleWallpaper::save(KConfigGroup &config)
{
    if (!m_map) {
        // When switching the background containment, this is called with m_map==0 for some reason
        return;
    }

    config.writeEntry(MAP_THEME_CONFIG_KEY, m_map->mapThemeId());
    config.writeEntry(MOVEMENT_CONFIG_KEY, static_cast<int>(m_movement));
    config.writeEntry(POSITION_DISTANCE_CONFIG_KEY, m_positionDist);
    config.writeEntry(POSITION_LAT_CONFIG_KEY, m_map->centerLatitude());
    config.writeEntry(POSITION_LON_CONFIG_KEY, m_map->centerLongitude());
    config.writeEntry(PROJECTION_CONFIG_KEY, static_cast<int>(m_projection));
    config.writeEntry(QUALITY_CONFIG_KEY, static_cast<int>(m_quality));
    config.writeEntry(ROTATION_LAT_CONFIG_KEY, m_rotationLat);
    config.writeEntry(ROTATION_LON_CONFIG_KEY, m_rotationLon);
    config.writeEntry(ROTATION_TIMEOUT_CONFIG_KEY, m_rotationTimeout);
    config.writeEntry(SHOW_PLACEMARKS_CONFIG_KEY, m_showPlacemarks);
}

void MarbleWallpaper::paint(QPainter *painter, const QRectF &exposedRect)
{
    // Update the pixmap
    if (m_pixmap.size() != boundingRect().size().toSize()) {
        m_pixmap = QPixmap(boundingRect().size().toSize());
    }
    if (m_pixmap.size().isEmpty()) {
        return;
    }
    m_map->setSize(m_pixmap.size());
    m_pixmap.fill(QColor(0x00, 0x00, 0x00, 0xFF));
    GeoPainter gp(&m_pixmap, m_map->viewParams()->viewport(), m_quality, true);
    QRect mapRect(0, 0, m_map->width(), m_map->height());
    m_map->paint(gp, mapRect);
    // Draw the requested part of the pixmap
    painter->drawPixmap(exposedRect, m_pixmap, exposedRect);
}

void MarbleWallpaper::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (m_movement == Interactive) {
        event->accept();
        m_map->zoomViewBy((int)(event->delta() / 3));
        m_positionDist = m_map->distance();
        emit update(boundingRect());
    }
}

void MarbleWallpaper::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_movement == Interactive && event->buttons() == Qt::LeftButton) {
        event->accept();
        int polarity = m_map->viewParams()->viewport()->polarity();

        qreal radius = (qreal)(m_map->radius());
        int deltaX = event->screenPos().x() - m_dragStartPositionX;
        int deltaY = event->screenPos().y() - m_dragStartPositionY;

        // Only start dragging after a certain distance
        if (abs(deltaX) <= DRAGTHRESHOLD && abs(deltaY) <= DRAGTHRESHOLD) {
            return;
        }

        qreal direction = 1;
        // Choose spin direction by taking into account whether we
        // drag above or below the visible pole.
        if (m_projection == Spherical) {
            if (polarity > 0) {
                if (event->screenPos().y() < (- m_map->northPoleY() + m_map->height() / 2))
                    direction = -1;
            }
            else {
                if (event->screenPos().y() > (+ m_map->northPoleY() + m_map->height() / 2))
                    direction = -1;
            }
        }
        m_positionLon = RAD2DEG * (qreal)(m_leftPressedTranslationX)
                        - 90.0 * direction * deltaX / radius;
        m_positionLat = RAD2DEG * (qreal)(m_leftPressedTranslationY)
                        + 90.0 * deltaY / radius;
        m_map->centerOn(m_positionLon, m_positionLat);
        emit update(boundingRect());
    }
}

void MarbleWallpaper::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_movement == Interactive && event->buttons() == Qt::LeftButton) {
        event->accept();

        // On the single event of a mouse button press these
        // values get stored, to enable us to e.g. calculate the
        // distance of a mouse drag while the mouse button is
        // still down.
        m_dragStartPositionX = event->screenPos().x();
        m_dragStartPositionY = event->screenPos().y();

        // Calculate translation of center point
        m_leftPressedTranslationX =  m_map->centerLongitude() * DEG2RAD;
        m_leftPressedTranslationY =  m_map->centerLatitude() * DEG2RAD;
    }
}

void MarbleWallpaper::widgetChanged()
{
    m_map->setProjection(m_projection);
    m_map->setShowCities(m_showPlacemarks);
    m_map->setShowOtherPlaces(m_showPlacemarks);
    m_map->setShowPlaces(m_showPlacemarks);

    if (m_rotationTimer) {
        m_rotationTimer->stop();
        disconnect(m_rotationTimer);
    }
    if (m_rotationTimeout && (m_rotationLon || m_rotationLat)) {
        if (!m_rotationTimer) {
            m_rotationTimer = new QTimer(this);
            connect(m_rotationTimer, SIGNAL(timeout()), SLOT(rotate()));
        }
        m_rotationTimer->setInterval(m_rotationTimeout);
        m_rotationTimer->start();
    }
    emit update(boundingRect());
}

void MarbleWallpaper::rotate()
{
    if (m_movement == FollowSun) {
        m_map->sunLocator()->update();
        if (m_map->sunLocator()->getLon() == m_map->centerLongitude()) {
            return;
        }
        m_positionLon = m_map->sunLocator()->getLon();
        m_positionLat = m_map->sunLocator()->getLat();
        m_map->centerOn(m_positionLon, m_positionLat);
        m_map->setDistance(m_positionDist);
    } else if (m_movement == Rotate) {
        m_map->rotateBy(m_rotationLon * m_rotationTimeout / 1000,
                        m_rotationLat * m_rotationTimeout / 1000);
    } else {
        return;
    }
    emit update(boundingRect());
}

void MarbleWallpaper::updateSettings()
{
    m_projection = static_cast<Projection>(m_ui.projection->currentIndex());
    m_rotationLon = m_ui.rotationLon->value();
    m_rotationLat = m_ui.rotationLat->value();
    // The first MapQuality value is wireframe, which we don't show in the list
    m_quality = static_cast<MapQuality>(m_ui.quality->currentIndex() + 1);
    m_showPlacemarks = m_ui.showPlacemarks->isChecked();
    m_rotationTimeout = m_ui.timeout->value() * 1000;

    widgetChanged();
}

void MarbleWallpaper::changeTheme(int index)
{
    m_mapTheme = m_ui.themeList->itemData(index).toString();
    m_map->setMapThemeId(m_mapTheme);
    emit update(boundingRect());
}

void MarbleWallpaper::updateConfigScreen(int index)
{
    m_movement = static_cast<Movement>(index);

    m_ui.mouseInstructions->setVisible(m_movement == Interactive);
    if (m_movement == Rotate) {
        m_ui.rotationLat->setVisible(true);
        m_ui.rotationLon->setVisible(true);
        m_ui.labelRotationLat->setVisible(true);
        m_ui.labelRotationLon->setVisible(true);
    } else {
        m_ui.rotationLat->setVisible(false);
        m_ui.rotationLon->setVisible(false);
        m_ui.labelRotationLat->setVisible(false);
        m_ui.labelRotationLon->setVisible(false);
    }
    if (m_movement == FollowSun || m_movement == Rotate) {
        m_ui.timeout->setVisible(true);
        m_ui.labelTimeout->setVisible(true);
    } else {
        m_ui.timeout->setVisible(false);
        m_ui.labelTimeout->setVisible(false);
    }
}

} // Marble namespace

#include "moc_marble.cpp"

// vim: sw=4 sts=4 et tw=100
