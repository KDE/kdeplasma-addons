/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
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

#include <QLabel>
#include <QTreeView>
#include <QHeaderView>
#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QPainter>
#include <QStandardItemModel>

#include <KGlobalSettings>
#include <KLocale>
#include <KMessageBox>
#include <KToolInvocation>
#include <KUnitConversion/Value>

#include <Plasma/Frame>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Svg>
#include <Plasma/TabBar>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>

#include <weatherview.h>
#include <cmath>

template <typename T> T clampValue(T value, int decimals)
{
        const T mul = std::pow(static_cast<T>(10), decimals); return int(value * mul) / mul;
}

class BackgroundWidget : public QGraphicsWidget
{
public:
    BackgroundWidget(QGraphicsWidget *parent)
        : QGraphicsWidget(parent)
    {
    }

    KIcon currentWeather() const
    {
        return m_currentWeather;
    }

    void setCurrentWeather(const KIcon &currentWeather)
    {
        m_currentWeather = currentWeather;
        update();
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0)
    {
        if (m_currentWeather.isNull()) {
            return;
        }

        QSize s(KIconLoader::SizeEnormous, KIconLoader::SizeEnormous);
        s.boundedTo(size().toSize());
        painter->drawPixmap(QPoint(0, 0), m_currentWeather.pixmap(s));
    }

private:
    KIcon m_currentWeather;
};

WeatherApplet::WeatherApplet(QObject *parent, const QVariantList &args)
        : WeatherPopupApplet(parent, args),
        m_locationLabel(new Plasma::Label),
        m_forecastTemps(new Plasma::Label),
        m_conditionsLabel(new Plasma::Label),
        m_tempLabel(new Plasma::Label),
        m_windIcon(new Plasma::IconWidget),
        m_courtesyLabel(new Plasma::Label),
        m_tabBar(new Plasma::TabBar),
        m_fiveDaysModel(0),
        m_detailsModel(0),
        m_fiveDaysView(0),
        m_detailsView(0),
        m_setupLayout(0),
        m_graphicsWidget(0),
        m_titleFrame(new Plasma::Frame)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPopupIcon("weather-not-available");
}

QGraphicsWidget *WeatherApplet::graphicsWidget()
{
    return m_graphicsWidget;
}

void WeatherApplet::init()
{
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(reloadTheme()));
    m_graphicsWidget = new BackgroundWidget(this);

    switch (formFactor()) {
    case Plasma::Horizontal:
    case Plasma::Vertical:
        Plasma::ToolTipManager::self()->registerWidget(this);
        break;
    default:
        Plasma::ToolTipManager::self()->unregisterWidget(this);
        break;
    }

    //FIXME: hardcoded quantities, could be better?
    m_titlePanel = new QGraphicsGridLayout;
    //m_titlePanel->setColumnMinimumWidth(0, KIconLoader::SizeHuge);
    //m_titlePanel->setColumnMaximumWidth(0, KIconLoader::SizeHuge * 1.4);

    //these minimum widths seems to give different "weights" when resizing the applet
    //m_titlePanel->setColumnMinimumWidth(1, 10);
    //m_titlePanel->setColumnMinimumWidth(2, 12);
    //m_titlePanel->setColumnMinimumWidth(3, 5);

    m_titlePanel->setHorizontalSpacing(0);
    m_titlePanel->setVerticalSpacing(0);
    m_bottomLayout = new QGraphicsLinearLayout(Qt::Horizontal);

    m_locationLabel->nativeWidget()->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    m_titleFont = QApplication::font();
    m_titleFont.setPointSize(m_titleFont.pointSize() * 1.4);
    m_titleFont.setBold(true);
    m_locationLabel->nativeWidget()->setFont(m_titleFont);
    m_locationLabel->nativeWidget()->setWordWrap(false);
    m_locationLabel->setMinimumWidth(85);

    m_conditionsLabel->nativeWidget()->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    m_conditionsLabel->nativeWidget()->setWordWrap(false);
    m_conditionsLabel->setMinimumWidth(55);

    m_windIcon->setMaximumSize(0,0);
    m_windIcon->setOrientation(Qt::Horizontal);
    m_windIcon->setTextBackgroundColor(QColor());

    m_tempLabel->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    m_tempLabel->nativeWidget()->setFont(m_titleFont);

    // This one if a bit crude, ideally we set the horizontal SizePolicy to Preferred, but that doesn't seem
    // to actually respect the minimum size needed to display the temperature. (Bug in Label or QGL?)
    m_tempLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_forecastTemps->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    m_forecastTemps->nativeWidget()->setWordWrap(false);
    m_forecastTemps->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_forecastTemps->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_titlePanel->setColumnMinimumWidth(0, KIconLoader::SizeHuge);
    m_titlePanel->addItem(m_locationLabel, 0, 1);
    m_titlePanel->addItem(m_tempLabel, 0, 4);
    m_titlePanel->addItem(m_conditionsLabel, 1, 1);
    m_titlePanel->addItem(m_forecastTemps, 1, 4);

    m_titlePanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    m_layout->setPreferredSize(400,300);

    m_titleFrame->setLayout(m_titlePanel);
    m_layout->addItem(m_titleFrame);

    m_courtesyLabel->nativeWidget()->setWordWrap(false);
    m_courtesyLabel->nativeWidget()->setAlignment(Qt::AlignRight);
    m_courtesyLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    connect(m_courtesyLabel, SIGNAL(linkActivated(QString)), this, SLOT(invokeBrowser(QString)));

    m_graphicsWidget->setLayout(m_layout);

    WeatherPopupApplet::init();
}

WeatherApplet::~WeatherApplet()
{
}

void WeatherApplet::toolTipAboutToShow()
{
    if (isPopupShowing()) {
        Plasma::ToolTipManager::self()->clearContent(this);
        return;
    }

    Plasma::ToolTipContent data(i18nc("Shown when you have not set a weather provider", "Please Configure"), "", popupIcon().pixmap(IconSize(KIconLoader::Desktop)));
    if (!m_locationLabel->text().isEmpty()) {
         data.setMainText(m_locationLabel->text());
         data.setSubText(i18nc("%1 is the weather condition, %2 is the temperature, both come from the weather provider", "%1 %2", m_conditionsLabel->text(), m_tempLabel->text()));
    }
    Plasma::ToolTipManager::self()->setContent(this, data);
}

void WeatherApplet::resizeView()
{
    if (m_fiveDaysView) {
        int totalColumns = m_fiveDaysView->nativeWidget()->header()->count();
        QString maxString, curString;
        int longestString = 0;

        kDebug() << "Total Columns: " << totalColumns;
        // Figure out maximum text length in model this will give us width sizing we want with setWidthSize
        for (int i = 0; i < totalColumns; i++) {
             curString  = m_fiveDaysView->model()->index(0, i).data(Qt::DisplayRole).toString();
             if (m_fiveDaysView->nativeWidget()->fontMetrics().width(curString) > m_fiveDaysView->nativeWidget()->fontMetrics().width(maxString)) {
                 maxString = curString;
                 longestString = m_fiveDaysView->nativeWidget()->fontMetrics().width("XX"+maxString);
             }
        }
        kDebug() << "Maximum string is: " << maxString;
        kDebug() << "Longest Size: " << longestString;
        kDebug() << "m_fiveDaysView width is: " << m_fiveDaysView->size().width();

        int maxColumns = m_fiveDaysView->size().width() / longestString;
        int shownColumns = 0;
        for (int i = 0; i < totalColumns; i++) {
             if (!m_fiveDaysView->nativeWidget()->isColumnHidden(i)) {
                 shownColumns++;
                 kDebug() << "Column " << i << " is NOT hidden";
             }
        }
        int difference = 0;
        kDebug() << "Maximum Column Width: " << maxColumns;
        kDebug() << "shown Columns: " << shownColumns;
        if (maxColumns < shownColumns) {
            difference = qAbs(maxColumns-shownColumns);
            kDebug() << "A: Difference is:" << difference;
            for (int i = maxColumns; i < shownColumns; ++i) {
                 kDebug() << "HIDE: i = " << i;
                 m_fiveDaysView->nativeWidget()->setColumnHidden(i, true);
            }
        } else {
            difference = qAbs(shownColumns-maxColumns);
            if (difference > totalColumns) {
                kDebug() << "Difference is bigger than totalColumns!";
            }
            kDebug() << "B: Difference is: " << difference;
            for (int i = difference; i < maxColumns; ++i) {
                 kDebug() << "UNHIDE: i = " << i;
                 m_fiveDaysView->nativeWidget()->setColumnHidden(i, false);
            }
        }
    }
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
    } else if (constraints & Plasma::SizeConstraint) {
        resizeView();
        update();
    }
}

void WeatherApplet::invokeBrowser(const QString& url) const
{
    KToolInvocation::invokeBrowser(url);
}

void WeatherApplet::setVisible(bool visible, QGraphicsLayout *layout)
{
    for (int i = 0; i < layout->count(); i++) {
        QGraphicsWidget *item = dynamic_cast<QGraphicsWidget *>(layout->itemAt(i));
        if (item){
            item->setVisible(visible);
        }
    }
}

void WeatherApplet::setVisibleLayout(bool val)
{
    if (m_titleFrame) {
        m_titleFrame->setVisible(val);
    }

    setVisible(val, m_titlePanel);
    setVisible(val, m_bottomLayout);

    m_courtesyLabel->setVisible(val);
}

QString WeatherApplet::convertTemperature(KUnitConversion::UnitPtr format, QString value,
                                          int type, bool rounded = false, bool degreesOnly = false)
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

void WeatherApplet::weatherContent(const Plasma::DataEngine::Data &data)
{
    m_locationLabel->setText(data["Place"].toString());
    QStringList fiveDayTokens = data["Short Forecast Day 0"].toString().split('|'); // Get current time period of day

    if (fiveDayTokens.count() > 1) {
        // fiveDayTokens[3] = High Temperature
        // fiveDayTokens[4] = Low Temperature
        if (fiveDayTokens[4] != "N/A" && fiveDayTokens[3] == "N/A") {  // Low temperature
            m_tempLabel->setText(convertTemperature(temperatureUnit(), data["Temperature"].toString(), data["Temperature Unit"].toInt(), false));
            m_forecastTemps->setText(i18nc("Low temperature", "Low: %1", convertTemperature(temperatureUnit(), fiveDayTokens[4], data["Temperature Unit"].toInt(), true)));
        } else if (fiveDayTokens[3] != "N/A" && fiveDayTokens[4] == "N/A") { // High temperature
            m_forecastTemps->setText(i18nc("High temperature", "High: %1", convertTemperature(temperatureUnit(), fiveDayTokens[3], data["Temperature Unit"].toInt(), true)));
        } else { // Both high and low
            m_forecastTemps->setText(i18nc("High & Low temperature", "H: %1 L: %2", convertTemperature(temperatureUnit(), fiveDayTokens[3], data["Temperature Unit"].toInt(), true), convertTemperature(temperatureUnit(),  fiveDayTokens[4], data["Temperature Unit"].toInt(), true)));
        }
    }
    else {
        m_forecastTemps->setText(QString());
    }

    m_conditionsLabel->setText(data["Current Conditions"].toString().trimmed());

    if (isValidData(data["Temperature"])) {
        m_tempLabel->setText(convertTemperature(temperatureUnit(), data["Temperature"].toString(), data["Temperature Unit"].toInt(), false));
    } else {
        m_tempLabel->setText(i18nc("Not available","N/A"));
    }

    if (!m_windIcon) {
        kDebug() << "Create new Plasma::IconWidget (wind)";
        m_windIcon = new Plasma::IconWidget();
        m_windIcon->setOrientation(Qt::Horizontal);
        m_windIcon->setDrawBackground(false);
    }

    if (data["Wind Speed"] != "N/A" && data["Wind Speed"].toDouble() != 0 && data["Wind Speed"] != "Calm") {
        KUnitConversion::Value v(data["Wind Speed"].toDouble(), data["Wind Speed Unit"].toInt());
        v = v.convertTo(speedUnit());
        m_windIcon->setText(i18nc("wind direction, speed","%1 %2 %3", data["Wind Direction"].toString(),
                clampValue(v.number(), 1), v.unit()->symbol()));
    } else {
        if (data["Wind Speed"] == "N/A") {
            m_windIcon->setText(i18nc("Not available","N/A"));
        } else {
            if (data["Wind Speed"].toInt() == 0 || data["Wind Speed"] == "Calm") {
                m_windIcon->setText(i18nc("Wind condition","Calm"));
            }
        }
    }

    m_courtesyLabel->setText(data["Credit"].toString());

    if (!data["Credit Url"].toString().isEmpty()) {
        QString creditUrl = QString("<A HREF=\"%1\" ><FONT size=\"-0.5\" color=\"%2\">%3</FONT></A>").arg(data["Credit Url"].toString()).arg(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).rgb()).arg(data["Credit"].toString());
        m_courtesyLabel->nativeWidget()->setTextInteractionFlags(Qt::TextBrowserInteraction);
        m_courtesyLabel->setText(creditUrl);
    }

    if (!isValidData(data["Condition Icon"]) ||
        data["Condition Icon"].toString() == "N/U" ||
        data["Condition Icon"].toString() == "N/A" ||
        data["Condition Icon"].toString() == "weather-none-available") {

        if (fiveDayTokens.count() > 2) {
            // if there is no specific icon, show the current weather
            m_graphicsWidget->setCurrentWeather(KIcon(fiveDayTokens[1]));
        } else {
            // if we are inside here, we could not find any proper icon
            // then just hide it
            m_graphicsWidget->setCurrentWeather(KIcon());
        }
    } else {
        m_graphicsWidget->setCurrentWeather(KIcon(data["Condition Icon"].toString()));
        setPopupIcon(data["Condition Icon"].toString());
    }

    m_tabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // If we have items in tab clean it up first
    while (m_tabBar->count()) {
        m_tabBar->takeTab(0);
    }

    // If we have a 5 day forecast, display it
    if (data["Total Weather Days"].toInt() > 0) {
        if (!m_fiveDaysView) {
            kDebug() << "Create 5 Days Plasma::WeatherView";
            m_fiveDaysView = new Plasma::WeatherView(m_tabBar);
            connect(m_fiveDaysView->nativeWidget()->header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(fiveDaysColumnResized(int, int, int)));
        }

        if (!m_fiveDaysModel) {
            kDebug() << "Create 5 Days QStandardItemModel";
            m_fiveDaysModel = new QStandardItemModel(this);
        } else {
            m_fiveDaysModel->clear();
        }

        QList<QStandardItem *>dayItems;
        QList<QStandardItem *>conditionItems; // Icon
        QList<QStandardItem *>hiItems;
        QList<QStandardItem *>lowItems;

        QFont titleFont = QApplication::font();

        QColor darkColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        darkColor.setAlphaF(0.5);

        for (int i = 0; i < data["Total Weather Days"].toInt(); i++) {
            QStringList fiveDayTokens = data[QString("Short Forecast Day %1").arg(i)].toString().split('|');

            if (fiveDayTokens.count() != 6) {
                // We don't have the right number of tokens, abort trying
                break;
            }

            QStandardItem *dayName = new QStandardItem();

            if (fiveDayTokens[0].contains("nt")) {
                fiveDayTokens[0].remove("nt");
                dayName->setForeground(darkColor);
            } else if (fiveDayTokens[0].contains("nite")) {
                dayName->setForeground(darkColor);
            } else {
                dayName->setForeground(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
            }

            titleFont.setBold(true);
            dayName->setFont(titleFont);
            dayName->setText(fiveDayTokens[0].trimmed());
            dayItems.append(dayName);

            // If we see N/U (Not Used) we skip the item
            if (fiveDayTokens[1] != "N/U") {
                Plasma::IconWidget *fiveDayIcon = new Plasma::IconWidget(this);
                fiveDayIcon->setIcon(KIcon(fiveDayTokens[1]));
                fiveDayIcon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
                fiveDayIcon->setDrawBackground(false);
                fiveDayIcon->hide();
                QStandardItem *iconItem = new QStandardItem(fiveDayIcon->icon(), NULL);

                if (fiveDayTokens[5] != "N/U") {
                    if (fiveDayTokens[5] != "N/A") {
                        iconItem->setToolTip(i18nc("certain weather condition, probability percentage", "%1 (%2%)", fiveDayTokens[2], fiveDayTokens[5]));
                    } else {
                        iconItem->setToolTip(fiveDayTokens[2]);
                    }
                } else {
                    iconItem->setToolTip(fiveDayTokens[2]);
                }
                conditionItems.append(iconItem);
            }

            if (fiveDayTokens[3] != "N/U") {
                QStandardItem *highItem = new QStandardItem();
                titleFont.setBold(false);
                highItem->setFont(titleFont);
                if (fiveDayTokens[3] == "N/A") {
                    highItem->setText(i18nc("Short for no data available","-"));
                    hiItems.append(highItem);
                } else {
                    highItem->setText(convertTemperature(temperatureUnit(), fiveDayTokens[3], data["Temperature Unit"].toInt(), true));
                    hiItems.append(highItem);
                }
            }

            if (fiveDayTokens[4] != "N/U") {
                QStandardItem *lowItem = new QStandardItem();
                titleFont.setBold(false);
                lowItem->setFont(titleFont);
                if (fiveDayTokens[4] == "N/A") {
                    lowItem->setText(i18nc("Short for no data available","-"));
                    lowItems.append(lowItem);
                } else {
                    lowItem->setText(convertTemperature(temperatureUnit(), fiveDayTokens[4], data["Temperature Unit"].toInt(), true));
                    lowItems.append(lowItem);
                }
            }
        }

        if (dayItems.count() > 0) {
            m_fiveDaysModel->appendRow(dayItems);
        }
        if (conditionItems.count() > 0) {
            m_fiveDaysModel->appendRow(conditionItems);
        }
        if (hiItems.count() > 0)  {
            m_fiveDaysModel->appendRow(hiItems);
        }
        if (lowItems.count() > 0) {
            m_fiveDaysModel->appendRow(lowItems);
        }

        if (m_fiveDaysModel->rowCount() != 0) {
            if (!m_fiveDaysView->model()) {
                m_fiveDaysView->setModel(m_fiveDaysModel);
            }
            // If we have any items, display 5 Day tab, otherwise only details
            QString totalDays = i18ncp("Forecast period timeframe", "1 Day", "%1 Days", data["Total Weather Days"].toInt());
            m_tabBar->addTab(totalDays, m_fiveDaysView);
        } else {
            delete m_fiveDaysView;
            m_fiveDaysView = 0;
        }
    } else {
        delete m_fiveDaysView;
        m_fiveDaysView = 0;
    }

    // Details data
    if (!m_detailsView) {
        kDebug() << "Create Details Plasma::WeatherView";
        m_detailsView = new Plasma::WeatherView(m_tabBar);
    }

    if (!m_detailsModel) {
        kDebug() << "Create Details QStandardItemModel";
        m_detailsModel = new QStandardItemModel(this);
    } else {
        m_detailsModel->clear();
    }

    if (isValidData(data["Windchill"])) {
       QStandardItem *dataWindchill = new QStandardItem();

       // Use temperature unit to convert windchill temperature we only show degrees symbol not actual temperature unit
       dataWindchill->setText(i18nc("windchill, unit", "Windchill: %1", convertTemperature(temperatureUnit(), data["Windchill"].toString(), data["Temperature Unit"].toInt(), false, true)));
       m_detailsModel->appendRow(dataWindchill);
    }

    if (isValidData(data["Humidex"])) {
        QStandardItem *dataHumidex = new QStandardItem();

        // Use temperature unit to convert humidex temperature we only show degrees symbol not actual temperature unit
        dataHumidex->setText(i18nc("humidex, unit","Humidex: %1", convertTemperature(temperatureUnit(), data["Humidex"].toString(), data["Temperature Unit"].toInt(), false, true)));

        m_detailsModel->appendRow(dataHumidex);
    }

    if (isValidData(data["Dewpoint"])) {
        QStandardItem *dataDewpoint = new QStandardItem();
        dataDewpoint->setText(i18nc("ground temperature, unit", "Dewpoint: %1", convertTemperature(temperatureUnit(), data["Dewpoint"].toString(), data["Temperature Unit"].toInt(), false)));
        m_detailsModel->appendRow(dataDewpoint);
    }

    if (isValidData(data["Pressure"])) {
        QStandardItem *dataPressure = new QStandardItem();
        KUnitConversion::Value v(data["Pressure"].toDouble(), data["Pressure Unit"].toInt());
        v = v.convertTo(pressureUnit());
        dataPressure->setText(i18nc("pressure, unit","Pressure: %1 %2", clampValue(v.number(), 2), v.unit()->symbol()));
        m_detailsModel->appendRow(dataPressure);
    }

    if (isValidData(data["Pressure Tendency"])) {
        QStandardItem *dataPressureTend = new QStandardItem();
        dataPressureTend->setText(i18nc("pressure tendency, rising/falling/steady", "Pressure Tendency: %1", data["Pressure Tendency"].toString()));
        m_detailsModel->appendRow(dataPressureTend);
    }

    if (isValidData(data["Visibility"])) {
        QStandardItem *dataVisibility = new QStandardItem();
        bool isNumeric;
        data["Visibility"].toDouble(&isNumeric);
        if (isNumeric) {
            KUnitConversion::Value v(data["Visibility"].toDouble(), data["Visibility Unit"].toInt());
            v = v.convertTo(visibilityUnit());
            dataVisibility->setText(i18nc("distance, unit","Visibility: %1 %2", clampValue(v.number(), 1), v.unit()->symbol()));
        } else {
            dataVisibility->setText(i18nc("visibility from distance", "Visibility: %1", data["Visibility"].toString()));
        }

        m_detailsModel->appendRow(dataVisibility);
    }

    if (isValidData(data["Humidity"])) {
        QStandardItem *dataHumidity = new QStandardItem();
        dataHumidity->setText(i18nc("content of water in air", "Humidity: %1%2", data["Humidity"].toString(), i18nc("Precent, measure unit", "%")));
        m_detailsModel->appendRow(dataHumidity);
    }

    if (isValidData(data["Wind Speed"])) {
        Plasma::Svg svgWindIcon;
        svgWindIcon.setImagePath("weather/wind-arrows");
        QIcon windIcon = svgWindIcon.pixmap(data["Wind Direction"].toString());
        m_windIcon->setIcon(windIcon);
        m_windIcon->setMaximumSize(m_windIcon->sizeFromIconSize(KIconLoader::SizeSmall));
        m_windIcon->update();

        QStandardItem *windInfo = new QStandardItem(m_windIcon->icon(), NULL);
        windInfo->setTextAlignment(Qt::AlignRight);
        windInfo->setText(m_windIcon->text());
        m_detailsModel->appendRow(windInfo);
    }

    if (isValidData(data["Wind Gust"])) {
        // Convert the wind format for nonstandard types
        QStandardItem *dataGust = new QStandardItem();
        KUnitConversion::Value v(data["Wind Gust"].toDouble(), data["Wind Gust Unit"].toInt());
        v = v.convertTo(speedUnit());
        dataGust->setText(i18nc("winds exceeding wind speed briefly", "Wind Gust: %1 %2", clampValue(v.number(), 1), v.unit()->symbol()));
        m_detailsModel->appendRow(dataGust);
    }

    if (m_detailsModel->rowCount() > 0) {
        if (!m_detailsView->model()) {
            m_detailsView->setModel(m_detailsModel);
        }
        m_tabBar->addTab(i18nc("current weather information", "Details"), m_detailsView);
    }

    int rowCount = 0;
    if (data["Total Watches Issued"].toInt() > 0 || data["Total Warnings Issued"].toInt() > 0) {
        QGraphicsLinearLayout *noticeLayout = new QGraphicsLinearLayout(Qt::Vertical);
        QPalette pal;

        QFont noticeTitleFont = QApplication::font();
        noticeTitleFont.setBold(true);

        // If we have watches or warnings display them in a tab for now
        if (data["Total Warnings Issued"].toInt() > 0) {
            QGraphicsGridLayout *warningLayout = new QGraphicsGridLayout();
            Plasma::Label *warningTitle = new Plasma::Label();
            warningTitle->setText(i18nc("weather warnings", "Warnings Issued:"));
            warningTitle->nativeWidget()->setFont(noticeTitleFont);
            noticeLayout->addItem(warningTitle);
            for (int k = 0; k < data["Total Warnings Issued"].toInt(); k++) {
                Plasma::Label *warnNotice = new Plasma::Label();
                connect(warnNotice, SIGNAL(linkActivated(QString)), this, SLOT(invokeBrowser(QString)));
                pal = warnNotice->nativeWidget()->palette();
                pal.setColor(warnNotice->nativeWidget()->foregroundRole(), Qt::red);
                // If there is a Url to go along with the watch/warning turn label into clickable link
                if (!data[QString("Warning Info %1").arg(k)].toString().isEmpty()) {
                    QString warnLink = QString("<A HREF=\"%1\">%2</A>").arg(data[QString("Warning Info %1").arg(k)].toString())
                                                                       .arg(data[QString("Warning Description %1").arg(k)].toString());
                    warnNotice->setText(warnLink);
                } else {
                    warnNotice->setText(data[QString("Warning Description %1").arg(k)].toString());
                }
                warnNotice->nativeWidget()->setPalette(pal);
                warningLayout->setRowSpacing(rowCount, 0);
                warningLayout->setRowStretchFactor(rowCount, 0);
                warningLayout->setRowMinimumHeight(rowCount, 0);
                warningLayout->setRowPreferredHeight(rowCount, 0);
                warningLayout->addItem(warnNotice, rowCount, 0);
                rowCount++;
            }

            noticeLayout->addItem(warningLayout);
        }

        QGraphicsWidget *spacer = new QGraphicsWidget(this);
        spacer->setMinimumHeight(15);
        spacer->setMaximumHeight(15);
        noticeLayout->addItem(spacer);

        rowCount = 0;
        // If we have watches or warnings display them in a tab for now
        if (data["Total Watches Issued"].toInt() > 0) {
            QGraphicsGridLayout *watchLayout = new QGraphicsGridLayout();
            Plasma::Label *watchTitle = new Plasma::Label();
            watchTitle->setText(i18nc("weather watches" ,"Watches Issued:"));
            watchTitle->nativeWidget()->setFont(noticeTitleFont);
            noticeLayout->addItem(watchTitle);
            for (int j = 0; j < data["Total Watches Issued"].toInt(); j++) {
                Plasma::Label *watchNotice = new Plasma::Label();
                connect(watchNotice, SIGNAL(linkActivated(QString)), this, SLOT(invokeBrowser(QString)));
                pal = watchNotice->nativeWidget()->palette();
                pal.setColor(watchNotice->nativeWidget()->foregroundRole(), Qt::yellow);
                if (!data[QString("Watch Info %1").arg(j)].toString().isEmpty()) {
                    QString watchLink = QString("<A HREF=\"%1\">%2</A>").arg(data[QString("Watch Info %1").arg(j)].toString())
                                                                       .arg(data[QString("Watch Description %1").arg(j)].toString());
                    watchNotice->setText(watchLink);
                } else {
                    watchNotice->setText(data[QString("Watch Description %1").arg(j)].toString());
                }
                watchNotice->nativeWidget()->setPalette(pal);
                watchLayout->setRowSpacing(rowCount, 0);
                watchLayout->setRowStretchFactor(rowCount, 0);
                watchLayout->setRowMinimumHeight(rowCount, 0);
                watchLayout->setRowPreferredHeight(rowCount, 0);
                watchLayout->addItem(watchNotice, rowCount, 0);
                rowCount++;
            }
            noticeLayout->addItem(watchLayout);
        }

        m_tabBar->addTab(i18nc("weather notices", "Notices"), noticeLayout);
    }

    if (!m_setupLayout) {
        QGraphicsWidget *spacer = new QGraphicsWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        spacer->setMinimumWidth(KIconLoader::SizeMedium);
        spacer->setMaximumWidth(KIconLoader::SizeHuge);
        m_bottomLayout->addItem(spacer);
        m_bottomLayout->addItem(m_tabBar);
        m_bottomLayout->setStretchFactor(m_tabBar, 2);
        m_layout->addItem(m_bottomLayout);
        m_layout->addItem(m_courtesyLabel);
        m_setupLayout = 1;
    }

    setVisibleLayout(true);
    update();
}

void WeatherApplet::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (data.isEmpty()) {
        return;
    }

    m_currentData = data;
    setVisibleLayout(false);
    weatherContent(data);
    WeatherPopupApplet::dataUpdated(source, data);
    update();
}

void WeatherApplet::configAccepted()
{
    setVisibleLayout(false);
    WeatherPopupApplet::configAccepted();
}

void WeatherApplet::reloadTheme()
{
    m_locationLabel->nativeWidget()->setFont(m_titleFont);
    m_tempLabel->nativeWidget()->setFont(m_titleFont);
}

#include "weatherapplet.moc"
