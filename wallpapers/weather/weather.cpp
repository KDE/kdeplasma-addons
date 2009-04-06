/***************************************************************************
 *   Copyright (C) 2009 by Jonathan Thomas <echidnaman@kubuntu.org>        *
 *   This also happens to borrow heavily from the weather plasmoid ;-), so:*
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "weather.h"

#include <QImage>
#include <QPainter>
#include <QStandardItemModel>

#include <KDialog>
#include <KLocale>
#include <KDebug>
#include <KMessageBox>
#include <KStandardDirs>
#include <kdeversion.h>

#include <Plasma/Animator>
#include <Plasma/DataEngineManager>


WeatherWallpaper::WeatherWallpaper(QObject * parent, const QVariantList & args )
        : Plasma::Wallpaper(parent, args),
        m_addDialog(0),
        m_amodel(0)
{
    connect(this, SIGNAL(renderCompleted(QImage)), this, SLOT(updateBackground(QImage)));
}

WeatherWallpaper::~WeatherWallpaper()
{

}

void WeatherWallpaper::init(const KConfigGroup & config)
{
    m_weatherUpdateTime = config.readEntry("updateWeather", 30);

    // Connect to weather engine.
    weatherEngine = dataEngine("weather");

    // Set custom options
    m_ionPlugins = weatherEngine->query("ions");

    m_activePlace = config.readEntry("place");
    m_activeIon = config.readEntry("ion");
    m_extraData[m_activePlace] = config.readEntry("data");

    // TODO Hardcoding colors is ebil!
    m_color = QColor(56, 111, 150);
    m_resizeMethod = (ResizeMethod)config.readEntry("wallpaperposition", (int)ScaledResize);
    getWeather();

    if (m_dir.isEmpty()) {
        m_dir = KStandardDirs::installPath("wallpaper");
    }

    getWeather();                                                                             
    loadImage();                                                                              
    emit update(boundingRect());
}

void WeatherWallpaper::save(KConfigGroup & config)
{
    if (!m_activePlace.isEmpty()) {

        //TODO: Don't reload date if data source and location aren't changed
        kDebug() << "'this' is equal to: " << this;
        weatherEngine->disconnectSource(QString("%1|weather|%2").arg(m_activeIon).arg(m_activePlace), this);

        config.writeEntry("place", m_activePlace);
        config.writeEntry("ion", m_activeIon);
        if (!m_extraData[m_activePlace].isEmpty()) {
            config.writeEntry("data", m_extraData[m_activePlace]);
        }

        getWeather();
    } else {
          kDebug() << "Nothing got saved apparently";
    }

    config.writeEntry("updateWeather", m_weatherUpdateTime);
}

void WeatherWallpaper::configWidgetDestroyed()
{
    m_configWidget = 0;
}

QWidget * WeatherWallpaper::createConfigurationInterface(QWidget * parent)
{
    m_configWidget = new QWidget(parent);
    connect(m_configWidget, SIGNAL(destroyed(QObject*)), this, SLOT(configWidgetDestroyed()));
    locationsUi.setupUi(m_configWidget);

    locationsUi.validateButton->setEnabled(false);

    foreach(const QVariant& item, m_ionPlugins) {
        QStringList pluginInfo = item.toString().split("|");
        locationsUi.pluginComboList->addItem(pluginInfo[0], pluginInfo[1]);
    }
    locationsUi.pluginComboList->model()->sort(0, Qt::AscendingOrder);

    locationsUi.locationEdit->setTrapReturnKey(true);

    if (m_activeIon.isEmpty()) {
        locationsUi.pluginComboList->setCurrentIndex(0);
    } else {
        locationsUi.pluginComboList->setCurrentIndex(locationsUi.pluginComboList->findData(m_activeIon));
    }

    locationsUi.weatherUpdateSpin->setSuffix(ki18np(" minute", " minutes"));
    locationsUi.weatherUpdateSpin->setValue(m_weatherUpdateTime);
    locationsUi.validatedPlaceLabel->setText(m_activePlace);

    connect(locationsUi.validateButton, SIGNAL(clicked()), this, SLOT(getValidation()));
    connect(locationsUi.locationEdit, SIGNAL(textChanged(const QString &)), this, SLOT(placeEditChanged(const QString &)));
    connect(locationsUi.pluginComboList, SIGNAL(currentIndexChanged(int)), this, SLOT(pluginIndexChanged(int)));
    connect(locationsUi.locationEdit, SIGNAL(returnPressed()), this, SLOT(getValidation()));
    connect(locationsUi.weatherUpdateSpin, SIGNAL(valueChanged(int)), this, SLOT(spinValueChanged(int)));

    return m_configWidget;
}


void WeatherWallpaper::calculateGeometry()
{
    m_size = boundingRect().size().toSize();
}

void WeatherWallpaper::paint(QPainter * painter, const QRectF & exposedRect)
{
    // Check if geometry changed
    if (m_size != boundingRect().size().toSize()) {
        calculateGeometry();
        if (!m_size.isEmpty() && !m_img.isEmpty()) { // We have previous image
            renderWallpaper();
            return;
        }
    }

    if (m_pixmap.isNull()) {
        painter->fillRect(exposedRect, QBrush(m_color));
        return;
    }

    if (painter->worldMatrix() == QMatrix()) {
        // draw the background untransformed when possible;(saves lots of per-pixel-math)
        painter->resetTransform();
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    // for pixmaps we draw only the exposed part (untransformed since the
    // bitmapBackground already has the size of the viewport)
    painter->drawPixmap(exposedRect, m_pixmap, exposedRect.translated(-boundingRect().topLeft()));

    if (!m_oldFadedPixmap.isNull()) {
        // Put old faded image on top.
        painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter->drawPixmap(exposedRect, m_oldFadedPixmap,
                            exposedRect.translated(-boundingRect().topLeft()));
    }
}

void WeatherWallpaper::loadImage()
{
    if (m_wallpaper.isEmpty()) {
        return;
    }

    QString img;
    qreal ratio = m_size.isEmpty() ? 1.0 : m_size.width() / qreal(m_size.height());
    BackgroundPackage b(m_wallpaper, ratio);

    img = b.findBackground(m_size, m_resizeMethod); // isValid() returns true for jpg?

    if (img.isEmpty()) {
        img = m_wallpaper;
    }

    if (!m_size.isEmpty()) {
        renderWallpaper(img);
    }
}

void WeatherWallpaper::showAddPlaceDialog(const QStringList& tokens)
{
    if (m_addDialog == 0) {
        m_addDialog = new KDialog;
        addUi.setupUi(m_addDialog->mainWidget());
        m_addDialog->mainWidget()->layout()->setMargin(0);

        // Set up QListView with model/view
        m_amodel = new QStandardItemModel();
        addUi.foundPlacesListView->setModel(m_amodel);
        addUi.foundPlacesListView->setSelectionMode(QAbstractItemView::SingleSelection);

        addUi.foundPlacesListView->show();

        m_addDialog->setCaption(i18n("Found Places"));
        m_addDialog->setButtons(KDialog::Ok | KDialog::Cancel);
        m_addDialog->setButtonText(KDialog::Ok, i18n("&Add"));
        m_addDialog->setDefaultButton(KDialog::NoDefault);


        connect(m_addDialog, SIGNAL(okClicked()), this, SLOT(addPlace()));
        connect(m_addDialog, SIGNAL(cancelClicked()), this, SLOT(cancelAddClicked()));
        connect(addUi.foundPlacesListView, SIGNAL(doubleClicked(const QModelIndex &)), this , SLOT(selectPlace()));
    }
    bool placeflag = false;
    QStringList headers;
    m_amodel->clear();
    headers << i18n("Found Places");
    m_amodel->setHorizontalHeaderLabels(headers);
    m_amodel->setColumnCount(1);
    addUi.foundPlacesListView->setResizeMode(QListView::Adjust);

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


void WeatherWallpaper::renderWallpaper(const QString& image)
{
    if (!image.isEmpty()) {
        m_img = image;
    }

    if (m_img.isEmpty()) {
        return;
    }

    render(m_img, m_size, m_resizeMethod, m_color);
}

void WeatherWallpaper::getWeather()
{
    kDebug() << "m_extraData[m_activePlace].isEmpty():" << m_extraData[m_activePlace].isEmpty();
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

void WeatherWallpaper::updateBackground(const QImage &img)
{
    m_oldPixmap = m_pixmap;
    m_oldFadedPixmap = m_oldPixmap;
    m_pixmap = QPixmap::fromImage(img);

    if (!m_oldPixmap.isNull()) {
        Plasma::Animator::self()->customAnimation(254, 1000, Plasma::Animator::LinearCurve, this, "updateFadedImage");
    } else {
        emit update(boundingRect());
    }
}

void WeatherWallpaper::updateFadedImage(qreal frame)
{
    //If we are done, delete the pixmaps and don't draw.
    if (frame == 1) {
        m_oldFadedPixmap = QPixmap();
        m_oldPixmap = QPixmap();
        emit update(boundingRect());
        return;
    }

    //Create the faded image.
    m_oldFadedPixmap.fill(Qt::transparent);

    QPainter p;
    p.begin(&m_oldFadedPixmap);
    p.drawPixmap(0, 0, m_oldPixmap);

    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);  
    p.fillRect(m_oldFadedPixmap.rect(), QColor(0, 0, 0, 254 * (1-frame)));//255*((150 - frame)/150)));

    p.end();

    emit update(boundingRect());
}

void WeatherWallpaper::validate(const QString& source, const QVariant& data)
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
           locationsUi.validatedPlaceLabel->setText(tokens[4]);
           locationsUi.validateButton->setEnabled(false);
           m_activePlace = locationsUi.validatedPlaceLabel->text();
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

void WeatherWallpaper::selectPlace()
{
    addPlace();
    m_addDialog->close();
}

void WeatherWallpaper::getValidation()
{
    if (locationsUi.locationEdit->text().size() >= 3) {
        // Destroy all other datasources
        foreach (const QString& source, weatherEngine->sources()) {
            if (source != "ions") {
                weatherEngine->disconnectSource(source, this);
            }
        }

        QString ion = locationsUi.pluginComboList->itemData(locationsUi.pluginComboList->currentIndex()).toString();
        QString checkPlace = locationsUi.locationEdit->text();
        checkPlace[0] = checkPlace[0].toUpper();
        m_activeValidation = QString("%1|validate|%2").arg(ion).arg(checkPlace);
        weatherEngine->connectSource(m_activeValidation, this);
    }
}

void WeatherWallpaper::spinValueChanged(int interval)
{
    m_weatherUpdateTime = interval;
}

void WeatherWallpaper::addPlace()
{
    QModelIndex item = addUi.foundPlacesListView->currentIndex();

    m_activePlace = item.data().toString();
    m_activeIon = locationsUi.pluginComboList->itemData(locationsUi.pluginComboList->currentIndex()).toString();
    locationsUi.validatedPlaceLabel->setText(m_activePlace);
    locationsUi.validateButton->setEnabled(false);
}

void WeatherWallpaper::cancelAddClicked()
{
    weatherEngine->disconnectSource(m_activeValidation, this);
}

void WeatherWallpaper::pluginIndexChanged(int index)
{
   Q_UNUSED(index)

   locationsUi.validatedPlaceLabel->clear();
   m_activePlace.clear();
   if (locationsUi.locationEdit->text().size() < 3) {
       locationsUi.validateButton->setEnabled(false);
   } else {
       locationsUi.validateButton->setEnabled(true);
   }
   m_activeIon = locationsUi.pluginComboList->itemData(locationsUi.pluginComboList->currentIndex()).toString();
}

void WeatherWallpaper::placeEditChanged(const QString& text)
{
    if (text.size() < 3) {
        locationsUi.validateButton->setEnabled(false);
    } else {
        locationsUi.validateButton->setEnabled(true);
    }
}

void WeatherWallpaper::weatherContent(const Plasma::DataEngine::Data &data)
{
    // TODO - Find a better way to retrieve weather than by looking at the icon name...
    kDebug() << "Current weather is:" << data["Condition Icon"].toString();
    if (data["Condition Icon"].toString() == "N/A") {
        m_wallpaper = QString(m_dir + "Air");
        kDebug() << "Weather not available";
    } else if (data["Condition Icon"].toString() == "weather-clear") {
        m_wallpaper = QString(m_dir + "Fields_of_Peace");
    } else if (data["Condition Icon"].toString() == "weather-few-clouds" || data["Condition Icon"].toString() == "weather-clouds"){
        m_wallpaper = QString(m_dir + "Colorado_Farm");
    } else if (data["Condition Icon"].toString() == "weather-many-clouds") {
        m_wallpaper = QString(m_dir + "Blue_Sun");
    } else if (data["Condition Icon"].toString() == "weather-showers" || data["Condition Icon"].toString() == "weather-showers-scattered" || data["Condition Icon"].toString() == "weather-rain" ) {
        m_wallpaper = QString(m_dir + "There_is_Rain_on_the_Table");
    } else if (data["Condition Icon"].toString() == "weather-mist") {
        m_wallpaper = QString(m_dir + "Fresh_Morning");
    } else if (data["Condition Icon"].toString() == "weather-storm") {
        m_wallpaper = QString(m_dir + "Lightning");
    } else if (data["Condition Icon"].toString() == "weather-hail") {
        m_wallpaper = QString(m_dir + "Hail");
    } else if (data["Condition Icon"].toString() == "weather-snow" || data["Condition Icon"].toString() == "weather-snow-scattered") {
        m_wallpaper = QString(m_dir + "Winter_Track");
    } else if (data["Condition Icon"].toString() == "weather-few-clouds-night" || data["Condition Icon"].toString() == "weather-clouds-night") {
        m_wallpaper = QString(m_dir + "JK_Bridge_at_Night");
    } else if (data["Condition Icon"].toString() == "weather-clear-night") {
         m_wallpaper = QString(m_dir + "At_Night");
    } else if (data["Condition Icon"].toString() == "weather-freezing-rain" || data["Condition Icon"].toString() == "weather-snow-rain") {
        m_wallpaper = QString(m_dir + "Icy_Tree");
    }

    loadImage();
    emit update(boundingRect());
}

void WeatherWallpaper::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (data.isEmpty()) {
        return;
    }

    QStringList tokens = data["validate"].toString().split('|');

    if (tokens.size() > 0 && data.contains("validate")) {
        weatherEngine->disconnectSource(source, this);
        if (tokens[1] == "valid" || tokens[1] == "invalid") {
            validate(source, data["validate"]);
        }
    } else {
        m_currentData = data;
        weatherContent(data);
    }
}

#include "weather.moc"
