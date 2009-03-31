/***************************************************************************
 *   Copyright (C) 2007-2008 by Shawn Starr <shawn.starr@rogers.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef WEATHERAPPLET_H
#define WEATHERAPPLET_H

#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>
#include <plasma/weather/weatherutils.h>

#include "ui_weatherLocations.h"
#include "ui_weatherUnits.h"
#include "ui_weatherAddPlace.h"

class QGraphicsGridLayout;
class QGraphicsLinearLayout;
class QStandardItem;
class QStandardItemModel;

class KDialog;

namespace Plasma
{
    class Frame;
    class IconWidget;
    class Label;
    class TabBar;
    class WeatherView;
}

class WeatherApplet : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    WeatherApplet(QObject *parent, const QVariantList &args);
    ~WeatherApplet();

    QGraphicsWidget *graphicsWidget();

public slots:
    void createConfigurationInterface(KConfigDialog *parent);
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

protected slots:
    void configAccepted();
    void toolTipAboutToShow();

protected:
    void constraintsEvent(Plasma::Constraints constraints);

private slots:
    void addPlace();
    void showAddPlaceDialog(const QStringList& tokens);
    void getValidation();
    void getWeather();
    void invokeBrowser(const QString& url);
    void selectPlace();
    void pluginIndexChanged(int index);
    void placeEditChanged(const QString& text);
    void cancelAddClicked();

private:
    Plasma::DataEngine *weatherEngine;
    KDialog *m_addDialog;
    Ui::weatherLocations ui;
    Ui::weatherUnits uui;
    Ui::weatherAddPlace aui;
    QStandardItemModel *m_amodel;
    QList<QStandardItem *> m_items;

    QString m_activeValidation; // Current place to validate
    QString m_activeIon; // Current data source plugin
    QString m_activePlace; // Current place we get weather from
    Plasma::DataEngine::Data m_currentData; // Current data returned from ion
    QMap<QString, QString> m_extraData; // Some ions pass back extra info to applet

    // Data stored by config
    int m_weatherWindFormat;
    int m_weatherTempFormat;
    int m_weatherPressureFormat;
    int m_weatherVisibilityFormat;
    int m_weatherUpdateTime;

    Plasma::DataEngine::Data m_ionPlugins;
    
    Plasma::Label *m_locationLabel;
    Plasma::Label *m_forecastTemps;
    Plasma::Label *m_conditionsLabel;
    Plasma::IconWidget *m_currentIcon;
    Plasma::Label *m_tempLabel;
    Plasma::IconWidget *m_windIcon;
    Plasma::Label *m_courtesyLabel;
    Plasma::TabBar *m_tabBar;

    QStandardItemModel *m_fiveDaysModel;
    QStandardItemModel *m_detailsModel;
    Plasma::WeatherView *m_fiveDaysView;
    Plasma::WeatherView *m_detailsView;

    // Layout
    int m_setupLayout;
    QGraphicsLinearLayout *m_layout;
    QGraphicsGridLayout *m_titlePanel;
    QGraphicsLinearLayout *m_bottomLayout;
    QGraphicsWidget *m_graphicsWidget;

    Plasma::Frame *m_titleFrame;

    void weatherContent(const Plasma::DataEngine::Data &data);
    void setVisible(bool visible, QGraphicsLayout *layout);
    void setVisibleLayout(bool val);
    void validate(const QString& source, const QVariant& data);
    QString convertTemperature(int format, QString value, int type, bool rounded, bool degreesOnly);
};

K_EXPORT_PLASMA_APPLET(weather, WeatherApplet)

#endif
