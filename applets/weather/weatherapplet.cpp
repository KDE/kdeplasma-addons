/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
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

#include "weatherapplet.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

#include <KIcon>
#include <KIconLoader>
#include <KToolInvocation>
#include <KUnitConversion/Value>

#include <Plasma/ToolTipManager>
#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>

#include <cmath>

template <typename T>
T clampValue(T value, int decimals)
{
    const T mul = std::pow(static_cast<T>(10), decimals);
    return int(value * mul) / mul;
}

bool isValidIconName(const QString &icon)
{
    return !icon.isEmpty() &&
           !KIconLoader::global()->loadIcon(icon, KIconLoader::Desktop, 0,
                                            KIconLoader::DefaultState, QStringList(), 0, true).isNull();
}


WeatherApplet::WeatherApplet(QObject *parent, const QVariantList &args)
        : WeatherPopupApplet(parent, args)
        , m_declarativeWidget(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPopupIcon("weather-none-available");
}

void WeatherApplet::init()
{
    switch (formFactor()) {
    case Plasma::Horizontal:
    case Plasma::Vertical:
        Plasma::ToolTipManager::self()->registerWidget(this);
        break;
    default:
        Plasma::ToolTipManager::self()->unregisterWidget(this);
        break;
    }

    resetPanelModel();

    WeatherPopupApplet::init();
}

WeatherApplet::~WeatherApplet()
{
}

QGraphicsWidget *WeatherApplet::graphicsWidget()
{
    if (!m_declarativeWidget) {
        m_declarativeWidget = new Plasma::DeclarativeWidget(this);

        m_declarativeWidget->engine()->rootContext()->setContextProperty("weatherApplet", this);

        Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
        Plasma::Package package(QString(), "org.kde.weather", structure);
        m_declarativeWidget->setQmlPath(package.filePath("mainscript"));
    }
    return m_declarativeWidget;
}

void WeatherApplet::toolTipAboutToShow()
{
    if (isPopupShowing()) {
        Plasma::ToolTipManager::self()->clearContent(this);
        return;
    }

    QString config = i18nc("Shown when you have not set a weather provider", "Please Configure");
    Plasma::ToolTipContent data(config, "", popupIcon().pixmap(IconSize(KIconLoader::Desktop)));

    QString location = m_panelModel["location"].toString();
    QString conditions = m_panelModel["conditions"].toString();
    QString temp = m_panelModel["temp"].toString();
    if (!location.isEmpty()) {
         data.setMainText(location);
         data.setSubText(i18nc("%1 is the weather condition, %2 is the temperature,"
                               " both come from the weather provider", "%1 %2", conditions, temp));
    }
    Plasma::ToolTipManager::self()->setContent(this, data);
}

void WeatherApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        switch (formFactor()) {
        case Plasma::Horizontal:
        case Plasma::Vertical:
            Plasma::ToolTipManager::self()->registerWidget(this);
            break;
        default:
            Plasma::ToolTipManager::self()->unregisterWidget(this);
            break;
        }
    }
}

void WeatherApplet::invokeBrowser(const QString& url) const
{
    if (url.isEmpty())
        KToolInvocation::invokeBrowser(m_creditUrl);
    else
        KToolInvocation::invokeBrowser(url);
}

QString WeatherApplet::convertTemperature(KUnitConversion::UnitPtr format, QString value,
                                          int type, bool rounded, bool degreesOnly)
{
    KUnitConversion::Value v(value.toDouble(), type);
    v = v.convertTo(format);

    if (rounded) {
        int tempNumber = qRound(v.number());
        if (degreesOnly) {
            return i18nc("temperature, unit", "%1%2", tempNumber, i18nc("Degree, unit symbol", "°"));
        } else {
            return i18nc("temperature, unit", "%1%2", tempNumber, v.unit()->symbol());
        }
    } else {
        float formattedTemp = clampValue(v.number(), 1);
        if (degreesOnly) {
            return i18nc("temperature, unit", "%1%2", formattedTemp, i18nc("Degree, unit symbol", "°"));
        } else {
            return i18nc("temperature, unit", "%1%2", formattedTemp, v.unit()->symbol());
        }
    }
}

bool WeatherApplet::isValidData(const QVariant &data) const
{
    return ((data != "N/A") && (!data.toString().isEmpty()));
}

void WeatherApplet::resetPanelModel()
{
    m_panelModel.clear();
    m_panelModel["location"] = "";
    m_panelModel["forecastTemps"] = "";
    m_panelModel["conditions"] = "";
    m_panelModel["temp"] = "";
    m_panelModel["courtesy"] = "";
    m_panelModel["conditionIcon"] = "";
    m_panelModel["totalDays"] = "";
    m_panelModel["enableLink"] = false;
}

void WeatherApplet::updatePanelModel(const Plasma::DataEngine::Data &data)
{
    resetPanelModel();
    m_panelModel["location"] = data["Place"].toString();

    int unit = data["Temperature Unit"].toInt();
    // Get current time period of day
    QStringList fiveDayTokens = data["Short Forecast Day 0"].toString().split('|');

    if (fiveDayTokens.count() > 1) {
        // fiveDayTokens[3] => High Temperature
        // fiveDayTokens[4] => Low Temperature
        QString high, low;

        if (fiveDayTokens[4] != "N/A") {
            low = convertTemperature(temperatureUnit(), fiveDayTokens[4], unit, true);
        }

        if (fiveDayTokens[3] != "N/A") {
            high = convertTemperature(temperatureUnit(), fiveDayTokens[3], unit, true);
        }

        if (!low.isEmpty() && !high.isEmpty()) {
            m_panelModel["forecastTemps"] = i18nc("High & Low temperature", "H: %1 L: %2", high, low);
        } else if (!low.isEmpty()) {
            m_panelModel["forecastTemps"] = i18nc("Low temperature", "Low: %1", low);
        } else {
            m_panelModel["forecastTemps"] = i18nc("High temperature", "High: %1", high);
        }
    }

    m_panelModel["conditions"] = data["Current Conditions"].toString().trimmed();

    if (isValidData(data["Temperature"])) {
        m_panelModel["temp"] = convertTemperature(temperatureUnit(), data["Temperature"].toString(), unit);
    }

    m_panelModel["courtesy"] = data["Credit"].toString();
    m_creditUrl = data["Credit Url"].toString();
    m_panelModel["enableLink"] = !m_creditUrl.isEmpty();

    const QString conditionIcon = data["Condition Icon"].toString();
    if (!isValidData(data["Condition Icon"])
        || conditionIcon == "weather-none-available"
        || conditionIcon == "N/U"
        || conditionIcon == "N/A") {

        if (fiveDayTokens.count() > 2) {
            // if there is no specific icon, show the current weather
            m_panelModel["conditionIcon"] = fiveDayTokens[1];
            setPopupIcon(KIcon(fiveDayTokens[1]));
        } else {
            // if we could not find any proper icon then just hide it
            m_panelModel["conditionIcon"] = "";
            setPopupIcon(KIcon("weather-none-available"));
        }
    } else {
        m_panelModel["conditionIcon"] = conditionIcon;
        if (isValidIconName(conditionIcon)) {
            setPopupIcon(conditionIcon);
        } else {
            setPopupIcon("weather-not-available");
        }
    }
}

void WeatherApplet::updateFiveDaysModel(const Plasma::DataEngine::Data &data)
{
    if (data["Total Weather Days"].toInt() <= 0) {
        return;
    }

    m_fiveDaysModel.clear();

    QStringList dayItems;
    QStringList conditionItems; // Icon
    QStringList hiItems;
    QStringList lowItems;

    for (int i = 0; i < data["Total Weather Days"].toInt(); i++) {
        QString current = QString("Short Forecast Day %1").arg(i);
        QStringList fiveDayTokens = data[current].toString().split('|');

        if (fiveDayTokens.count() != 6) {
            // We don't have the right number of tokens, abort trying
            break;
        }

        dayItems << fiveDayTokens[0];

        // If we see N/U (Not Used) we skip the item
        if (fiveDayTokens[1] != "N/U") {
            QString iconAndToolTip;
            if (isValidIconName(fiveDayTokens[1])) {
                iconAndToolTip = fiveDayTokens[1];
            } else {
                iconAndToolTip = "weather-not-available";
            }
            iconAndToolTip += "|";

            if (fiveDayTokens[5] != "N/U" && fiveDayTokens[5] != "N/A") {
                iconAndToolTip += i18nc("certain weather condition, probability percentage",
                                        "%1 (%2%)", fiveDayTokens[2], fiveDayTokens[5]);
            } else {
                iconAndToolTip += fiveDayTokens[2];
            }
            conditionItems << iconAndToolTip;
        }

        if (fiveDayTokens[3] != "N/U") {
            if (fiveDayTokens[3] == "N/A") {
                hiItems << i18nc("Short for no data available", "-");
            } else {
                hiItems << convertTemperature(temperatureUnit(),
                                              fiveDayTokens[3],
                                              data["Temperature Unit"].toInt(),
                                              true);
            }
        }

        if (fiveDayTokens[4] != "N/U") {
            if (fiveDayTokens[4] == "N/A") {
                lowItems << i18nc("Short for no data available", "-");
            } else {
                lowItems << convertTemperature(temperatureUnit(),
                                               fiveDayTokens[4],
                                               data["Temperature Unit"].toInt(),
                                               true);
            }
        }
    }

    if (dayItems.count() > 0) {
        m_fiveDaysModel << dayItems;
    }
    if (conditionItems.count() > 0) {
        m_fiveDaysModel << conditionItems;
    }
    if (hiItems.count() > 0)  {
        m_fiveDaysModel << hiItems;
    }
    if (lowItems.count() > 0) {
        m_fiveDaysModel << lowItems;
    }

    m_panelModel["totalDays"] = i18ncp("Forecast period timeframe", "1 Day",
                                       "%1 Days", data["Total Weather Days"].toInt());
}

void WeatherApplet::updateDetailsModel(const Plasma::DataEngine::Data &data)
{
    m_detailsModel.clear();

    QVariantMap row;
    row["icon"] = "";
    row["text"] = "";

    int unit = data["Temperature Unit"].toInt();
    QString temp;

    if (isValidData(data["Windchill"])) {
        // Use temperature unit to convert windchill temperature
        // we only show degrees symbol not actual temperature unit
        temp = convertTemperature(temperatureUnit(), data["Windchill"].toString(), unit, false, true);
        row["text"] = i18nc("windchill, unit", "Windchill: %1", temp);
        m_detailsModel << row;
    }

    if (isValidData(data["Humidex"])) {
        // Use temperature unit to convert humidex temperature
        // we only show degrees symbol not actual temperature unit
        temp = convertTemperature(temperatureUnit(), data["Humidex"].toString(), unit, false, true);
        row["text"] = i18nc("humidex, unit","Humidex: %1", temp);
        m_detailsModel << row;
    }

    if (isValidData(data["Dewpoint"])) {
        temp = convertTemperature(temperatureUnit(), data["Dewpoint"].toString(), unit);
        row["text"] = i18nc("ground temperature, unit", "Dewpoint: %1", temp);
        m_detailsModel << row;
    }

    if (isValidData(data["Pressure"])) {
        KUnitConversion::Value v(data["Pressure"].toDouble(), data["Pressure Unit"].toInt());
        v = v.convertTo(pressureUnit());
        row["text"] = i18nc("pressure, unit","Pressure: %1 %2",
                            clampValue(v.number(), 2), v.unit()->symbol());
        m_detailsModel << row;
    }

    if (isValidData(data["Pressure Tendency"])) {
        row["text"] = i18nc("pressure tendency, rising/falling/steady",
                            "Pressure Tendency: %1", data["Pressure Tendency"].toString());
        m_detailsModel << row;
    }

    if (isValidData(data["Visibility"])) {
        bool isNumeric;
        data["Visibility"].toDouble(&isNumeric);
        if (isNumeric) {
            KUnitConversion::Value v(data["Visibility"].toDouble(), data["Visibility Unit"].toInt());
            v = v.convertTo(visibilityUnit());
            row["text"] = i18nc("distance, unit","Visibility: %1 %2",
                                clampValue(v.number(), 1), v.unit()->symbol());
        } else {
            row["text"] = i18nc("visibility from distance", "Visibility: %1", data["Visibility"].toString());
        }

        m_detailsModel << row;
    }

    if (isValidData(data["Humidity"])) {
        row["text"] = i18nc("content of water in air", "Humidity: %1%2",
                            data["Humidity"].toString(), i18nc("Precent, measure unit", "%"));
        m_detailsModel << row;
    }

    if (isValidData(data["Wind Speed"])) {
        row["icon"] = data["Wind Direction"].toString();

        if (data["Wind Speed"] == "N/A") {
            row["text"] = i18nc("Not available","N/A");
        } else if (data["Wind Speed"].toDouble() != 0 && data["Wind Speed"] != "Calm") {
            KUnitConversion::Value v(data["Wind Speed"].toDouble(), data["Wind Speed Unit"].toInt());
            v = v.convertTo(speedUnit());
            row["text"] = i18nc("wind direction, speed","%1 %2 %3", data["Wind Direction"].toString(),
                                clampValue(v.number(), 1), v.unit()->symbol());
        } else {
            row["text"] = i18nc("Wind condition","Calm");
        }

        m_detailsModel << row;
        row["icon"] = "";
    }

    if (isValidData(data["Wind Gust"])) {
        // Convert the wind format for nonstandard types
        KUnitConversion::Value v(data["Wind Gust"].toDouble(), data["Wind Gust Unit"].toInt());
        v = v.convertTo(speedUnit());
        row["text"] = i18nc("winds exceeding wind speed briefly", "Wind Gust: %1 %2",
                            clampValue(v.number(), 1), v.unit()->symbol());
        m_detailsModel << row;
    }
}

void WeatherApplet::updateNoticesModel(const Plasma::DataEngine::Data &data)
{
    m_noticesModel.clear();

    QVariantList warnings;
    for (int i = 0; i < data["Total Warnings Issued"].toInt(); i++) {
        QVariantMap warning;
        warning["description"] = data[QString("Warning Description %1").arg(i)];
        warning["info"] = data[QString("Warning Info %1").arg(i)];
        warnings << warning;
    }
    m_noticesModel << QVariant(warnings);

    QVariantList watches;
    for (int i = 0; i < data["Total Watches Issued"].toInt(); i++) {
        QVariantMap watch;
        watch["description"] = data[QString("Watch Description %1").arg(i)];
        watch["info"] = data[QString("Watch Info %1").arg(i)];
        watches << watch;
    }
    m_noticesModel << QVariant(watches);
}

void WeatherApplet::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (data.isEmpty()) {
        return;
    }

    updatePanelModel(data);
    updateFiveDaysModel(data);
    updateDetailsModel(data);
    updateNoticesModel(data);
    WeatherPopupApplet::dataUpdated(source, data);

    emit modelUpdated();
    update();
}

void WeatherApplet::configAccepted()
{
    resetPanelModel();
    m_fiveDaysModel.clear();
    m_detailsModel.clear();
    emit modelUpdated();

    WeatherPopupApplet::configAccepted();
}

#include "weatherapplet.moc"
