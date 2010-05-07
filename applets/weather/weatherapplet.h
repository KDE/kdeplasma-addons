/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
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
#include <plasmaweather/weatherpopupapplet.h>

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

class BackgroundWidget;

class WeatherApplet : public WeatherPopupApplet
{
    Q_OBJECT
public:
    WeatherApplet(QObject *parent, const QVariantList &args);
    ~WeatherApplet();

    void init();
    QGraphicsWidget *graphicsWidget();

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
    void reloadTheme(void);

protected Q_SLOTS:
    void configAccepted();
    void toolTipAboutToShow();
    void clearCurrentWeatherIcon();

protected:
    void constraintsEvent(Plasma::Constraints constraints);
    void resizeView(void);

private Q_SLOTS:
    void invokeBrowser(const QString& url) const;

private:
    Plasma::DataEngine::Data m_currentData; // Current data returned from ion

    Plasma::Label *m_locationLabel;
    Plasma::Label *m_forecastTemps;
    Plasma::Label *m_conditionsLabel;
    Plasma::Label *m_tempLabel;
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
    BackgroundWidget *m_graphicsWidget;

    Plasma::Frame *m_titleFrame;
    QFont m_titleFont;

    bool isValidData(const QVariant &data) const;
    void weatherContent(const Plasma::DataEngine::Data &data);
    void setVisible(bool visible, QGraphicsLayout *layout);
    void setVisibleLayout(bool val);
    QString convertTemperature(KUnitConversion::UnitPtr format, QString value,
                               int type, bool rounded, bool degreesOnly);
};

K_EXPORT_PLASMA_APPLET(weather, WeatherApplet)

#endif
