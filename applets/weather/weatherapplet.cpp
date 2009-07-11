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
#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QStandardItemModel>

#include <KGlobalSettings>
#include <KLocale>
#include <KMessageBox>
#include <KToolInvocation>

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

WeatherApplet::WeatherApplet(QObject *parent, const QVariantList &args)
        : WeatherPopupApplet(parent, args),
        m_locationLabel(new Plasma::Label),
        m_forecastTemps(new Plasma::Label),
        m_conditionsLabel(new Plasma::Label),
        m_currentIcon(new Plasma::IconWidget),
        m_tempLabel(new Plasma::Label),
        m_windIcon(new Plasma::IconWidget),
        m_courtesyLabel(new Plasma::Label),
        m_fiveDaysModel(0),
        m_detailsModel(0),
        m_graphicsWidget(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_fiveDaysView = 0;
    m_detailsView = 0;
    m_tabBar = 0;
    m_currentIcon = 0;
    m_titleFrame = 0;
    m_setupLayout = 0;
    setPopupIcon("weather-not-available");
}

QGraphicsWidget *WeatherApplet::graphicsWidget()
{
    return m_graphicsWidget;
}

void WeatherApplet::init()
{
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(reloadTheme()));

    m_graphicsWidget = new QGraphicsWidget(this);

    switch (formFactor()) {
    case Plasma::Horizontal:
    case Plasma::Vertical:
        Plasma::ToolTipManager::self()->registerWidget(this);
        break;
    default:
        Plasma::ToolTipManager::self()->unregisterWidget(this);
        break;
    }

    m_titleFrame = new Plasma::Frame(this);

    //FIXME: hardcoded quantities, could be better?
    m_titlePanel = new QGraphicsGridLayout;
    m_titlePanel->setColumnMinimumWidth(0, KIconLoader::SizeHuge);
    m_titlePanel->setColumnMaximumWidth(0, KIconLoader::SizeHuge * 1.5);

    //these minimum widths seems to give different "weights" when resizing the applet
    m_titlePanel->setColumnMinimumWidth(1, 10);
    m_titlePanel->setColumnMinimumWidth(2, 12);
    m_titlePanel->setColumnMinimumWidth(3, 5);

    m_titlePanel->setHorizontalSpacing(0);
    m_titlePanel->setVerticalSpacing(0);
    m_bottomLayout = new QGraphicsLinearLayout(Qt::Horizontal);

    m_locationLabel->nativeWidget()->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    m_titleFont = QApplication::font();
    m_titleFont.setPointSize(m_titleFont.pointSize() * 1.6);
    m_titleFont.setBold(true);
    m_locationLabel->nativeWidget()->setFont(m_titleFont);
    m_locationLabel->nativeWidget()->setWordWrap(false);

    m_locationLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_conditionsLabel->nativeWidget()->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    m_conditionsLabel->nativeWidget()->setWordWrap(false);

    m_windIcon->setMaximumSize(0,0);
    m_windIcon->setOrientation(Qt::Horizontal);
    m_windIcon->setTextBackgroundColor(QColor());

    m_tempLabel->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    m_tempLabel->nativeWidget()->setFont(m_titleFont);
    m_tempLabel->nativeWidget()->setWordWrap(false);
    // This one if a bit crude, ideally we set the horizontal SizePolicy to Preferred, but that doesn't seem
    // to actually respect the minimum size needed to display the temperature. (Bug in Label or QGL?)
    m_tempLabel->setMinimumWidth(85);
    m_tempLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_forecastTemps->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    m_forecastTemps->nativeWidget()->setWordWrap(false);
    m_forecastTemps->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_forecastTemps->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    /*QGraphicsWidget *titleSpacer = new QGraphicsWidget(this);
    //FIXME: will be a width somewhat related to the weather icon size
    titleSpacer->setPreferredWidth(60);
    titleSpacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_titlePanel->addItem(titleSpacer, 0, 0, 2, 1);*/

    m_titlePanel->addItem(m_locationLabel, 0, 0, 1, 3);
    m_titlePanel->addItem(m_tempLabel, 0, 3);
    m_titlePanel->addItem(m_conditionsLabel, 1, 0);
    m_titlePanel->addItem(m_windIcon, 1, 2);
    m_titlePanel->addItem(m_forecastTemps, 1, 3);

    m_titlePanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    m_layout->setPreferredSize(400,300);

    // This seems broken with layouts :(
    m_titleFrame->setLayout(m_titlePanel);
    m_layout->addItem(m_titleFrame);

    m_courtesyLabel->nativeWidget()->setWordWrap(false);
    m_courtesyLabel->nativeWidget()->setAlignment(Qt::AlignRight);
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

void WeatherApplet::invokeBrowser(const QString& url)
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

QString WeatherApplet::convertTemperature(int format, QString value, int type, bool rounded = false, bool degreesOnly = false)
{
    double temp = WeatherUtils::convertTemperature(value.toDouble(), type, format);

    if (rounded) {
        int tempNumber = qRound(temp);
        if (degreesOnly) {
            return i18nc("temperature, unit", "%1%2", tempNumber, WeatherUtils::getUnitString(WeatherUtils::DegreeUnit, false));
        } else {
            return i18nc("temperature, unit", "%1%2", tempNumber, WeatherUtils::getUnitString(format, false));
        }
    } else {
        float formattedTemp = clampValue(temp, 1);
        if (degreesOnly) {
            return i18nc("temperature, unit", "%1%2", formattedTemp, WeatherUtils::getUnitString(WeatherUtils::DegreeUnit, false));
        } else {
            return i18nc("temperature, unit", "%1%2", formattedTemp, WeatherUtils::getUnitString(format, false));
        }
    }
}

bool WeatherApplet::isValidData(const QVariant &data)
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
            m_tempLabel->setText(convertTemperature(temperatureUnitInt(), data["Temperature"].toString(), data["Temperature Unit"].toInt(), false));
            m_forecastTemps->setText(i18nc("Low temperature", "Low: %1", convertTemperature(temperatureUnitInt(), fiveDayTokens[4], data["Temperature Unit"].toInt())));
        } else if (fiveDayTokens[3] != "N/A" && fiveDayTokens[4] == "N/A") { // High temperature
            m_forecastTemps->setText(i18nc("High temperature", "High: %1", convertTemperature(temperatureUnitInt(), fiveDayTokens[3], data["Temperature Unit"].toInt())));
        } else { // Both high and low
            m_forecastTemps->setText(i18nc("High & Low temperature", "H: %1 L: %2", convertTemperature(temperatureUnitInt(), fiveDayTokens[3], data["Temperature Unit"].toInt()), convertTemperature(temperatureUnitInt(),  fiveDayTokens[4], data["Temperature Unit"].toInt())));
        }
    }
    else {
        m_forecastTemps->setText(QString());
    }

    m_conditionsLabel->setText(data["Current Conditions"].toString());

    if (isValidData(data["Temperature"])) {
        m_tempLabel->setText(convertTemperature(temperatureUnitInt(), data["Temperature"].toString(), data["Temperature Unit"].toInt(), false));

    } else {
        m_tempLabel->setText(QString());
    }

    if (!m_currentIcon) {
        kDebug() << "Create new Plasma::IconWidget";
        m_currentIcon = new Plasma::IconWidget(this);
        m_currentIcon->setMaximumWidth(KIconLoader::SizeEnormous);
        m_currentIcon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //m_currentIcon = new Plasma::IconWidget(KIcon(data["Condition Icon"].toString()), QString(), this);
        //m_currentIcon->icon().pixmap(QSize(KIconLoader::SizeEnormous,KIconLoader::SizeEnormous));
        m_currentIcon->setDrawBackground(false);
    }
    m_courtesyLabel->setText(data["Credit"].toString());

    if (!data["Credit Url"].toString().isEmpty()) {
        QString creditUrl = QString("<A HREF=\"%1\" ><FONT size=\"-0.5\" color=\"%2\">%3</FONT></A>").arg(data["Credit Url"].toString()).arg(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).rgb()).arg(data["Credit"].toString());
        m_courtesyLabel->nativeWidget()->setTextInteractionFlags(Qt::TextBrowserInteraction);
        //m_courtesyLabel->setAcceptHoverEvents(true);
        //m_courtesyLabel->nativeWidget()->setMouseTracking(true);

        m_courtesyLabel->setText(creditUrl);
    }

    if (data["Condition Icon"].toString() == "N/A") {
        m_currentIcon->setIcon(KIcon("weather-not-available"));
        setPopupIcon("weather-not-available");
    } else {
        m_currentIcon->setIcon(KIcon(data["Condition Icon"].toString()));
        setPopupIcon(data["Condition Icon"].toString());
    }

    if (!m_tabBar) {
        m_tabBar = new Plasma::TabBar(this);
        m_tabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        kDebug() << "Create new Plasma::TabBar";
    }

    if (m_tabBar->count() > 0) {
        // If we have items in tab clean it up first
        while (m_tabBar->count()) {
             m_tabBar->removeTab(0);
        }
    }

    // FIXME: Destroy the treeview because if we don't Plasma crashes?:w
    if (m_fiveDaysView) {
        delete m_fiveDaysView;
        m_fiveDaysView = 0;
    }

    // If we have a 5 day forecast, display it
    if (data["Total Weather Days"].toInt() > 0) {
        if (!m_fiveDaysView) {
            kDebug() << "Create 5 Days Plasma::WeatherView";
            m_fiveDaysView = new Plasma::WeatherView(m_tabBar);
            m_fiveDaysView->setHasHeader(true);
            m_fiveDaysView->setOrientation(Qt::Vertical);
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
            dayName->setText(fiveDayTokens[0]);
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
                    highItem->setText(convertTemperature(temperatureUnitInt(), fiveDayTokens[3], data["Temperature Unit"].toInt()));
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
                    lowItem->setText(convertTemperature(temperatureUnitInt(), fiveDayTokens[4], data["Temperature Unit"].toInt()));
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
        }
    }

    // Details data
    if (!m_detailsView) {
        kDebug() << "Create Details Plasma::WeatherView";
        m_detailsView = new Plasma::WeatherView(m_tabBar);
        m_detailsView->setHasHeader(false);
        m_detailsView->setOrientation(Qt::Horizontal);
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
       dataWindchill->setText(i18nc("windchill, unit", "Windchill: %1", convertTemperature(temperatureUnitInt(), data["Windchill"].toString(), data["Temperature Unit"].toInt(), false, true)));
       m_detailsModel->appendRow(dataWindchill);
    }

    if (isValidData(data["Humidex"])) {
        QStandardItem *dataHumidex = new QStandardItem();

        // Use temperature unit to convert humidex temperature we only show degrees symbol not actual temperature unit
        dataHumidex->setText(i18nc("humidex, unit","Humidex: %1", convertTemperature(temperatureUnitInt(), data["Humidex"].toString(), data["Temperature Unit"].toInt(), false, true)));

        m_detailsModel->appendRow(dataHumidex);
    }

    if (isValidData(data["Dewpoint"])) {
        QStandardItem *dataDewpoint = new QStandardItem();
        dataDewpoint->setText(i18nc("ground temperature, unit", "Dewpoint: %1", convertTemperature(temperatureUnitInt(), data["Dewpoint"].toString(), data["Dewpoint Unit"].toInt(), false)));
        m_detailsModel->appendRow(dataDewpoint);
    }

    if (isValidData(data["Pressure"])) {
        QStandardItem *dataPressure = new QStandardItem();
        dataPressure->setText(i18nc("pressure, unit","Pressure: %1 %2", clampValue(WeatherUtils::convertPressure(data["Pressure"].toDouble(), data["Pressure Unit"].toInt(), pressureUnitInt()), 2), pressureUnit()));
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
        double visibility = data["Visibility"].toDouble(&isNumeric);
        Q_UNUSED(visibility)
        if (isNumeric) {
            dataVisibility->setText(i18nc("distance, unit","Visibility: %1 %2", clampValue(WeatherUtils::convertDistance(data["Visibility"].toDouble(), data["Visibility Unit"].toInt(), visibilityUnitInt()), 1), visibilityUnit()));
        } else {
            dataVisibility->setText(i18nc("visibility from distance", "Visibility: %1", data["Visibility"].toString()));
        }

        m_detailsModel->appendRow(dataVisibility);
    }

    if (isValidData(data["Humidity"])) {
        QStandardItem *dataHumidity = new QStandardItem();
        dataHumidity->setText(i18nc("content of water in air", "Humidity: %1", data["Humidity"].toString()));
        m_detailsModel->appendRow(dataHumidity);
    }

    if (data["Wind Speed"] != "N/A" && data["Wind Speed"].toDouble() != 0 && data["Wind Speed"] != "Calm") {
        m_windIcon->setText(i18nc("wind direction, speed","%1 %2 %3", data["Wind Direction"].toString(),
                clampValue(WeatherUtils::convertSpeed(data["Wind Speed"].toDouble(), data["Wind Speed Unit"].toInt(), speedUnitInt()), 1), speedUnit()));
    } else {
        if (data["Wind Speed"] == "N/A") {
            m_windIcon->setText(i18nc("Not available","N/A"));
        } else {
            if (data["Wind Speed"].toInt() == 0 || data["Wind Speed"] == "Calm") {
                m_windIcon->setText(i18nc("Wind condition","Calm"));
            }
        }
    }

    m_windIcon->setSvg("weather/wind-arrows", data["Wind Direction"].toString());
    m_windIcon->setMaximumSize(m_windIcon->sizeFromIconSize(KIconLoader::SizeSmall));
    m_windIcon->update();

    if (isValidData(data["Wind Gust"])) {
        // Convert the wind format for nonstandard types
        QStandardItem *dataGust = new QStandardItem();
        dataGust->setText(i18nc("winds exceeding wind speed briefly", "Wind Gust: %1 %2", clampValue(WeatherUtils::convertSpeed(data["Wind Gust"].toDouble(), data["Wind Gust Unit"].toInt(), speedUnitInt()), 1), speedUnit()));
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
        m_bottomLayout->addItem(m_currentIcon);
        m_bottomLayout->addItem(m_tabBar);
        m_layout->addItem(m_bottomLayout);
        m_layout->addItem(m_courtesyLabel);
        m_setupLayout = 1;
    }
    update();

    setVisibleLayout(true);
    //updateGeometry();
}

void WeatherApplet::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (data.isEmpty()) {
        return;
    }

    m_currentData = data;
    setVisibleLayout(false);
    weatherContent(data);
    update();
    WeatherPopupApplet::dataUpdated(source, data);
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
