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
#include <KToolInvocation>
#include <Plasma/Containment>
#include <Plasma/Theme>
#include <plasma/weather/weatherutils.h>
#include <conversion/converter.h>
#include <plasmaweather/weatherconfig.h>
#include <math.h>
#include "lcd.h"

WeatherStation::WeatherStation(QObject *parent, const QVariantList &args)
    : WeatherPopupApplet(parent, args), m_lcd(0), m_lcdPanel(0)
{
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

    m_useBackground = cfg.readEntry("background", true);
    setBackground();

    setLCDIcon();
    
    WeatherPopupApplet::init();
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
    WeatherPopupApplet::createConfigurationInterface(parent);
    WeatherConfig* wc = weatherConfig();
    wc->setConfigurableUnits(WeatherConfig::Temperature | WeatherConfig::Speed |
                             WeatherConfig::Pressure);
    
    QWidget *w = new QWidget();
    m_appearanceConfig.setupUi(w);
    m_appearanceConfig.backgroundCheckBox->setChecked(m_useBackground);
    parent->addPage(w, i18n("Appearance"), icon());
}

void WeatherStation::configAccepted()
{
    KConfigGroup cfg = config();
    cfg.writeEntry("background", m_useBackground =
            m_appearanceConfig.backgroundCheckBox->isChecked());
    setBackground();

    WeatherPopupApplet::configAccepted();
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
    WeatherPopupApplet::dataUpdated(source, data);

    if (data.contains("Credit Url")) {
        Conversion::Value temp = Conversion::Value(data["Temperature"],
                WeatherUtils::getUnitString(data["Temperature Unit"].toInt(), true));
        setTemperature(temp);
        setPressure(conditionIcon(),
                    Conversion::Value(data["Pressure"],
                        WeatherUtils::getUnitString(data["Pressure Unit"].toInt())),
                    data["Pressure Tendency"].toString());
        setHumidity(data["Humidity"].toString());
        setWind(Conversion::Value(data["Wind Speed"],
                WeatherUtils::getUnitString(data["Wind Speed Unit"].toInt(), true)),
                data["Wind Direction"].toString());
        m_lcd->setLabel("label0", data["Credit"].toString());
        m_url = data["Credit Url"].toString();
        m_lcd->setItemClickable("label0", !m_url.isEmpty());
    }
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

void WeatherStation::setPressure(const QString& condition, const Conversion::Value& pressure,
                                 const QString& tendencyString)
{
    QStringList current;
    current = fromCondition(condition);
    m_lcd->setGroup("weather", current);

    QString s = fitValue(Conversion::Converter::self()->convert(pressure, pressureUnit()), 5);
    m_lcd->setNumber("pressure", s);
    m_lcd->setGroup("pressure_unit", QStringList() << pressureUnit());

    qreal t;
    if (tendencyString.toLower() == "rising") {
        t = 1.0;
    } else if (tendencyString.toLower() == "falling") {
        t = -1.0;
    } else {
        t = tendencyString.toDouble();
    }

    if (t > 0.0) {
        m_lcd->setGroup("pressure_direction", QStringList() << "up");
    } else if (t < 0.0) {
        m_lcd->setGroup("pressure_direction", QStringList() << "down");
    } else {
        m_lcd->setGroup("pressure_direction", QStringList());
    }
}

void WeatherStation::setTemperature(const Conversion::Value& temperature)
{
    Conversion::Value v = Conversion::Converter::self()->convert(temperature, temperatureUnit());
    m_lcd->setGroup("temp_unit", QStringList() << temperatureUnit());
    m_lcdPanel->setGroup("temp_unit", QStringList() << temperatureUnit());
    m_lcd->setNumber("temperature", fitValue(v , 4));
    m_lcdPanel->setNumber("temperature", fitValue(v , 3));
    setLCDIcon();
}

void WeatherStation::setHumidity(QString humidity)
{
    if (humidity == "N/A") {
        humidity = "-";
    } else {
        humidity.remove('%');
    }
    m_lcd->setNumber("humidity", humidity);
}

void WeatherStation::setWind(const Conversion::Value& speed, const QString& dir)
{
    //kDebug() << speed.number() << speed.unit()->symbol() << dir;
    QString s = fitValue(Conversion::Converter::self()->convert(speed, speedUnit()), 3);

    if (dir == "N/A") {
        m_lcd->setGroup("wind", m_lcd->groupItems("wind"));
    } else {
        m_lcd->setGroup("wind", QStringList() << dir);
    }

    m_lcd->setNumber("wind_speed", s);
    m_lcd->setGroup("wind_unit", QStringList() << speedUnit());
}

void WeatherStation::clicked(const QString &name)
{
    Q_UNUSED(name)
    KToolInvocation::invokeBrowser(m_url);
}

#include "weatherstation.moc"
