/*
 * Copyright 2008-2009  Petri Damstén <damu@iki.fi>
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WEATHERSTATION_HEADER
#define WEATHERSTATION_HEADER

#include <KUnitConversion/Value>

#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>

#include <plasmaweather/weatherpopupapplet.h>

#include "ui_appearanceconfig.h"

class LCD;

namespace Plasma {
    class DeclarativeWidget;
}

namespace Conversion {
    class Value;
}

class WeatherStation : public WeatherPopupApplet
{
    Q_OBJECT
    Q_PROPERTY(bool useBackground READ useBackground WRITE setUseBackground NOTIFY useBackgroundChanged)

public:
    WeatherStation(QObject *parent, const QVariantList &args);
    ~WeatherStation();

    virtual void init();
    QGraphicsWidget* graphicsWidget();
    virtual void createConfigurationInterface(KConfigDialog *parent);

    bool useBackground() const;
    void setUseBackground(bool use);

public slots:
    virtual void configAccepted();
    virtual void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
    void clicked();
    void configChanged();

signals:
    void useBackgroundChanged();
    void temperatureChanged(QString temperature, QString unit);
    void humidityChanged(QString humidity);
    void providerLabelChanged(QString label);
    void weatherLabelChanged(QString label);
    void conditionChanged();
    void pressureChanged(QString conditionId, QString pressure, QString unit, QString direction);
    void windChanged(QString direction, QString speed, QString unit);

protected:
    void setWind(const KUnitConversion::Value& speed, const QString& direction);
    void setPressure(const QString& condition, const KUnitConversion::Value& pressure,
                     const QString& tendency);
    void setTemperature(const KUnitConversion::Value& temperature, bool hasDigit);
    void setHumidity(QString humidity);
    void setToolTip(const QString& place);

    QString fitValue(const KUnitConversion::Value& value, int digits);
    QString fromCondition(const QString& condition);
    KUnitConversion::Value value(const QString& value, int unit);

private:
    void setLCDIcon();

    Plasma::DeclarativeWidget *m_declarativeWidget;
    LCD *m_lcdPanel;

    Ui::AppearanceConfig m_appearanceConfig;
    bool m_useBackground;
    bool m_showToolTip;
    QString m_url;
};

K_EXPORT_PLASMA_APPLET(weatherstation, WeatherStation)

#endif
