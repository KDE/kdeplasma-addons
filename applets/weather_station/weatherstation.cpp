/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
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

#include "weatherstation.h"
#include "lcd.h"
#include "weatherconfig.h"
#include <plasma/containment.h>
#include <plasma/theme.h>
#include <KConfigDialog>
#include <KConfigGroup>
#include <QGraphicsLinearLayout>

WeatherStation::WeatherStation(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_layout(0), m_lcd(0)
{
    setHasConfigurationInterface(true);
    resize(250, 350);
}

WeatherStation::~WeatherStation()
{
}

void WeatherStation::init()
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    m_lcd = new LCD(this);
    m_lcd->setSvg("weatherstation/lcd");
    m_layout->addItem(m_lcd);
    setLayout(m_layout);

    //m_lcd->setItemOn("under_construction");

    KConfigGroup cfg = config();
    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        c.weatherWindFormat = cfg.readEntry("windFormat", (int)ConfigData::Kilometers);
    } else {
        c.weatherWindFormat = cfg.readEntry("windFormat", (int)ConfigData::Miles);
    }
    c.weatherUpdateTime = cfg.readEntry("updateWeather", 30);
    c.weatherEngine = dataEngine("weather");
    c.ionPlugins = c.weatherEngine->query("ions");
    ConfigData::PlaceInfo placeInfo;
    foreach(const QString& place, cfg.groupList()) {
        KConfigGroup placeConfig(&cfg, place);
        placeInfo.place = place;
        placeInfo.ion = placeConfig.readEntry("ion");
        c.extraData[place] = placeConfig.readEntry("data");
        c.placeList[place] = placeInfo;
        //kDebug() << place << placeInfo.ion << placeConfig.readEntry("data");
    }

    constraintsUpdated(Plasma::FormFactorConstraint);
    connectToEngine();
}

void WeatherStation::connectToEngine()
{
    if (c.placeList.isEmpty()) {
        setConfigurationRequired(true);
    } else {
        m_lcd->setNumber("temperature", "Load");
        m_lcd->setNumber("humidity", "ing");

        // we support only one place
        const ConfigData::PlaceInfo& info = c.placeList[c.placeList.keys()[0]];
        //kDebug() << c.placeList.keys();
        QString source;
        if (c.extraData[info.place].isEmpty()) {
            source = QString("%1|weather|%2").arg(info.ion).arg(info.place);
        } else {
            source = QString("%1|weather|%2|%3")
                    .arg(info.ion).arg(info.place).arg(c.extraData[info.place]);
        }
        //kDebug() << source;
        c.weatherEngine->connectSource(source, this, c.weatherUpdateTime * 60 * 1000);
    }
}

void WeatherStation::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    setPressure(data["Pressure"].toString(), data["Pressure Unit"].toInt(),
                data["Pressure Tendency"].toString());
    setTemperature(data["Temperature"].toString(), data["Temperature Unit"].toInt());
    setHumidity(data["Humidity"].toString());
    setWind(data["Wind Speed"].toString(), data["Wind Speed Unit"].toInt(),
            data["Wind Direction"].toString());
}

void WeatherStation::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        switch (formFactor()) {
        case Plasma::Planar:
        case Plasma::MediaCenter:
            setAspectRatioMode(Plasma::IgnoreAspectRatio);
            break;
        case Plasma::Horizontal:
        case Plasma::Vertical:
            setAspectRatioMode(Plasma::Square);
            break;
        }
    }
}

void WeatherStation::createConfigurationInterface(KConfigDialog *parent)
{
    m_weatherConfig = new WeatherConfig(parent);
    m_weatherConfig->setData(c);
    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    parent->addPage(m_weatherConfig, i18n("Weather"), "browser");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void WeatherStation::configAccepted()
{
    setConfigurationRequired(false);
    foreach(const ConfigData::PlaceInfo& info, c.placeList) {
        c.weatherEngine->disconnectSource(
                QString("%1|weather|%2").arg(info.ion).arg(info.place), this);
    }

    KConfigGroup cfg = config();

    foreach(const QString& place, cfg.groupList()) {
        KConfigGroup placeConfig(&cfg, place);
        placeConfig.deleteGroup();
    }

    m_weatherConfig->getData(&c);
    cfg.writeEntry("updateWeather", c.weatherUpdateTime);
    cfg.writeEntry("windFormat", c.weatherWindFormat);

    foreach(const ConfigData::PlaceInfo& info, c.placeList) {
        KConfigGroup placeConfig(&cfg, info.place);
        placeConfig.writeEntry("ion", info.ion);
        if (!c.extraData[info.place].isEmpty()) {
            placeConfig.writeEntry("data", c.extraData[info.place]);
        }
    }
    emit configNeedsSaving();
    connectToEngine();
}

void WeatherStation::setPressure(const QString& pressure, int unit,
                                 const QString& tendency)
{
    // TODO Convert "Current Conditions" to picture instead of using pressure
    // It seems that those are just strings that come from weather source so
    // it might be hard to convert them to a picture.
    qreal p = pressure.toDouble();
    qreal t = tendency.toDouble();
    QStringList current;

    if (unit == WeatherUtils::Inches) {
        p = WeatherUtils::convert(p, WeatherUtils::Inches, WeatherUtils::Kilopascals);
        t = WeatherUtils::convert(t, WeatherUtils::Inches, WeatherUtils::Kilopascals);
    }
    if (unit == WeatherUtils::Hectopascals) {
        p /= 10.0;
        t /= 10.0;
    }
    //kDebug() << p << t;
    p += t * 10; // This is completely unscientific so if anyone have a better formula for this :-)

    if (p > 103.0) {
        current << "sun";
    } else if (p > 100.0) {
        current << "change";
    } else if (p > 99.0) {
        current << "rain";
    } else {
        current << "rain" << "heavy_rain";
    }
    m_lcd->setGroup("weather", current);
    m_lcd->setNumber("pressure", pressure);
    m_lcd->setGroup("pressure_unit", QStringList() << WeatherUtils::getUnitString(unit, true));
}

void WeatherStation::setTemperature(const QString& temperature, int unit)
{
    m_lcd->setGroup("temp_unit", QStringList() << WeatherUtils::getUnitString(unit, true));
    m_lcd->setNumber("temperature", temperature);
}

void WeatherStation::setHumidity(QString humidity)
{
    humidity.replace("%", "");
    m_lcd->setNumber("humidity", humidity);
}

void WeatherStation::setWind(const QString& speed, int fromUnit, const QString& dir)
{
    QString direction(dir);
    bool dirInDegrees = false;
    int dirDegrees = direction.toInt(&dirInDegrees);

    if (direction == "N/A") {
        m_lcd->setGroup("wind", m_lcd->groupItems("wind"));
    } else if(dirInDegrees) {
        direction = WeatherUtils::degreesToCardinal(dirDegrees);
        if(!direction.isEmpty()) {
            m_lcd->setGroup("wind", QStringList() << direction);
        }
    } else {
        m_lcd->setGroup("wind", QStringList() << direction);
    }

    int toUnit;
    switch (c.weatherWindFormat) {
        case ConfigData::Kilometers:
            toUnit = WeatherUtils::Kilometers;
            break;
        case ConfigData::Miles:
            toUnit = WeatherUtils::Miles;
            break;
        case ConfigData::Knots:
            toUnit = WeatherUtils::Knots;
            break;
        case ConfigData::Beaufort:
            toUnit = WeatherUtils::Beaufort;
            break;
        case ConfigData::MetersPerSecond:
        default:
            toUnit = WeatherUtils::MetersPerSecond;
            break;
    }

    double windSpeed;
    if(fromUnit != toUnit)
        windSpeed = WeatherUtils::convert(speed.toDouble(), fromUnit, toUnit);
    else
        windSpeed = speed.toDouble();

    QString unit(WeatherUtils::getUnitString(toUnit, true));
    m_lcd->setNumber("wind_speed", QString::number(windSpeed, 'f', 1));
    m_lcd->setGroup("wind_unit", QStringList() << unit);
}

#include "weatherstation.moc"
