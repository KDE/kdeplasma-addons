/***************************************************************************
 *   Copyright (C) 2009 by Jonathan Thomas <echidnaman@kubuntu.org>        *
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "weatherwallpaper.h"

#include <QImage>
#include <QPainter>
#include <QStandardItemModel>

#include <KDialog>
#include <KFileDialog>
#include <KLocale>
#include <KDebug>
#include <KMessageBox>
#include <KNS/Engine>
#include <KStandardDirs>

#include <Plasma/Animator>
#include <Plasma/DataEngineManager>
#include <Plasma/Theme>

#include "backgroundlistmodel.h"
#include "backgrounddelegate.h"

#include "plasmaweather/weatherconfig.h"
#include "plasmaweather/weatherlocation.h"

K_EXPORT_PLASMA_WALLPAPER(weather, WeatherWallpaper)

WeatherWallpaper::WeatherWallpaper(QObject * parent, const QVariantList & args )
    : Plasma::Wallpaper(parent, args)
    , m_configWidget(0)
    , m_weatherLocation(0)
    , m_advancedDialog(0)
{
    connect(this, SIGNAL(renderCompleted(QImage)), this, SLOT(updateBackground(QImage)));
}

WeatherWallpaper::~WeatherWallpaper()
{

}

void WeatherWallpaper::init(const KConfigGroup & config)
{
    // Connect to weather engine.
    weatherEngine = dataEngine("weather");

    // Set custom weather options
    m_source = config.readEntry("source");
    m_weatherUpdateTime = config.readEntry("updateWeather", 30);

    m_color = config.readEntry("wallpapercolor", QColor(56, 111, 150));
    m_dir = KStandardDirs::installPath("wallpaper");
    m_usersWallpapers = config.readEntry("userswallpapers", QStringList());
    m_resizeMethod = (ResizeMethod)config.readEntry("wallpaperposition", (int)ScaledResize);

    if (!isInitialized()) {
        // TODO - Find a better way to retrieve weather than by looking at the icon name...
        // Map each wallpaper to a weather condition
        m_weatherMap["weather-none-available"] = Plasma::Theme::defaultTheme()->wallpaperPath();
        m_weatherMap["weather-clear"] = m_dir + "Fields_of_Peace";
        m_weatherMap["weather-few-clouds"] = m_dir + "Evening";
        m_weatherMap["weather-clouds"] = m_dir + "Colorado_Farm";
        m_weatherMap["weather-many-clouds"] = m_dir + "Beach_Reflecting_Clouds";
        m_weatherMap["weather-showers"] = m_dir + "There_is_Rain_on_the_Table";
        m_weatherMap["weather-showers-scattered"] = m_dir + "There_is_Rain_on_the_Table";
        m_weatherMap["weather-rain"] = m_dir + "There_is_Rain_on_the_Table";
        m_weatherMap["weather-mist"] = m_dir + "Fresh_Morning";
        m_weatherMap["weather-storm"] = m_dir + "Lightning";
        m_weatherMap["weather-hail"] = m_dir + "Hail";
        m_weatherMap["weather-snow"] = m_dir + "Winter_Track";
        m_weatherMap["weather-snow-scattered"] = m_dir + "Winter_Track";
        m_weatherMap["weather-few-clouds-night"] = m_dir + "JK_Bridge_at_Night";
        m_weatherMap["weather-clouds-night"] = m_dir + "JK_Bridge_at_Night";
        m_weatherMap["weather-clear-night"] = m_dir + "At_Night";
        m_weatherMap["weather-freezing-rain"] = m_dir + "Icy_Tree";
        m_weatherMap["weather-snow-rain"] = m_dir + "Icy_Tree";
    }

    calculateGeometry();
    getWeather();
}

void WeatherWallpaper::save(KConfigGroup & config)
{
    QString oldSource(m_source);
    int oldInterval = m_weatherUpdateTime;
    
    if (m_configWidget) {
        m_source = m_configWidget->source();
        m_weatherUpdateTime = m_configWidget->updateInterval();
    }
    if (m_source != oldSource || m_weatherUpdateTime != oldInterval) {
        if (!oldSource.isEmpty()) {
            weatherEngine->disconnectSource(oldSource, this);
        }
        if (!m_source.isEmpty()) {
            getWeather();
        }
    }
    config.writeEntry("source", m_source);
    config.writeEntry("updateWeather", m_weatherUpdateTime);
    config.writeEntry("wallpaperposition", (int)m_resizeMethod);
    config.writeEntry("wallpapercolor", m_color);
    config.writeEntry("userswallpapers", m_usersWallpapers);
}

void WeatherWallpaper::configWidgetDestroyed()
{
    m_configWidget = 0;
}

void WeatherWallpaper::advancedDialogDestroyed()
{
    m_advancedDialog = 0;
    m_model = 0;
}

QWidget * WeatherWallpaper::createConfigurationInterface(QWidget * parent)
{
    m_configWidget = new WeatherConfig(parent);
    connect(m_configWidget, SIGNAL(destroyed(QObject*)), this, SLOT(configWidgetDestroyed()));
    m_configWidget->setDataEngine(weatherEngine);
    m_configWidget->setSource(m_source);
    m_configWidget->setUpdateInterval(m_weatherUpdateTime);
    m_configWidget->setConfigurableUnits(WeatherConfig::None);
    m_configWidget->setHeadersVisible(false);
    
    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
    connect(m_configWidget, SIGNAL(settingsChanged()), this, SIGNAL(settingsChanged()));
        
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
        m_wallpaper = Plasma::Theme::defaultTheme()->wallpaperPath();
    }

    QString img;
    Plasma::Package b(m_wallpaper, packageStructure(this));

    img = b.filePath("preferred");

    if (img.isEmpty()) {
        img = m_wallpaper;
    }

    if (!m_size.isEmpty()) {
        renderWallpaper(img);
    }
}

void WeatherWallpaper::showAdvancedDialog()
{
    if (m_advancedDialog == 0) {
        m_advancedDialog = new KDialog;
        m_advancedUi.setupUi(m_advancedDialog->mainWidget());
        m_advancedDialog->mainWidget()->layout()->setMargin(0);

        m_advancedDialog->setCaption(i18n("Advanced Wallpaper Settings"));
        m_advancedDialog->setButtons(KDialog::Ok | KDialog::Cancel);

        qreal ratio = m_size.isEmpty() ? 1.0 : m_size.width() / qreal(m_size.height());
        m_model = new BackgroundListModel(ratio, this, m_advancedDialog);
        m_model->setResizeMethod(m_resizeMethod);
        m_model->setWallpaperSize(m_size);
        m_model->reload(m_usersWallpapers);
        m_advancedUi.m_wallpaperView->setModel(m_model);
        m_advancedUi.m_wallpaperView->setItemDelegate(new BackgroundDelegate(m_advancedUi.m_wallpaperView->view(),
                                                                             ratio, m_advancedDialog));
        m_advancedUi.m_wallpaperView->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

        connect(m_advancedUi.m_conditionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(conditionChanged(int)));
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-clear"), i18n("Clear"), "weather-clear");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-few-clouds"), i18n("Partly Cloudy"), "weather-few-clouds");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-clouds"), i18n("Cloudy"), "weather-clouds");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-many-clouds"), i18n("Very Cloudy"), "weather-many-clouds");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-showers"), i18n("Showering"), "weather-showers");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-showers-scattered"), i18n("Scattered Showers"), "weather-showers-scattered");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-showers"), i18n("Rainy"), "weather-rain");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-mist"), i18n("Misty"), "weather-mist");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-storm"), i18n("Storming"), "weather-storm");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-hail"), i18n("Hailing"), "weather-hail");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-snow"), i18n("Snowing"), "weather-snow");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-snow-scattered"), i18n("Scattered Snow"), "weather-snow-scattered");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-few-clouds-night"), i18n("Partly Cloudy Night"), "weather-few-clouds-night");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-clouds-night"), i18n("Cloudy Night"), "weather-clouds-night");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-clear-night"), i18n("Clear Night"), "weather-clear-night");
        m_advancedUi.m_conditionCombo->addItem(KIcon("weather-snow-rain"), i18n("Mixed Precipitation"), "weather-snow-rain");


        connect(m_advancedUi.m_wallpaperView, SIGNAL(currentIndexChanged(int)), this, SLOT(pictureChanged(int)));

        m_advancedUi.m_pictureUrlButton->setIcon(KIcon("document-open"));
        connect(m_advancedUi.m_pictureUrlButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));

        m_advancedUi.m_emailLine->setTextInteractionFlags(Qt::TextSelectableByMouse);

        m_advancedUi.m_resizeMethod->addItem(i18n("Scaled & Cropped"), ScaledAndCroppedResize);
        m_advancedUi.m_resizeMethod->addItem(i18n("Scaled"), ScaledResize);
        m_advancedUi.m_resizeMethod->addItem(i18n("Scaled, keep proportions"), MaxpectResize);
        m_advancedUi.m_resizeMethod->addItem(i18n("Centered"), CenteredResize);
        m_advancedUi.m_resizeMethod->addItem(i18n("Tiled"), TiledResize);
        m_advancedUi.m_resizeMethod->addItem(i18n("Center Tiled"), CenterTiledResize);
        for (int i = 0; i < m_advancedUi.m_resizeMethod->count(); ++i) {
            if (m_resizeMethod == m_advancedUi.m_resizeMethod->itemData(i).value<int>()) {
                m_advancedUi.m_resizeMethod->setCurrentIndex(i);
                break;
            }
        }
        connect(m_advancedUi.m_resizeMethod, SIGNAL(currentIndexChanged(int)),
                this, SLOT(positioningChanged(int)));

        m_advancedUi.m_color->setColor(m_color);
        connect(m_advancedUi.m_color, SIGNAL(changed(const QColor&)), this, SLOT(colorChanged(const QColor&)));
        connect(m_advancedUi.m_newStuff, SIGNAL(clicked()), this, SLOT(getNewWallpaper()));
    }
    KDialog::centerOnScreen(m_advancedDialog);
    connect(m_advancedDialog, SIGNAL(destroyed(QObject*)), this, SLOT(advancedDialogDestroyed()));
    m_advancedDialog->show();
}

void WeatherWallpaper::getNewWallpaper()
{
    KNS::Engine engine(m_advancedDialog);
    if (engine.init("wallpaper.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(m_configWidget);
        if (entries.size() > 0 && m_model) {
            m_model->reload();
        }
    }
}

void WeatherWallpaper::colorChanged(const QColor& color)
{
    m_color = color;
    loadImage();
}

void WeatherWallpaper::pictureChanged(int index)
{
    if (index == -1 || !m_model) {
        return;
    }

    Plasma::Package *b = m_model->package(index);
    if (!b) {
        return;
    }

    QString conditionIndexValue = m_advancedUi.m_conditionCombo->itemData(m_advancedUi.m_conditionCombo->currentIndex()).toString();
    fillMetaInfo(b);
    if (b->structure()->contentsPrefix().isEmpty()) {
        // it's not a full package, but a single paper
        m_weatherMap[conditionIndexValue] = b->filePath("preferred");
    } else {
        m_weatherMap[conditionIndexValue] = b->path();
    }

    getWeather();
}

void WeatherWallpaper::conditionChanged(int index)
{
    if (index == -1) {
        return;
    }
    QString conditionIndexValue = m_advancedUi.m_conditionCombo->itemData(index).toString();
    QString paper = m_weatherMap.value(conditionIndexValue);
    kDebug() << "paper currently is:" << paper;

    // Set the wallpaper view to the current wallpaper for the condition we just changed to
    // FIXME In theory this is supposed to set the model index to that of the default wallpaper for the chosen
    // condition, but it isn't :/ (Quite possibly an obvious mistake on my part)
    int modelIndex = m_model->indexOf(paper);
    kDebug() << "modelIndex is equal to:" << modelIndex;
    if (modelIndex != -1) {
        m_advancedUi.m_wallpaperView->setCurrentIndex(modelIndex);
        Plasma::Package *b = m_model->package(modelIndex);
        if (b) {
            fillMetaInfo(b);
        }
    }
}

void WeatherWallpaper::positioningChanged(int index)
{
    m_resizeMethod = (ResizeMethod)m_advancedUi.m_resizeMethod->itemData(index).value<int>();
    loadImage();

    setResizeMethodHint(m_resizeMethod);

    if (m_model) {
        m_model->setResizeMethod(m_resizeMethod);
    }
}

void WeatherWallpaper::fillMetaInfo(Plasma::Package *b)
{
    // Prepare more user-friendly forms of some pieces of data.
    // - license by config is more a of a key value,
    //   try to get the proper name if one of known licenses.

    // not needed for now...
    //QString license = b->license();
    /*
    KAboutLicense knownLicense = KAboutLicense::byKeyword(license);
    if (knownLicense.key() != KAboutData::License_Custom) {
        license = knownLicense.name(KAboutData::ShortName);
    }
    */
    // - last ditch attempt to localize author's name, if not such by config
    //   (translators can "hook" names from outside if resolute enough).
    QString author = b->metadata().author();
    if (author.isEmpty()) {
        setMetadata(m_advancedUi.m_authorLine, QString());
        m_advancedUi.m_authorLabel->setAlignment(Qt::AlignLeft);
    } else {
        QString authorIntl = i18nc("Wallpaper info, author name", "%1", author);
        m_advancedUi.m_authorLabel->setAlignment(Qt::AlignRight);
        setMetadata(m_advancedUi.m_authorLine, authorIntl);
    }
    setMetadata(m_advancedUi.m_licenseLine, QString());
    setMetadata(m_advancedUi.m_emailLine, QString());
    m_advancedUi.m_emailLabel->hide();
    m_advancedUi.m_licenseLabel->hide();
}

bool WeatherWallpaper::setMetadata(QLabel *label, const QString &text)
{
    if (text.isEmpty()) {
        label->hide();
        return false;
    }
    else {
        label->show();
        label->setText(text);
        return true;
    }
}

void WeatherWallpaper::showFileDialog()
{
    if (!m_fileDialog) {
        m_fileDialog = new KFileDialog(KUrl(), "*.png *.jpeg *.jpg *.xcf *.svg *.svgz", m_advancedDialog);
        m_fileDialog->setOperationMode(KFileDialog::Opening);
        m_fileDialog->setInlinePreviewShown(true);
        m_fileDialog->setCaption(i18n("Select Wallpaper Image File"));
        m_fileDialog->setModal(false);
    }

    m_fileDialog->show();
    m_fileDialog->raise();
    m_fileDialog->activateWindow();

    connect(m_fileDialog, SIGNAL(okClicked()), this, SLOT(browse()));
}

void WeatherWallpaper::browse()
{
    Q_ASSERT(m_model);

    QString wallpaper = m_fileDialog->selectedFile();
    disconnect(m_fileDialog, SIGNAL(okClicked()), this, SLOT(browse()));

    if (wallpaper.isEmpty()) {
        return;
    }

    if (m_model->contains(wallpaper)) {
        m_advancedUi.m_wallpaperView->setCurrentIndex(m_model->indexOf(wallpaper));
        return;
    }

    // add background to the model
    m_model->addBackground(wallpaper);

    // select it
    int index = m_model->indexOf(wallpaper);
    if (index != -1) {
        m_advancedUi.m_wallpaperView->setCurrentIndex(index);
    }
    // save it
    m_usersWallpapers << wallpaper;
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
    if (m_source.isEmpty()) {
        if (m_weatherLocation) {
            // already tried to get default location
            // A location probably hasn't been configured, so call loadImage
            loadImage();
        } else {
            m_weatherLocation = new WeatherLocation(this);
            connect(m_weatherLocation, SIGNAL(finished(const QString&)),
                    this, SLOT(locationReady(const QString&)));
            m_weatherLocation->setDataEngines(dataEngine("geolocation"), weatherEngine);
            m_weatherLocation->getDefault();
        }
    } else {
        weatherEngine->connectSource(m_source, this, m_weatherUpdateTime * 60 * 1000);
    }
}

void WeatherWallpaper::locationReady(const QString &source)
{
    m_source = source;
    if (!m_source.isEmpty()) {
        if (m_configWidget) {
            m_configWidget->setSource(m_source);
        }
        getWeather();
    }
}

void WeatherWallpaper::updateBackground(const QImage &img)
{
    m_oldPixmap = m_pixmap;
    m_oldFadedPixmap = m_oldPixmap;
    m_pixmap = QPixmap::fromImage(img);

    kDebug() << "!m_oldPixmap.isNull() is:" << !m_oldPixmap.isNull();
    if (!m_oldPixmap.isNull()) {
        Plasma::Animator::self()->customAnimation(254, 1000, Plasma::Animator::EaseInCurve, this, "updateFadedImage");
    } else {
        emit update(boundingRect());
    }
}

void WeatherWallpaper::updateScreenshot(QPersistentModelIndex index)
{
    m_advancedUi.m_wallpaperView->view()->update(index);
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

void WeatherWallpaper::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)
    if (data.isEmpty()) {
        return;
    }

    kDebug() << "Current weather is:" << data["Condition Icon"].toString();
    QString paper = m_weatherMap.value(data["Condition Icon"].toString());
    kDebug() << "Paper is:" << paper;
    if (!paper.isEmpty()) {
        m_wallpaper = paper;
        loadImage();
    }
}

#include "weatherwallpaper.moc"
