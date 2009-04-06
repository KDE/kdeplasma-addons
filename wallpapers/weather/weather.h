/***************************************************************************
 *   Copyright (C) 2008 by Jonathan Thomas                                 *
 *   echidnaman@kubuntu.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef WEATHER_H
#define WEATHER_H

#include <Plasma/DataEngine>
#include <Plasma/Wallpaper>
#include <plasma/weather/weatherutils.h>

#include "backgroundpackage.h"
#include "ui_weatherAddPlace.h"
#include "ui_weatherLocations.h"

class QStandardItemModel;
class QStandardItem;

class KDialog;

class WeatherWallpaper : public Plasma::Wallpaper
{
Q_OBJECT
public:
    WeatherWallpaper(QObject * parent, const QVariantList & args);
    ~WeatherWallpaper();

    QWidget * createConfigurationInterface(QWidget * parent);
    void paint(QPainter * painter, const QRectF & exposedRect);
    void validate(const QString& source, const QVariant& data);
    void weatherContent(const Plasma::DataEngine::Data &data);

public slots:
    void showAddPlaceDialog(const QStringList& tokens);
    void getValidation(void);
    void addPlace(void);
    void placeEditChanged(const QString& text);
    void pluginIndexChanged(int index);
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
    void selectPlace();
    void cancelAddClicked(void);
    void getWeather(void);
    void save(KConfigGroup & config);

protected slots:
    void updateBackground(const QImage &img);
    void updateFadedImage(qreal frame);
    void configWidgetDestroyed();

protected:
    void init(const KConfigGroup & config);
    void calculateGeometry();
    void renderWallpaper(const QString& image = QString());

private slots:
    void loadImage();

private:
    QWidget* m_configWidget;
    KDialog *m_addDialog;
    Ui::weatherLocations locationsUi;
    Ui::weatherAddPlace addUi;
    QStandardItemModel *m_amodel;
    QList<QStandardItem *> m_items;
    QString m_dir;

    QString m_currentSource; // Current source
    QString m_activeValidation; // Current place to validate
    QString m_activeIon; // Current data source plugin
    QString m_activePlace; // Current place we get weather from
    Plasma::DataEngine::Data m_currentData; // Current data returned from ion
    QMap<QString, QString> m_extraData; // Some ions pass back extra info to applet

    int m_weatherUpdateTime;

    Plasma::DataEngine *weatherEngine;
    Plasma::DataEngine::Data m_ionPlugins;

    Plasma::Wallpaper::ResizeMethod m_resizeMethod;
    QColor m_color;
    QString m_wallpaper;
    QPixmap m_pixmap;
    QPixmap m_oldPixmap;
    QPixmap m_oldFadedPixmap;
    QSize m_size;
    QString m_img;
};

K_EXPORT_PLASMA_WALLPAPER(weather, WeatherWallpaper)

#endif //PLASMA_PLUGIN_WALLPAPER_WEATHER_H
