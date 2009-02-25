/*
 * Copyright 2008-2009  Petri Damstén <damu@iki.fi>
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
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KRun>
#include <Plasma/Containment>
#include <Plasma/Theme>
#include <plasma/weather/weatherutils.h>
#include <conversion/converter.h>
#include <math.h>
#include "lcd.h"
#include "weatherconfig.h"

WeatherStation::WeatherStation(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args), m_lcd(0), m_lcdPanel(0)
{
    setHasConfigurationInterface(true);
    resize(250, 350);
}

WeatherStation::~WeatherStation()
{
}

void WeatherStation::init()
{
    m_lcd = new LCD(this);
    m_lcd->setSvg("weatherstation/lcd");
    // So we don't show in panel
    m_lcd->setMinimumSize(m_lcd->preferredSize() / 2);
    connect(m_lcd, SIGNAL(clicked(const QString&)), this, SLOT(clicked(const QString&)));

    m_lcdPanel = new LCD(this);
    m_lcdPanel->setSvg("weatherstation/lcd_panel");
    m_lcdPanel->hide();

    //m_lcd->setItemOn("under_construction");

    KConfigGroup cfg = config();

    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        m_temperatureUnit = cfg.readEntry("temperatureUnit", "C");
        m_speedUnit = cfg.readEntry("speedUnit", "ms");
        m_pressureUnit = cfg.readEntry("pressureUnit", "hPa");
    } else {
        m_temperatureUnit = cfg.readEntry("temperatureUnit", "F");
        m_speedUnit = cfg.readEntry("speedUnit", "mph");
        m_pressureUnit = cfg.readEntry("pressureUnit", "mb");
    }
    m_updateInterval = cfg.readEntry("updateWeather", 30);
    m_source = cfg.readEntry("source", "");
    m_useBackground = cfg.readEntry("background", true);
    setBackground();

    m_weatherEngine = dataEngine("weather");
    setLCDIcon();
    connectToEngine();
}

void WeatherStation::connectToEngine()
{
    if (m_source.isEmpty()) {
        setConfigurationRequired(true);
    } else {
        m_lcd->setNumber("temperature", "Load");
        m_lcd->setNumber("humidity", "ing");
        //kDebug() << m_source;
        m_weatherEngine->connectSource(m_source, this, m_updateInterval * 60 * 1000);
    }
}

QGraphicsWidget* WeatherStation::graphicsWidget()
{
    return m_lcd;
}

void WeatherStation::setBackground()
{
    m_lcd->clear();
    if (m_useBackground) {
        m_lcd->setItemOn("lcd_background");
    }
    m_lcd->setItemOn("background");
    m_lcdPanel->clear();
    if (m_useBackground) {
        m_lcdPanel->setItemOn("lcd_background");
    }
    m_lcdPanel->setItemOn("background");
}

void WeatherStation::createConfigurationInterface(KConfigDialog *parent)
{
    m_weatherConfig = new WeatherConfig(parent);
    m_weatherConfig->setDataEngine(m_weatherEngine);
    m_weatherConfig->setSource(m_source);
    m_weatherConfig->setUpdateInterval(m_updateInterval);
    m_weatherConfig->setTemperatureUnit(m_temperatureUnit);
    m_weatherConfig->setSpeedUnit(m_speedUnit);
    m_weatherConfig->setPressureUnit(m_pressureUnit);
    m_weatherConfig->setBackground(m_useBackground);
    parent->addPage(m_weatherConfig, i18n("Weather"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void WeatherStation::configAccepted()
{
    setConfigurationRequired(false);
    if (!m_source.isEmpty()) {
        m_weatherEngine->disconnectSource(m_source, this);
    }

    KConfigGroup cfg = config();
    cfg.writeEntry("temperatureUnit", m_temperatureUnit = m_weatherConfig->temperatureUnit());
    cfg.writeEntry("speedUnit", m_speedUnit = m_weatherConfig->speedUnit());
    cfg.writeEntry("pressureUnit", m_pressureUnit = m_weatherConfig->pressureUnit());
    cfg.writeEntry("updateInterval", m_updateInterval = m_weatherConfig->updateInterval());
    cfg.writeEntry("source", m_source = m_weatherConfig->source());
    cfg.writeEntry("background", m_useBackground = m_weatherConfig->background());
    setBackground();

    emit configNeedsSaving();
    connectToEngine();
}

void WeatherStation::setLCDIcon()
{
    if (m_lcdPanel->size().toSize() != size().toSize()) {
        m_lcdPanel->resize(size());
    }
    setPopupIcon(QIcon(m_lcdPanel->toPixmap()));
}

void WeatherStation::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    //kDebug() << source << data;
    Q_UNUSED(source);
    if (data.isEmpty()) {
        return;
    }
    Conversion::Value temp = Conversion::Value(data["Temperature"],
                             WeatherUtils::getUnitString(data["Temperature Unit"].toInt(), true));
    setTemperature(temp);
    setPressure(data["Condition Icon"].toString(),
                Conversion::Value(data["Pressure"],
                    WeatherUtils::getUnitString(data["Pressure Unit"].toInt())),
                data["Pressure Tendency"].toString(), temp);
    setHumidity(data["Humidity"].toString());
    setWind(Conversion::Value(data["Wind Speed"],
            WeatherUtils::getUnitString(data["Wind Speed Unit"].toInt(), true)),
            data["Wind Direction"].toString());
    m_lcd->setLabel("label0", data["Credit"].toString());
    m_url = data["Credit Url"].toString();
    m_lcd->setItemClickable("label0", !m_url.isEmpty());
}

QString WeatherStation::fitValue(const Conversion::Value& value, int digits)
{
    double v = value.number();
    int mainDigits = (int)floor(log(fabs(v))) + 1;
    int precision = 0;

    if (mainDigits < 3 && mainDigits + (v < 0)?1:0 + 1 < digits) {
        precision = 1;
    }
    return QString::number(v, 'f', precision);
}

QStringList WeatherStation::fromCondition(const QString& condition)
{
    QStringList result;

    if (condition == "weather-clear-night") {
        result << "moon";
    } else if (condition == "weather-clear") {
        result << "sun";
    } else if (condition == "weather-clouds-night" ||
               condition == "weather-few-clouds-night") {
        result << "half_moon" << "lower_cloud";
    } else if (condition == "weather-clouds" ||
               condition == "weather-few-clouds") {
        result << "half_sun" << "lower_cloud";
    } else if (condition == "weather-hail") {
        result << "cloud" << "cloud_flash_hole" << "hail";
    } else if (condition == "weather-many-clouds") {
        result << "cloud" << "cloud_flash_hole";
    } else if (condition == "weather-mist") {
        result << "cloud" << "cloud_flash_hole";
    } else if (condition == "weather-showers-day" ||
               condition == "weather-showers-night") {
        result << "cloud" << "cloud_flash_hole" << "water_drop";
    } else if (condition == "weather-showers") {
        result << "cloud" << "cloud_flash_hole" << "water_drop" << "water_drops";
    } else if (condition == "weather-showers-scattered-day" ||
               condition == "weather-showers-scattered-night" ||
               condition == "weather-showers-scattered") {
        result << "cloud" << "cloud_flash_hole" << "water_drops";
    } else if (condition == "weather-snow") {
        result << "cloud" << "cloud_flash_hole" << "snow_flake" << "snow_flakes";
    } else if (condition == "weather-snow-rain") {
        result << "cloud" << "cloud_flash_hole" << "snow_flake" << "water_drops";
    } else if (condition == "weather-snow-scattered-day" ||
               condition == "weather-snow-scattered-night" ||
               condition == "weather-snow-scattered") {
        result << "cloud" << "cloud_flash_hole" << "snow_flakes";
    } else if (condition == "weather-storm-day") {
    } else if (condition == "weather-storm-night") {
    } else if (condition == "weather-storm") {
        result << "cloud" << "flash" << "water_drops";
    }
    //kDebug() << result;
    return result;
}

QStringList WeatherStation::fromPressure(const Conversion::Value& pressure, const QString& tendency,
                                         const Conversion::Value& temperature)
{
    QStringList result;
    qreal temp = Conversion::Converter::self()->convert(temperature, "C").number();
    qreal p = Conversion::Converter::self()->convert(pressure, "kPa").number();
    qreal t;

    if (tendency.toLower() == "rising") {
        t = 0.75;
    } else if (tendency.toLower() == "falling") {
        t = -0.75;
    } else {
        t = Conversion::Converter::self()->convert(
                Conversion::Value(tendency.toDouble(), pressure.unit()), "kPa").number();
    }
    p += t * 10; // This is completely unscientific so if anyone have a better formula for this :-)

    // Moon 22:00 -> 6:00
    QString sunOrMoon = (((QDateTime::currentDateTime().time().hour() + 2) % 24) <= 8) ?
                        "moon" : "sun";
    if (p > 103.0) {
        result << sunOrMoon;
    } else if (p > 100.0) {
        result << "half_" + sunOrMoon << "lower_cloud";
    } else if (p > 99.0) {
        result << "cloud" << "cloud_flash_hole";
        if (temp > 1.0) {
            result << "water_drop";
        } else if (temp < -1.0)  {
            result << "snow_flake";
        } else {
            result << "water_drop" << "snow_flakes";
        }
    } else {
        result << "cloud" << "cloud_flash_hole";
        if (temp > 1.0) {
            result << "water_drop" << "water_drops";
        } else if (temp < -1.0)  {
            result<< "snow_flake" << "snow_flakes";
        } else {
            result<< "water_drop" << "snow_flakes";
        }
    }
    //kDebug() << result;
    return result;
}

void WeatherStation::setPressure(const QString& condition, const Conversion::Value& pressure,
                                 const QString& tendency, const Conversion::Value& temperature)
{
    QStringList current;
    if (!condition.isEmpty() && condition != "weather-none-available") {
        current = fromCondition(condition);
    }
    if (current.size() == 0) {
        current = fromPressure(pressure, tendency, temperature);
    }
    m_lcd->setGroup("weather", current);

    QString s = fitValue(Conversion::Converter::self()->convert(pressure, m_pressureUnit), 5);
    m_lcd->setNumber("pressure", s);
    m_lcd->setGroup("pressure_unit", QStringList() << m_pressureUnit);
}

void WeatherStation::setTemperature(const Conversion::Value& temperature)
{
    Conversion::Value v = Conversion::Converter::self()->convert(temperature, m_temperatureUnit);
    m_lcd->setGroup("temp_unit", QStringList() << m_temperatureUnit);
    m_lcdPanel->setGroup("temp_unit", QStringList() << m_temperatureUnit);
    m_lcd->setNumber("temperature", fitValue(v , 4));
    m_lcdPanel->setNumber("temperature", fitValue(v , 3));
    setLCDIcon();
}

void WeatherStation::setHumidity(QString humidity)
{
    humidity.remove('%');
    m_lcd->setNumber("humidity", humidity);
}

void WeatherStation::setWind(const Conversion::Value& speed, const QString& dir)
{
    //kDebug() << speed.number() << speed.unit()->symbol() << dir;
    QString s = fitValue(Conversion::Converter::self()->convert(speed, m_speedUnit), 3);

    if (dir == "N/A") {
        m_lcd->setGroup("wind", m_lcd->groupItems("wind"));
    } else {
        m_lcd->setGroup("wind", QStringList() << dir);
    }

    m_lcd->setNumber("wind_speed", s);
    m_lcd->setGroup("wind_unit", QStringList() << m_speedUnit);
}

void WeatherStation::clicked(const QString &name)
{
    Q_UNUSED(name)
    KRun::runUrl(KUrl(m_url), "text/html", 0);
}

#include "weatherstation.moc"
