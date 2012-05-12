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

#include "weatherstation.h"

#include <math.h>

#include <QGraphicsLinearLayout>

#include <KConfigDialog>
#include <KConfigGroup>
#include <KToolInvocation>
#include <KUnitConversion/Converter>

#include <Plasma/Containment>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>
#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>

#include <plasmaweather/weatherconfig.h>

using namespace KUnitConversion;

WeatherStation::WeatherStation(QObject *parent, const QVariantList &args)
    : WeatherPopupApplet(parent, args)
{
    resize(250, 350);
}

WeatherStation::~WeatherStation()
{
}

void WeatherStation::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    m_declarativeWidget = new Plasma::DeclarativeWidget(this);
    layout->addItem(m_declarativeWidget);

    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    m_package = new Plasma::Package(QString(), "org.kde.lcdweather", structure);
    m_declarativeWidget->setQmlPath(m_package->filePath("mainscript"));
    //m_declarativeWidget->engine()->rootContext()->setContextProperty("", );

    WeatherPopupApplet::init();
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
    connect(m_appearanceConfig.backgroundCheckBox, SIGNAL(clicked(bool)) , parent, SLOT(settingsModified()));
    connect(m_appearanceConfig.tooltipCheckBox, SIGNAL(clicked(bool)) , parent, SLOT(settingsModified()));
}

void WeatherStation::configAccepted()
{
    KConfigGroup cfg = config();
    cfg.writeEntry("background", m_useBackground =
            m_appearanceConfig.backgroundCheckBox->isChecked());
    cfg.writeEntry("tooltip", m_showToolTip =
            m_appearanceConfig.tooltipCheckBox->isChecked());
    WeatherPopupApplet::configAccepted();
}

void WeatherStation::configChanged()
{
    KConfigGroup cfg = config();
    m_useBackground = cfg.readEntry("background", true);
    m_showToolTip = cfg.readEntry("tooltip", true);

    if (!m_showToolTip) {
        //m_lcd->setLabel("weather-label", i18n("CURRENT WEATHER"));
        Plasma::ToolTipManager::self()->clearContent(this);
    }
//    setBackground();
//    setLCDIcon();
    WeatherPopupApplet::configChanged();
}

Value WeatherStation::value(const QString& value, int unit)
{
    if (value.isEmpty() || value == "N/A") {
        return Value();
    }
    return Value(value.toDouble(), unit);
}

void WeatherStation::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    //kDebug() << data;
    WeatherPopupApplet::dataUpdated(source, data);

    if (data.contains("Place")) {
        QString v = data["Temperature"].toString();
        Value temp = value(v, data["Temperature Unit"].toInt());
//        setTemperature(temp, (v.indexOf('.') > -1));
//        setPressure(conditionIcon(),
//                    value(data["Pressure"].toString(), data["Pressure Unit"].toInt()),
//                    data["Pressure Tendency"].toString());
//        setHumidity(data["Humidity"].toString());
//        setWind(value(data["Wind Speed"].toString(), data["Wind Speed Unit"].toInt()),
//                data["Wind Direction"].toString());
//        m_lcd->setLabel("provider-label", data["Credit"].toString());
        m_url = data["Credit Url"].toString();
//        m_lcd->setItemClickable("provider-click", !m_url.isEmpty());

        if (m_showToolTip) {
//           m_lcd->setLabel("weather-label", data["Place"].toString().toUpper());
            Plasma::ToolTipContent ttc(data["Place"].toString(),
                    i18n("Last updated: %1", KGlobal::locale()->formatDateTime(QDateTime::currentDateTime(), KLocale::FancyLongDate)));
            Plasma::ToolTipManager::self()->setContent(this, ttc);
        }
    }
}

QString WeatherStation::fitValue(const Value& value, int digits)
{
    if (!value.isValid()) {
        return "-";
    }
    double v = value.number();
    int mainDigits = (int)floor(log10(fabs(v))) + 1;
    int precision = 0;

    mainDigits += ((v < 0) ? 1 : 0); // minus sign
    if (mainDigits < digits) {
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
    //kDebug() << condition << result;
    return result;
}

void WeatherStation::setPressure(const QString& condition, const Value& pressure,
                                 const QString& tendencyString)
{
    QStringList current;
    current = fromCondition(condition);
//    m_lcd->setGroup("weather", current);

    Value value = pressure.convertTo(pressureUnit());
    QString s = fitValue(value, 5);
//    m_lcd->setNumber("pressure", s);
//    m_lcd->setLabel("pressure-unit-label", value.unit()->symbol());

    qreal t;
    if (tendencyString.toLower() == "rising") {
        t = 1.0;
    } else if (tendencyString.toLower() == "falling") {
        t = -1.0;
    } else {
        t = tendencyString.toDouble();
    }

//    if (t > 0.0) {
//        m_lcd->setGroup("pressure_direction", QStringList() << "up");
//    } else if (t < 0.0) {
//        m_lcd->setGroup("pressure_direction", QStringList() << "down");
//    } else {
//        m_lcd->setGroup("pressure_direction", QStringList());
//    }
}

void WeatherStation::setTemperature(const Value& temperature, bool hasDigit)
{
    hasDigit = hasDigit || (temperatureUnit() != temperature.unit());
    Value v = temperature.convertTo(temperatureUnit());
    qDebug() << v.isValid();
//    m_lcd->setLabel("temperature-unit-label", v.unit()->symbol());
//    m_lcdPanel->setLabel("temperature-unit-label", v.unit()->symbol());
    QString tmp = hasDigit ? fitValue(v , 4) : QString::number(v.number());
//    m_lcd->setNumber("temperature", tmp);
    tmp = hasDigit ? fitValue(v , 3) : QString::number(v.number());
//    m_lcdPanel->setNumber("temperature", tmp);
//    setLCDIcon();
}

void WeatherStation::setHumidity(QString humidity)
{
    if (humidity == "N/A") {
        humidity = '-';
    } else {
        humidity.remove('%');
    }
//    m_lcd->setNumber("humidity", humidity);
}

void WeatherStation::setWind(const Value& speed, const QString& dir)
{
    //kDebug() << speed.number() << speed.unit()->symbol() << dir;
    Value value = speed.convertTo(speedUnit());
    QString s = fitValue(value, 3);

//    if (dir == "N/A") {
//        m_lcd->setGroup("wind", QStringList());
//    } else {
//        m_lcd->setGroup("wind", QStringList() << dir);
//    }
//    m_lcd->setNumber("wind_speed", s);
//    m_lcd->setLabel("wind-unit-label", value.unit()->symbol());
}

void WeatherStation::clicked(const QString &name)
{
    Q_UNUSED(name)
    KToolInvocation::invokeBrowser(m_url);
}

#include "weatherstation.moc"
