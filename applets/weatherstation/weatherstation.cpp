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
#include <Plasma/ToolTipManager>
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
    // i18n: This and other all-caps messages are pieces of text shown on
    // an LCD-like image mimicking a electronic weather station display.
    // If weather station displays in your country are always in English,
    // you may want to consider leaving these strings in English too,
    // to achieve a more realistic feeling.
    m_lcd->setLabel("pressure-label", i18n("PRESSURE"));
    m_lcd->setLabel("weather-label", i18n("CURRENT WEATHER"));
    m_lcd->setLabel("temperature-label", i18n("OUTDOOR TEMP"));
    m_lcd->setLabel("humidity-label", i18n("HUMIDITY"));
    m_lcd->setLabel("wind-label", i18n("WIND"));
    m_lcd->setLabel("provider-label", QString());
    connect(m_lcd, SIGNAL(clicked(const QString&)), this, SLOT(clicked(const QString&)));

    m_lcdPanel = new LCD(this);
    m_lcdPanel->setSvg("weatherstation/lcd_panel");
    m_lcdPanel->setLabel("temperature-label", i18n("OUTDOOR TEMP"));
    m_lcdPanel->hide();

    //m_lcd->setItemOn("under_construction");

    KConfigGroup cfg = config();

    m_useBackground = cfg.readEntry("background", true);
    m_showToolTip = cfg.readEntry("tooltip", false);
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
    m_appearanceConfig.tooltipCheckBox->setChecked(m_showToolTip);
    parent->addPage(w, i18n("Appearance"), icon());
}

void WeatherStation::configAccepted()
{
    KConfigGroup cfg = config();
    cfg.writeEntry("background", m_useBackground =
            m_appearanceConfig.backgroundCheckBox->isChecked());
    cfg.writeEntry("tooltip", m_showToolTip =
            m_appearanceConfig.tooltipCheckBox->isChecked());
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

Conversion::Value WeatherStation::value(const QString& value, const QString& unit)
{
    if (value.isEmpty() || value == "N/A") {
        return Conversion::Value();
    }
    return Conversion::Value(value, unit);
}

void WeatherStation::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    //kDebug() << data;
    WeatherPopupApplet::dataUpdated(source, data);

    if (data.contains("Place")) {
        Conversion::Value temp = value(data["Temperature"].toString(),
                WeatherUtils::getUnitString(data["Temperature Unit"].toInt(), true));
        setTemperature(temp);
        setPressure(conditionIcon(),
                    value(data["Pressure"].toString(),
                        WeatherUtils::getUnitString(data["Pressure Unit"].toInt())),
                    data["Pressure Tendency"].toString());
        setHumidity(data["Humidity"].toString());
        setWind(value(data["Wind Speed"].toString(),
                WeatherUtils::getUnitString(data["Wind Speed Unit"].toInt(), true)),
                data["Wind Direction"].toString());
        m_lcd->setLabel("provider-label", data["Credit"].toString());
        m_url = data["Credit Url"].toString();
        m_lcd->setItemClickable("provider-click", !m_url.isEmpty());

        if (m_showToolTip) {
            Plasma::ToolTipContent ttc(data["Place"].toString(),
                    i18n("Last updated: %1", KGlobal::locale()->formatDateTime(QDateTime::currentDateTime(), KLocale::FancyLongDate)));
            Plasma::ToolTipManager::self()->setContent(this, ttc);
        }
    }
}

QString WeatherStation::fitValue(const Conversion::Value& value, int digits)
{
    if (!value.isValid()) {
        return "-";
    }
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

    Conversion::Value value = Conversion::Converter::self()->convert(pressure, pressureUnit());
    QString s = fitValue(value, 5);
    m_lcd->setNumber("pressure", s);
    m_lcd->setLabel("pressure-unit-label", value.unit()->symbol());

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
    m_lcd->setLabel("temperature-unit-label", v.unit()->symbol());
    m_lcdPanel->setLabel("temperature-unit-label", v.unit()->symbol());
    m_lcd->setNumber("temperature", fitValue(v , 4));
    m_lcdPanel->setNumber("temperature", fitValue(v , 3));
    setLCDIcon();
}

void WeatherStation::setHumidity(QString humidity)
{
    if (humidity == "N/A") {
        humidity = '-';
    } else {
        humidity.remove('%');
    }
    m_lcd->setNumber("humidity", humidity);
}

void WeatherStation::setWind(const Conversion::Value& speed, const QString& dir)
{
    //kDebug() << speed.number() << speed.unit()->symbol() << dir;
    Conversion::Value value = Conversion::Converter::self()->convert(speed, speedUnit());
    QString s = fitValue(value, 3);

    if (dir == "N/A") {
        m_lcd->setGroup("wind", QStringList());
    } else {
        m_lcd->setGroup("wind", QStringList() << dir);
    }
    m_lcd->setNumber("wind_speed", s);
    m_lcd->setLabel("wind-unit-label", value.unit()->symbol());
}

void WeatherStation::clicked(const QString &name)
{
    Q_UNUSED(name)
    KToolInvocation::invokeBrowser(m_url);
}

#include "weatherstation.moc"
