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

#include "ui_weatherAdvanced.h"

class QPropertyAnimation;
class QStandardItemModel;

class KFileDialog;

namespace KNS3 {
    class DownloadDialog;
}

class BackgroundListModel;
class WeatherConfig;
class WeatherLocation;

class WeatherWallpaper : public Plasma::Wallpaper
{
Q_OBJECT
Q_PROPERTY(qreal fadeValue READ fadeValue WRITE setFadeValue)

  public:
    WeatherWallpaper(QObject * parent, const QVariantList & args);
    ~WeatherWallpaper();

    QWidget * createConfigurationInterface(QWidget * parent);
    void paint(QPainter * painter, const QRectF & exposedRect);
    void updateScreenshot(QPersistentModelIndex index);
    qreal fadeValue();

signals:
    void settingsChanged(bool changed = true);

public slots:
    void showAdvancedDialog();
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
    void connectWeatherSource(void);
    void save(KConfigGroup & config);

protected slots:
    void getNewWallpaper();
    void colorChanged(const QColor& color);
    void pictureChanged(int index);
    void conditionChanged(int index);
    void positioningChanged(int index);
    void fileDialogFinished();
    void wallpaperBrowseCompleted();
    void updateBackground(const QImage &img);
    void showFileDialog();
    void setFadeValue(qreal value);
    void configWidgetDestroyed();
    void advancedDialogDestroyed();
    void locationReady(const QString &source);
    void newStuffFinished();

protected:
    void init(const KConfigGroup & config);
    void fillMetaInfo(Plasma::Package *b);
    bool setMetadata(QLabel *label, const QString &text);
    void calculateGeometry();
    void renderWallpaper(const QString& image = QString());

private slots:
    void loadImage();

private:
    WeatherConfig* m_configWidget;
    WeatherLocation* m_weatherLocation;
    KDialog *m_advancedDialog;
    Ui::weatherAdvanced m_advancedUi;
    QString m_dir;
    QStringList m_usersWallpapers;

    QString m_source; // Current source
    QString m_condition; // Current condition

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
    KFileDialog *m_fileDialog;
    qreal m_fadeValue;
    QPropertyAnimation *m_animation;
    BackgroundListModel *m_model;
    QSize m_size;
    QString m_img;
    KNS3::DownloadDialog* m_newStuffDialog;
};

#endif //PLASMA_PLUGIN_WALLPAPER_WEATHER_H
