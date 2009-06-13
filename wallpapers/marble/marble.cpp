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

#define DRAG_THRESHOLD 3                        // Distance (pixel) before starting drag updates
#define DEFAULT_UPDATE_INTERVAL 300000          // Wait time between redraws in msecs (5 min)

#define MOVEMENT_KEY "movement"                 // Type of movement
#define ROTATION_LON_KEY "rotateLongitude"      // Amount of rotation (degrees/second)
#define ROTATION_LAT_KEY "rotateLatitude"
#define ROTATION_TIMEOUT_KEY "rotationTimeout"
#define POSITION_LON_KEY "positionLongitude"    // Last position
#define POSITION_LAT_KEY "positionLatitude"
#define ZOOM_KEY "zoom"                         // Zoom distance
#define MAP_THEME_KEY "mapTheme"                // Marble settings
#define PROJECTION_KEY "projection"             // Projection type
#define QUALITY_KEY "quality"                   // Render quality
#define SHOW_PLACEMARKS_KEY "showPlacemarks"

namespace Marble {

MarbleWallpaper::MarbleWallpaper(QObject * parent, const QVariantList & args)
    : Plasma::Wallpaper(parent, args), m_timer(0), m_map(0)
{
    KGlobal::locale()->insertCatalog("marble");
}

MarbleWallpaper::~MarbleWallpaper()
{
    delete m_map;
    delete m_timer;
}

void MarbleWallpaper::init(const KConfigGroup &config)
{
    qreal home_lon, home_lat;
    int home_zoom;

    // Only on first start, otherwise opening the config dialog lets us
    // lose the current position
    if (!isInitialized()) {
        m_map = new MarbleMap();

        // Get default position from marble to initialize on first startup (empty config)
        m_map->home(home_lon, home_lat, home_zoom);

        // These settings apply to Marble's "satellite" view mostly, e.g. make it beautiful
        //m_map->setShowClouds(true);   // Note: The cloud layers look ugly over America
        m_map->sunLocator()->setCitylights(true);
        m_map->sunLocator()->setShow(true);

        // Disable all render plugins (scale bar, compass, etc.) except the "stars" plugin
        foreach (RenderPlugin *item, m_map->renderPlugins()) {
            if (item->nameId() == "stars") {
                item->setVisible(true);
                item->setEnabled(true);
            } else {
                item->setVisible(false);
                item->setEnabled(false);
            }
        }
    }

    // Read setting values or use defaults
    m_mapTheme = config.readEntry(MAP_THEME_KEY, QString::fromLatin1("earth/bluemarble/bluemarble.dgml"));
    m_movement = static_cast<Movement>(config.readEntry(MOVEMENT_KEY, static_cast<int>(Interactive)));
    m_positionLon = config.readEntry(POSITION_LON_KEY, home_lon);
    m_positionLat = config.readEntry(POSITION_LAT_KEY, home_lat);
    m_projection = static_cast<Projection>(config.readEntry(PROJECTION_KEY, static_cast<int>(Spherical)));
    m_quality = static_cast<MapQuality>(config.readEntry(QUALITY_KEY, static_cast<int>(Normal)));
    m_rotationLat = config.readEntry(ROTATION_LAT_KEY, 0.0);
    m_rotationLon = config.readEntry(ROTATION_LON_KEY, 0.025);
    m_rotationTimeout = config.readEntry(ROTATION_TIMEOUT_KEY, 10000);
    m_showPlacemarks = config.readEntry(SHOW_PLACEMARKS_KEY, false);
    m_zoom = config.readEntry(ZOOM_KEY, home_zoom);

    m_map->setMapThemeId(m_mapTheme);
    m_map->setProjection(m_projection);
    m_map->setShowCities(m_showPlacemarks);
    m_map->setShowOtherPlaces(m_showPlacemarks);
    m_map->setShowPlaces(m_showPlacemarks);

    if (!isInitialized()) {
        m_map->zoomView(m_zoom);
        m_map->centerOn(m_positionLon, m_positionLat);
    }
    updateGlobe();
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
    m_ui.timeout->setValue(m_rotationTimeout / 1000);
    m_ui.showPlacemarks->setChecked(m_showPlacemarks);

    MapThemeManager themeManager;
    themeManager.updateMapThemeModel();
    // FIXME: Going manually through the model is ugly as hell, but plugging the
    //        model into the view didn't work for me
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
    connect(m_ui.movement, SIGNAL(currentIndexChanged(int)), SLOT(updateSettings()));
    connect(m_ui.projection, SIGNAL(currentIndexChanged(int)), SLOT(updateSettings()));
    connect(m_ui.quality, SIGNAL(currentIndexChanged(int)), SLOT(updateSettings()));
    connect(m_ui.rotationLon, SIGNAL(valueChanged(double)), SLOT(updateSettings()));
    connect(m_ui.rotationLat, SIGNAL(valueChanged(double)), SLOT(updateSettings()));
    connect(m_ui.timeout, SIGNAL(valueChanged(double)), SLOT(updateSettings()));
    connect(m_ui.showPlacemarks, SIGNAL(stateChanged(int)), SLOT(updateSettings()));
    connect(m_ui.themeList, SIGNAL(currentIndexChanged(int)), SLOT(changeTheme(int)));

    // Notify the plasma background dialog of changes
    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));

    return configWidget;
}

void MarbleWallpaper::save(KConfigGroup &config)
{
    if (m_map) {
        config.writeEntry(MAP_THEME_KEY, m_map->mapThemeId());
        config.writeEntry(POSITION_LAT_KEY, m_map->centerLatitude());
        config.writeEntry(POSITION_LON_KEY, m_map->centerLongitude());
    }

    config.writeEntry(MOVEMENT_KEY, static_cast<int>(m_movement));
    config.writeEntry(ZOOM_KEY, m_zoom);
    config.writeEntry(PROJECTION_KEY, static_cast<int>(m_projection));
    config.writeEntry(QUALITY_KEY, static_cast<int>(m_quality));
    config.writeEntry(ROTATION_LAT_KEY, m_rotationLat);
    config.writeEntry(ROTATION_LON_KEY, m_rotationLon);
    config.writeEntry(ROTATION_TIMEOUT_KEY, m_rotationTimeout);
    config.writeEntry(SHOW_PLACEMARKS_KEY, m_showPlacemarks);
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
    painter->drawPixmap(exposedRect, m_pixmap, exposedRect.translated(-boundingRect().topLeft()));
}

void MarbleWallpaper::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (m_movement == Interactive) {
        event->accept();
        m_map->zoomViewBy(event->delta() > 0 ? 10 : -10);
        m_zoom = m_map->zoom();

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
        if (abs(deltaX) <= DRAG_THRESHOLD && abs(deltaY) <= DRAG_THRESHOLD) {
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

void MarbleWallpaper::updateGlobe()
{
    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), SLOT(updateGlobe()));
    } else {
        m_timer->stop();
    }

    if (m_movement == Rotate || m_movement == FollowSun) {
        m_timer->setInterval(m_rotationTimeout);
    } else {
        m_timer->setInterval(DEFAULT_UPDATE_INTERVAL);
    }
    m_timer->start();

    if (m_movement == FollowSun) {
        m_map->sunLocator()->update();
        if (m_map->sunLocator()->getLon() == m_map->centerLongitude()) {
            return;
        }
        m_positionLon = m_map->sunLocator()->getLon();
        m_positionLat = m_map->sunLocator()->getLat();
        m_map->centerOn(m_positionLon, m_positionLat);
    } else if (m_movement == Rotate) {
        m_map->rotateBy(m_rotationLon * m_rotationTimeout / 1000,
                        m_rotationLat * m_rotationTimeout / 1000);
        m_positionLon = m_map->centerLongitude();
        m_positionLat = m_map->centerLatitude();
    }
    emit update(boundingRect());
}

void MarbleWallpaper::updateSettings()
{
    m_projection = static_cast<Projection>(m_ui.projection->currentIndex());
    m_rotationLon = m_ui.rotationLon->value();
    m_rotationLat = m_ui.rotationLat->value();
    m_rotationTimeout = m_ui.timeout->value() * 1000;
    // The first MapQuality value is wireframe, which we don't show in the list
    m_quality = static_cast<MapQuality>(m_ui.quality->currentIndex() + 1);
    m_showPlacemarks = m_ui.showPlacemarks->isChecked();

    emit settingsChanged(true);
}

void MarbleWallpaper::changeTheme(int index)
{
    m_mapTheme = m_ui.themeList->itemData(index).toString();
    m_map->setMapThemeId(m_mapTheme);

    emit update(boundingRect());
    emit settingsChanged(true);
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

    emit settingsChanged(true);
}

} // Marble namespace

#include "moc_marble.cpp"

// vim: sw=4 sts=4 et tw=100
