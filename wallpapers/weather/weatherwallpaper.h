/***************************************************************************
 *   Copyright (C) 2009 by Jonathan Thomas <echidnaman@kubuntu.org>        *
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef WEATHERWALLPAPER_H
#define WEATHERWALLPAPER_H

#include <Plasma/DataEngine>
#include <Plasma/Package>
#include <Plasma/Wallpaper>
#include <plasma/weather/weatherutils.h>

#include "ui_weatherAddPlace.h"
#include "ui_weatherAdvanced.h"
#include "ui_weatherLocations.h"

class QStandardItemModel;
class QStandardItem;

class KDialog;
class KFileDialog;

class BackgroundListModel;

class WeatherWallpaper : public Plasma::Wallpaper
{
Q_OBJECT
public:
    WeatherWallpaper(QObject * parent, const QVariantList & args);
    ~WeatherWallpaper();

    QWidget * createConfigurationInterface(QWidget * parent);
    void paint(QPainter * painter, const QRectF & exposedRect);
    void updateScreenshot(QPersistentModelIndex index);
    void validate(const QString& source, const QVariant& data);
    void weatherContent(const Plasma::DataEngine::Data &data);

public slots:
    void showAddPlaceDialog(const QStringList& tokens);
    void showAdvancedDialog();
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
    void spinValueChanged(int interval);
    void getNewWallpaper();
    void colorChanged(const QColor& color);
    void pictureChanged(int index);
    void conditionChanged(int index);
    void positioningChanged(int index);
    void browse();
    void updateBackground(const QImage &img);
    void showFileDialog();
    void removeBackground(const QString &path);
    void updateFadedImage(qreal frame);
    void configWidgetDestroyed();

protected:
    void init(const KConfigGroup & config);
    void fillMetaInfo(Plasma::Package *b);
    bool setMetadata(QLabel *label, const QString &text);
    void calculateGeometry();
    void renderWallpaper(const QString& image = QString());

private slots:
    void loadImage();

private:
    QWidget* m_configWidget;
    KDialog *m_addDialog;
    KDialog *m_advancedDialog;
    Ui::weatherLocations locationsUi;
    Ui::weatherAddPlace addUi;
    Ui::weatherAdvanced m_advancedUi;
    QStandardItemModel *m_amodel;
    QList<QStandardItem *> m_items;
    QString m_dir;
    QStringList m_usersWallpapers;

    QString m_currentSource; // Current source
    QString m_activeValidation; // Current place to validate
    QString m_activeIon; // Current data source plugin
    QString m_activePlace; // Current place we get weather from
    Plasma::DataEngine::Data m_currentData; // Current data returned from ion
    QMap<QString, QString> m_extraData; // Some ions pass back extra info to applet

    int m_weatherUpdateTime;

    QHash<QString,QString> m_weatherMap;

    Plasma::DataEngine *weatherEngine;
    Plasma::DataEngine::Data m_ionPlugins;

    Plasma::Wallpaper::ResizeMethod m_resizeMethod;
    QColor m_color;
    QString m_wallpaper;
    QPixmap m_pixmap;
    QPixmap m_oldPixmap;
    QPixmap m_oldFadedPixmap;
    BackgroundListModel *m_model;
    KFileDialog *m_fileDialog;
    QSize m_size;
    QString m_img;
};

#endif //PLASMA_PLUGIN_WALLPAPER_WEATHER_H
