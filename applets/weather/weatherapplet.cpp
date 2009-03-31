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
#include <QCheckBox>
#include <KMessageBox>
#include <KRun>
#include <KDialog>
#include <KConfigDialog>
#include <KLocale>
//#include <KAction>
#include <QStandardItemModel>
#include <QPushButton>
#include <QPainter>
#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/tooltipmanager.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/frame.h>
#include <plasma/widgets/tabbar.h>
#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/busywidget.h>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

#include <weatherview.h>

WeatherApplet::WeatherApplet(QObject *parent, const QVariantList &args)
        : Plasma::PopupApplet(parent, args),
        m_addDialog(0),
        m_amodel(0),
        m_activeValidation(0),
        m_weatherWindFormat(0),
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
    setHasConfigurationInterface(true);
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
    if (m_graphicsWidget) {
        return m_graphicsWidget;
    }

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

    KConfigGroup generalConfig = config();
    QStringList places;
    // Per applet automatically setup a KConfig
    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        m_weatherWindFormat = generalConfig.readEntry("windFormat", (int)WeatherUtils::KilometersPerHour);
        m_weatherTempFormat = generalConfig.readEntry("tempFormat", (int)WeatherUtils::Celsius);
        m_weatherPressureFormat = generalConfig.readEntry("pressureFormat", (int)WeatherUtils::Kilopascals);
        m_weatherVisibilityFormat = generalConfig.readEntry("visibilityFormat", (int)WeatherUtils::Kilometers);
    } else {
        m_weatherWindFormat = generalConfig.readEntry("windFormat", (int)WeatherUtils::MilesPerHour);
        m_weatherTempFormat = generalConfig.readEntry("tempFormat", (int)WeatherUtils::Fahrenheit);
        m_weatherPressureFormat = generalConfig.readEntry("pressureFormat", (int)WeatherUtils::InchesHG);
        m_weatherVisibilityFormat = generalConfig.readEntry("visibilityFormat", (int)WeatherUtils::Miles);
    }
    // Default time to update weather - 30 minutes
    m_weatherUpdateTime = generalConfig.readEntry("updateWeather", 30);

    // "Update Now" contextual action
    //m_actionUpdateNow = new KAction(i18n("&Update Now"), this);
    //connect(m_actionUpdateNow, SIGNAL(triggered()), this, SLOT(getWeather()));

    // Connect to weather engine.
    weatherEngine = dataEngine("weather");

    // Set custom options
    m_ionPlugins = weatherEngine->query("ions");

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
    QFont titleFont = QApplication::font();
    titleFont.setPointSize(titleFont.pointSize() * 1.6);
    titleFont.setBold(true);
    m_locationLabel->nativeWidget()->setFont(titleFont);
    m_locationLabel->nativeWidget()->setWordWrap(false);

    m_locationLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_conditionsLabel->nativeWidget()->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    m_conditionsLabel->nativeWidget()->setWordWrap(false);

    m_windIcon->setMaximumSize(0,0);
    m_windIcon->setOrientation(Qt::Horizontal);

    m_tempLabel->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    m_tempLabel->nativeWidget()->setFont(titleFont);
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
    connect(m_courtesyLabel, SIGNAL(linkActivated(QString)), this, SLOT(creditLink(QString)));

    // If we have any weather observations set them up here.
    foreach(const QString& place, generalConfig.groupList()) {
        KConfigGroup placeConfig(&generalConfig, place);
        m_activePlace = place;
        m_activeIon = placeConfig.readEntry("ion");
        m_extraData[m_activePlace] = placeConfig.readEntry("data");
    }


    if (m_activePlace.isEmpty()) {
        setConfigurationRequired(true);
    } else {
        getWeather();
        setConfigurationRequired(false);
    }

    m_graphicsWidget->setLayout(m_layout);

    return m_graphicsWidget;
}

WeatherApplet::~WeatherApplet()
{
    if (m_addDialog) {
        delete m_addDialog;
    }

    if (m_bottomLayout) {
         if (m_fiveDaysView) {
             kDebug() << "Delete the Plasma::m_fiveDaysView";
             delete m_fiveDaysView;
             m_fiveDaysView = 0;
             kDebug() << "Finished deleting Plasma::m_fiveDaysView";
         }

         if (m_detailsView) {
             kDebug() << "Delete the Plasma::m_detailsView";
             delete m_detailsView;
             m_detailsView = 0;
             kDebug() << "Finished deleting Plasma::m_detailsView";
         }
    }
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

void WeatherApplet::getValidation()
{
    if (ui.locationEdit->text().size() >= 3) {
        // Destroy all other datasources
        foreach (const QString& source, weatherEngine->sources()) {
            if (source != "ions") {
                weatherEngine->disconnectSource(source, this);
            }
        }

        QString ion = ui.pluginComboList->itemData(ui.pluginComboList->currentIndex()).toString();
        QString checkPlace = ui.locationEdit->text();
        checkPlace[0] = checkPlace[0].toUpper();
        m_activeValidation = QString("%1|validate|%2").arg(ion).arg(checkPlace);
        weatherEngine->connectSource(m_activeValidation, this);
    }
}

void WeatherApplet::selectPlace()
{
    addPlace();
    m_addDialog->close();
}

void WeatherApplet::creditLink(const QString& url)
{
    KRun::runUrl(KUrl(url), "text/html", 0);
}

void WeatherApplet::weatherNoticeLink(const QString& url)
{
    KRun::runUrl(KUrl(url), "text/html", 0);
}

/*
QList<QAction*> WeatherApplet::contextualActions()
{
    QList<QAction*> actions;

    actions.append(m_actionUpdateNow);

    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    actions.append(separator);

    return actions;
}
*/

void WeatherApplet::setVisibleLayout(bool val)  
{
    if (m_titleFrame) {
        if (val) {
            m_titleFrame->show();
        } else {
            m_titleFrame->hide();
        }
    }

    if (m_titlePanel->count() > 0) {
        for (int j = 0; j < m_titlePanel->count(); j++) {
             QGraphicsWidget *titleItem = dynamic_cast<QGraphicsWidget *>(m_titlePanel->itemAt(j));
             if (val) {
                 titleItem->show();
             } else {
                 titleItem->hide();
             }
        }
    }

    if (m_bottomLayout->count() > 0) {
        for (int i = 0; i < m_bottomLayout->count(); i++) {
             QGraphicsWidget *item = dynamic_cast<QGraphicsWidget *>(m_bottomLayout->itemAt(i));
             if (val) {
                 item->show();
             } else {
                 item->hide();
             }
        }
    }

    if (val) {
        m_courtesyLabel->show();
    } else {
        m_courtesyLabel->hide();
    }
}

void WeatherApplet::addPlace()
{
    QModelIndex item = aui.foundPlacesListView->currentIndex();

    m_activePlace = item.data().toString();
    m_activeIon = ui.pluginComboList->itemData(ui.pluginComboList->currentIndex()).toString();
    ui.validatedPlaceLabel->setText(m_activePlace);
    ui.validateButton->setEnabled(false);
}

void WeatherApplet::cancelAddClicked()
{
    weatherEngine->disconnectSource(m_activeValidation, this);
}

void WeatherApplet::pluginIndexChanged(int index)
{
   Q_UNUSED(index)

   ui.validatedPlaceLabel->clear();
   if (ui.locationEdit->text().size() < 3) {
       ui.validateButton->setEnabled(false);
   } else {
       ui.validateButton->setEnabled(true);
   }
}

void WeatherApplet::placeEditChanged(const QString& text)
{
    if (text.size() < 3) {
        ui.validateButton->setEnabled(false);
    } else {
        ui.validateButton->setEnabled(true);
    }
}

void WeatherApplet::validate(const QString& source, const QVariant& data)
{
    const QStringList tokens = data.toString().split('|');
    bool extraflag = false;
    bool placeflag = false;
    QString place;

    // If the place is valid, check if there is one place or multiple places returned. The user will have
    // to select the place that best matches what they are looking for.
    if (tokens[1] == QString("valid")) {
        // Plugin returns only one matching place
        if (tokens[2] == QString("single") || tokens[2] == QString("multiple")) {
            m_activeValidation = source;
            m_items.clear();
            m_extraData.clear();

            foreach(const QString& val, tokens) {
                if (val.contains("place")) {
                    placeflag = true;
                    continue;
                }

                if (placeflag) {
                    place = val;
                    placeflag = false;
                }

                if (val.contains("extra")) {
                    extraflag = true;
                    continue;
                }

                if (extraflag) {
                    extraflag = false;
                    m_extraData[place] = val;
                    continue;
                }
            }

        }

       // Pop up dialog and allow user to choose places
       if (tokens[2] == "multiple") {
           showAddPlaceDialog(tokens);
       } else {
           ui.validatedPlaceLabel->setText(tokens[4]);
           ui.validateButton->setEnabled(false);
       }

       return;

    } else if (tokens[1] == "timeout") {
        KMessageBox::error(0, i18n("The applet was not able to contact the server, please try again later"));
        return;
    } else if (tokens[1] == "malformed") {
        KMessageBox::error(0 ,i18n("The data source received a malformed string and was not able to process your request"));
        return;
    } else {
        KMessageBox::error(0, i18n("The place '%1' is not valid. The data source is not able to find this place.", tokens[3]), i18n("Invalid Place"));
        return;
    }

}

void WeatherApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    QWidget *unitWidget = new QWidget();
    ui.setupUi(widget);
    uui.setupUi(unitWidget);
    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    parent->addPage(widget, i18n("Location"), icon());
    parent->addPage(unitWidget, i18n("Units"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.validateButton->setEnabled(false);

    foreach(const QVariant& item, m_ionPlugins) {
        QStringList pluginInfo = item.toString().split("|");
        ui.pluginComboList->addItem(pluginInfo[0], pluginInfo[1]);
    }
    ui.pluginComboList->model()->sort(0, Qt::AscendingOrder);

    uui.windOptionsComboList->addItem(i18n("Kilometers Per Hour (km/h)"), WeatherUtils::KilometersPerHour);
    uui.windOptionsComboList->addItem(i18n("Metres Per Second (m/s)"), WeatherUtils::MetersPerSecond);
    uui.windOptionsComboList->addItem(i18n("Miles Per Hour (mph)"), WeatherUtils::MilesPerHour);
    uui.windOptionsComboList->addItem(i18n("Knots (kt)"), WeatherUtils::Knots);
    uui.windOptionsComboList->addItem(i18n("Beaufort Scale"), WeatherUtils::Beaufort);

    uui.tempOptionsComboList->addItem(i18n("Celsius"), WeatherUtils::Celsius);
    uui.tempOptionsComboList->addItem(i18n("Fahrenheit"), WeatherUtils::Fahrenheit);
    uui.tempOptionsComboList->addItem(i18n("Kelvin"), WeatherUtils::Kelvin);

    uui.pressureOptionsComboList->addItem(i18n("Kilopascals (kPa)"), WeatherUtils::Kilopascals);
    uui.pressureOptionsComboList->addItem(i18n("Inches of Mercury (inHg)"), WeatherUtils::InchesHG);
    uui.pressureOptionsComboList->addItem(i18n("Millibars (mb)"), WeatherUtils::Millibars);
    uui.pressureOptionsComboList->addItem(i18n("Hectopascals (hPa)"), WeatherUtils::Hectopascals);

    //uui.visibilityOptionsComboList->addItem(i18n("Meters"), WeatherUtils::Meters);
    uui.visibilityOptionsComboList->addItem(i18n("Kilometers"), WeatherUtils::Kilometers);
    uui.visibilityOptionsComboList->addItem(i18n("Miles"), WeatherUtils::Miles);
    //uui.windOptionsComboList->model()->sort(0, Qt::AscendingOrder);

    ui.locationEdit->setTrapReturnKey(true);

    if (m_activeIon.isEmpty()) {
        ui.pluginComboList->setCurrentIndex(0);
    } else {
        ui.pluginComboList->setCurrentIndex(ui.pluginComboList->findData(m_activeIon));
    }

    connect(ui.validateButton, SIGNAL(clicked()), this, SLOT(getValidation()));
    connect(ui.locationEdit, SIGNAL(textChanged(const QString &)), this, SLOT(placeEditChanged(const QString &)));
    connect(ui.pluginComboList, SIGNAL(currentIndexChanged(int)), this, SLOT(pluginIndexChanged(int)));
    connect(ui.locationEdit, SIGNAL(returnPressed()), this, SLOT(getValidation()));

    ui.weatherUpdateSpin->setSuffix(ki18np(" minute", " minutes"));
    ui.weatherUpdateSpin->setValue(m_weatherUpdateTime);
    ui.locationEdit->clear();
    ui.validatedPlaceLabel->setText(m_activePlace);
    uui.windOptionsComboList->setCurrentIndex(uui.windOptionsComboList->findData(m_weatherWindFormat));
    uui.tempOptionsComboList->setCurrentIndex(uui.tempOptionsComboList->findData(m_weatherTempFormat));
    uui.pressureOptionsComboList->setCurrentIndex(uui.pressureOptionsComboList->findData(m_weatherPressureFormat));
    uui.visibilityOptionsComboList->setCurrentIndex(uui.visibilityOptionsComboList->findData(m_weatherVisibilityFormat));
}

void WeatherApplet::showAddPlaceDialog(const QStringList& tokens)
{
    if (m_addDialog == 0) {
        m_addDialog = new KDialog;
        aui.setupUi(m_addDialog->mainWidget());
        m_addDialog->mainWidget()->layout()->setMargin(0);

        // Set up QListView with model/view
        m_amodel = new QStandardItemModel();
        aui.foundPlacesListView->setModel(m_amodel);
        aui.foundPlacesListView->setSelectionMode(QAbstractItemView::SingleSelection);

        aui.foundPlacesListView->show();

        m_addDialog->setCaption(i18n("Found Places"));
        m_addDialog->setButtons(KDialog::Ok | KDialog::Cancel);
        m_addDialog->setButtonText(KDialog::Ok, i18n("&Add"));
        m_addDialog->setDefaultButton(KDialog::NoDefault);


        connect(m_addDialog, SIGNAL(okClicked()), this, SLOT(addPlace()));
        connect(m_addDialog, SIGNAL(cancelClicked()), this, SLOT(cancelAddClicked()));
        connect(aui.foundPlacesListView, SIGNAL(doubleClicked(const QModelIndex &)), this , SLOT(selectPlace()));
    }
    bool placeflag = false;
    QStringList headers;
    m_amodel->clear();
    headers << i18n("Found Places");
    m_amodel->setHorizontalHeaderLabels(headers);
    m_amodel->setColumnCount(1);
    aui.foundPlacesListView->setResizeMode(QListView::Adjust);

    foreach(const QString& item, tokens) {
        if (item.contains("place")) {
            placeflag = true;
            continue;
        }
        if (placeflag) {
            m_items.clear();
            m_items.append(new QStandardItem(item));
            m_amodel->appendRow(m_items);
            placeflag = false;
        }
    }
    KDialog::centerOnScreen(m_addDialog);
    m_addDialog->show();
}

void WeatherApplet::getWeather()
{

    setVisibleLayout(false);
    setBusy(true);

    if (m_extraData[m_activePlace].isEmpty()) {
        QString str = QString("%1|weather|%2").arg(m_activeIon).arg(m_activePlace);
        foreach (const QString& source, weatherEngine->sources()) {
            if (source == str) {
                if (!m_currentData.isEmpty()) {
                    weatherContent(m_currentData);
                    return;
                }
            }
        }
        weatherEngine->connectSource(QString("%1|weather|%2").arg(m_activeIon).arg(m_activePlace), this, m_weatherUpdateTime * 60 * 1000);

    } else {
        QString str = QString("%1|weather|%2|%3").arg(m_activeIon).arg(m_activePlace).arg(m_extraData[m_activePlace]);

        foreach (const QString& source, weatherEngine->sources()) {
            if (source == str) {
                if (!m_currentData.isEmpty()) {
                    weatherContent(m_currentData);
                    return;
                }
            }
        }

        weatherEngine->connectSource(QString("%1|weather|%2|%3").arg(m_activeIon).arg(m_activePlace).arg(m_extraData[m_activePlace]), this, m_weatherUpdateTime * 60 * 1000);
    }
}

QString WeatherApplet::convertTemperature(int format, QString value, int type, bool rounded = true, bool degreesOnly = false)
{
    double val = value.toDouble();
    double temp = WeatherUtils::convertTemperature(val, type, format);

    if (rounded) {
        int tempNumber = qRound(temp);
        if (degreesOnly) {
            return i18nc("temperature, unit", "%1%2", tempNumber, WeatherUtils::getUnitString(WeatherUtils::DegreeUnit, false));
        } else {
            return i18nc("temperature, unit", "%1%2", tempNumber, WeatherUtils::getUnitString(format, false));
        }
    } else {
        QString formattedTemp = (val - (int) val) ? QString::number(temp, 'f', 1) : QString::number((int) temp);
        if (degreesOnly) {
            return i18nc("temperature, unit", "%1%2", formattedTemp, WeatherUtils::getUnitString(WeatherUtils::DegreeUnit, false));
        } else {
            return i18nc("temperature, unit", "%1%2", formattedTemp, WeatherUtils::getUnitString(format, false));
        }
    }
}

void WeatherApplet::weatherContent(const Plasma::DataEngine::Data &data)
{
    m_locationLabel->setText(data["Place"].toString());
    QStringList fiveDayTokens = data["Short Forecast Day 0"].toString().split("|"); // Get current time period of day

    if (fiveDayTokens.count() > 1) {
        // fiveDayTokens[3] = High Temperature
        // fiveDayTokens[4] = Low Temperature
        if (fiveDayTokens[4] != "N/A" && fiveDayTokens[3] == "N/A") {  // Low temperature
            m_tempLabel->setText(convertTemperature(m_weatherTempFormat, data["Temperature"].toString(), data["Temperature Unit"].toInt(), false));
            m_forecastTemps->setText(i18n("Low: %1", convertTemperature(m_weatherTempFormat, fiveDayTokens[4], data["Temperature Unit"].toInt())));
        } else if (fiveDayTokens[3] != "N/A" && fiveDayTokens[4] == "N/A") { // High temperature
            m_forecastTemps->setText(i18n("High: %1", convertTemperature(m_weatherTempFormat, fiveDayTokens[3], data["Temperature Unit"].toInt())));
        } else { // Both high and low
            m_forecastTemps->setText(i18nc("High & Low temperature", "H: %1 L: %2", convertTemperature(m_weatherTempFormat, fiveDayTokens[3], data["Temperature Unit"].toInt()), convertTemperature(m_weatherTempFormat,  fiveDayTokens[4], data["Temperature Unit"].toInt())));
        }
    }
    else {
        m_forecastTemps->setText(QString());
    }

    m_conditionsLabel->setText(data["Current Conditions"].toString());

    if (data["Temperature"] != "N/A" && data["Temperature"].toString().isEmpty() == false) {
        m_tempLabel->setText(convertTemperature(m_weatherTempFormat, data["Temperature"].toString(), data["Temperature Unit"].toInt(), false));

    } else {
        m_tempLabel->setText(QString());
    }

    //m_windDirLabel->setText(i18n("Wind Direction: %1", data["Wind Direction"].toString()));

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

    // Do some cleanup on WeatherView
    if (m_fiveDaysView) {
        delete m_fiveDaysView;
        m_fiveDaysView = 0;
    }

    // If we have a 5 day forecast, display it
    if (data["Total Weather Days"].toInt() > 0) {
        if (!m_fiveDaysView) {
            kDebug() << "Create 5 Days Plasma::WeatherView";
            m_fiveDaysView = new Plasma::WeatherView();
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
        titleFont.setBold(true);

        QColor darkColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        darkColor.setAlphaF(0.5);

        for (int i = 0; i < data["Total Weather Days"].toInt(); i++) {
            QStringList fiveDayTokens = data[QString("Short Forecast Day %1").arg(i)].toString().split("|");

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
                highItem->setFont(titleFont);
                if (fiveDayTokens[3] == "N/A") {
                    highItem->setText(i18nc("Short for no data available","-"));
                    hiItems.append(highItem);
                } else {
                    highItem->setText(convertTemperature(m_weatherTempFormat, fiveDayTokens[3], data["Temperature Unit"].toInt()));
                    hiItems.append(highItem);
                }
            }

            if (fiveDayTokens[4] != "N/U") {
                QStandardItem *lowItem = new QStandardItem();
                lowItem->setFont(titleFont);
                if (fiveDayTokens[4] == "N/A") {
                    lowItem->setText(i18nc("Short for no data available","-"));
                    lowItems.append(lowItem);
                } else {
                    lowItem->setText(convertTemperature(m_weatherTempFormat, fiveDayTokens[4], data["Temperature Unit"].toInt()));
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
            m_tabBar->addTab(i18n("5 Days"), m_fiveDaysView);
        }
    }

    // Details data
    if (!m_detailsView) {
        kDebug() << "Create Details Plasma::WeatherView";
        m_detailsView = new Plasma::WeatherView();
        m_detailsView->setHasHeader(false);
        m_detailsView->setOrientation(Qt::Horizontal);
    }

    if (!m_detailsModel) {
        kDebug() << "Create Details QStandardItemModel";
        m_detailsModel = new QStandardItemModel(this);
    } else {
        m_detailsModel->clear();
    }

    if (data["Windchill"] != "N/A" && data["Windchill"].toString().isEmpty() == false) {
       QStandardItem *dataWindchill = new QStandardItem();
       
       // Use temperature unit to convert windchill temperature we only show degrees symbol not actual temperature unit
       dataWindchill->setText(i18nc("windchill, unit", "Windchill: %1", convertTemperature(m_weatherTempFormat, data["Windchill"].toString(), data["Temperature Unit"].toInt(), false, true)));
       m_detailsModel->appendRow(dataWindchill);
    }

    if (data["Humidex"] != "N/A" && data["Humidex"].toString().isEmpty() == false) {
        QStandardItem *dataHumidex = new QStandardItem();

        // Use temperature unit to convert humidex temperature we only show degrees symbol not actual temperature unit
        dataHumidex->setText(i18nc("humidex, unit","Humidex: %1", convertTemperature(m_weatherTempFormat, data["Humidex"].toString(), data["Temperature Unit"].toInt(), false, true)));

        m_detailsModel->appendRow(dataHumidex);
    }

    if (data["Dewpoint"] != "N/A" && data["Dewpoint"].toString().isEmpty() == false) {
        QStandardItem *dataDewpoint = new QStandardItem();
        dataDewpoint->setText(i18n("Dewpoint: %1", convertTemperature(m_weatherTempFormat, data["Dewpoint"].toString(), data["Dewpoint Unit"].toInt(), false)));
        m_detailsModel->appendRow(dataDewpoint);
    }

    if (data["Pressure"] != "N/A" && data["Pressure"].toString().isEmpty() == false) {
        QStandardItem *dataPressure = new QStandardItem();
        dataPressure->setText(i18nc("pressure, unit","Pressure: %1 %2", QString::number(WeatherUtils::convertPressure(data["Pressure"].toDouble(), data["Pressure Unit"].toInt(), m_weatherPressureFormat), 'f', 2), WeatherUtils::getUnitString(m_weatherPressureFormat, false)));
        m_detailsModel->appendRow(dataPressure);
    }

    if (data["Pressure Tendency"] != "N/A" && data["Pressure Tendency"].toString().isEmpty() == false) {
        QStandardItem *dataPressureTend = new QStandardItem();
        dataPressureTend->setText(i18n("Pressure Tendency: %1", data["Pressure Tendency"].toString()));
        m_detailsModel->appendRow(dataPressureTend);
    }

    if (data["Visibility"] != "N/A" && data["Visibility"].toString().isEmpty() == false) {
        QStandardItem *dataVisibility = new QStandardItem();
        bool isNumeric;
        double visibility = data["Visibility"].toDouble(&isNumeric);
        Q_UNUSED(visibility)
        if (isNumeric) {
            dataVisibility->setText(i18nc("distance, unit","Visibility: %1 %2", QString::number(WeatherUtils::convertDistance(data["Visibility"].toDouble(), data["Visibility Unit"].toInt(), m_weatherVisibilityFormat), 'f', 1), WeatherUtils::getUnitString(m_weatherVisibilityFormat, false)));
        } else {
            dataVisibility->setText(i18n("Visibility: %1", data["Visibility"].toString()));
        }

        m_detailsModel->appendRow(dataVisibility);
    }

    if (data["Humidity"] != "N/A" && data["Humidity"].toString().isEmpty() == false) {
        QStandardItem *dataHumidity = new QStandardItem();
        dataHumidity->setText(i18n("Humidity: %1", data["Humidity"].toString()));
        m_detailsModel->appendRow(dataHumidity);
    }

    if (data["Wind Speed"] != "N/A" && data["Wind Speed"].toDouble() != 0 && data["Wind Speed"] != "Calm") {
        m_windIcon->setText(i18nc("wind direction, speed","%1 %2 %3", data["Wind Direction"].toString(),
                QString::number(WeatherUtils::convertSpeed(data["Wind Speed"].toDouble(), data["Wind Speed Unit"].toInt(), m_weatherWindFormat), 'f', 1), WeatherUtils::getUnitString(m_weatherWindFormat)));
    } else {
        if (data["Wind Speed"] == "N/A") {
            m_windIcon->setText(i18nc("Not available","N/A"));
        } else {
            if (data["Wind Speed"].toInt() == 0 || data["Wind Speed"] == "Calm") {
                m_windIcon->setText(i18nc("Wind condition","Calm"));
            }
        }
    }

    //m_windDirLabel->setText(i18n("Wind Direction: %1", data["Wind Direction"].toString()));
    m_windIcon->setSvg("weather/wind-arrows", data["Wind Direction"].toString());
    m_windIcon->setMaximumSize(m_windIcon->sizeFromIconSize(KIconLoader::SizeSmall));
    m_windIcon->update();

    if (data["Wind Gust"] != "N/A" && data["Wind Gust"].toString().isEmpty() == false) {
        // Convert the wind format for nonstandard types
        QStandardItem *dataGust = new QStandardItem();
        dataGust->setText(i18n("Wind Gust: %1 %2", QString::number(WeatherUtils::convertSpeed(data["Wind Gust"].toDouble(), data["Wind Gust Unit"].toInt(), m_weatherWindFormat), 'f', 1), WeatherUtils::getUnitString(m_weatherWindFormat)));
        m_detailsModel->appendRow(dataGust);
    }

    if (m_detailsModel->rowCount() > 0) {
        if (!m_detailsView->model()) {
            m_detailsView->setModel(m_detailsModel);
        }
        m_tabBar->addTab(i18n("Details"), m_detailsView);
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
            warningTitle->setText(i18n("Warnings Issued:"));
            warningTitle->nativeWidget()->setFont(noticeTitleFont);
            noticeLayout->addItem(warningTitle);
            for (int k = 0; k < data["Total Warnings Issued"].toInt(); k++) {
                Plasma::Label *warnNotice = new Plasma::Label();
                connect(warnNotice, SIGNAL(linkActivated(QString)), this, SLOT(weatherNoticeLink(QString)));
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
            watchTitle->setText(i18n("Watches Issued:"));
            watchTitle->nativeWidget()->setFont(noticeTitleFont);
            noticeLayout->addItem(watchTitle);
            for (int j = 0; j < data["Total Watches Issued"].toInt(); j++) {
                Plasma::Label *watchNotice = new Plasma::Label();
                connect(watchNotice, SIGNAL(linkActivated(QString)), this, SLOT(weatherNoticeLink(QString)));
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

        m_tabBar->addTab(i18n("Notices"), noticeLayout);
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
    setBusy(false);
    //updateGeometry();
}

void WeatherApplet::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (data.isEmpty()) {
        return;
    }

    QStringList tokens = data["validate"].toString().split('|');

    if (tokens.size() > 0 && data.contains("validate")) {
        weatherEngine->disconnectSource(source, this);
        if (tokens[1] == "valid" || tokens[1] == "invalid") {
            validate(source, data["validate"]);
            if (isBusy()) {
                setVisibleLayout(false);
                setBusy(false);
                setConfigurationRequired(true);
            }
       }
    } else {
        m_currentData = data;
        //setVisibleLayout(true);
        //setBusy(true);
        weatherContent(data);
    }

    update();
}

void WeatherApplet::configAccepted()
{
    KConfigGroup generalConfig = config();
    foreach(const QString& place, generalConfig.groupList()) {
        KConfigGroup placeConfig(&generalConfig, place);
        placeConfig.deleteGroup();
    }

    m_weatherWindFormat = uui.windOptionsComboList->itemData(uui.windOptionsComboList->currentIndex()).toInt();
    m_weatherTempFormat = uui.tempOptionsComboList->itemData(uui.tempOptionsComboList->currentIndex()).toInt();
    m_weatherPressureFormat = uui.pressureOptionsComboList->itemData(uui.pressureOptionsComboList->currentIndex()).toInt();
    m_weatherVisibilityFormat = uui.visibilityOptionsComboList->itemData(uui.visibilityOptionsComboList->currentIndex()).toInt();

    m_weatherUpdateTime = ui.weatherUpdateSpin->value();
    m_activePlace = ui.validatedPlaceLabel->text();

    m_activeIon = ui.pluginComboList->itemData(ui.pluginComboList->currentIndex()).toString();

    if (!m_activePlace.isEmpty()) {
        setVisibleLayout(false);

        //TODO: Don't reload date if data source and location aren't changed
        weatherEngine->disconnectSource(QString("%1|weather|%2").arg(m_activeIon).arg(m_activePlace), this);

        KConfigGroup placeConfig(&generalConfig, m_activePlace);
        placeConfig.writeEntry("ion", m_activeIon);
        if (!m_extraData[m_activePlace].isEmpty()) {
            placeConfig.writeEntry("data", m_extraData[m_activePlace]);
        }

        // Write out config options
        generalConfig.writeEntry("updateWeather", m_weatherUpdateTime);
        generalConfig.writeEntry("windFormat", m_weatherWindFormat);
        generalConfig.writeEntry("tempFormat", m_weatherTempFormat);
        generalConfig.writeEntry("pressureFormat", m_weatherPressureFormat);
        generalConfig.writeEntry("visibilityFormat", m_weatherVisibilityFormat);
        setConfigurationRequired(false);
        getWeather();
    } else {
        setVisibleLayout(false);
        setConfigurationRequired(true);
    }
}

#include "weatherapplet.moc"
