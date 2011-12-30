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

// Qt includes
#include <QPainter>
#include <QEasingCurve>
#include <QPropertyAnimation>

// KDE includes
#include <KFileDialog>
#include <KLocalizedString>
#include <KNS3/DownloadDialog>
#include <KPushButton>
#include <KStandardDirs>
#include <Plasma/Animator>
#include <Plasma/Theme>

// Libplasmaweather includes
#include "plasmaweather/weatherconfig.h"
#include "plasmaweather/weatherlocation.h"

// Own includes
#include "backgroundlistmodel.h"
#include "backgrounddelegate.h"

K_EXPORT_PLASMA_WALLPAPER(weather, WeatherWallpaper)

WeatherWallpaper::WeatherWallpaper(QObject * parent, const QVariantList & args )
    : Plasma::Wallpaper(parent, args)
    , m_configWidget(0)
    , m_weatherLocation(0)
    , m_advancedDialog(0)
    , m_fileDialog(0)
    , m_fadeValue(0)
    , m_animation(0)
    , m_model(0)
    , m_newStuffDialog(0)
{
    connect(this, SIGNAL(renderCompleted(QImage)), this, SLOT(updateBackground(QImage)));
}

WeatherWallpaper::~WeatherWallpaper()
{
    delete m_animation;
}

void WeatherWallpaper::init(const KConfigGroup & config)
{
    // Connect to weather engine.
    weatherEngine = dataEngine(QLatin1String( "weather" ));

    // Set custom weather options
    m_source = config.readEntry("source");
    m_weatherUpdateTime = config.readEntry("updateWeather", 30);

    m_color = config.readEntry("wallpapercolor", QColor(56, 111, 150));
    m_dir = KStandardDirs::installPath("wallpaper");
    m_usersWallpapers = config.readEntry("userswallpapers", QStringList());
    m_resizeMethod = (ResizeMethod)config.readEntry("wallpaperposition", (int)ScaledResize);

    m_animation = new QPropertyAnimation(this, "fadeValue");
    m_animation->setProperty("easingCurve", QEasingCurve::InQuad);
    m_animation->setProperty("duration", 1000);
    m_animation->setProperty("startValue", 0.0);
    m_animation->setProperty("endValue", 1.0);

    m_weatherMap[QLatin1String( "weather-clear" )] = config.readEntry("clearPaper", QString(m_dir + QLatin1String( "Fields_of_Peace/" )));
    m_weatherMap[QLatin1String( "weather-few-clouds" )] = config.readEntry("partlyCloudyPaper", QString(m_dir + QLatin1String( "Evening/" )));
    m_weatherMap[QLatin1String( "weather-clouds" )] = config.readEntry("cloudyPaper", QString(m_dir + QLatin1String( "Colorado_Farm/" )));
    m_weatherMap[QLatin1String( "weather-many-clouds" )] = config.readEntry("manyCloudsPaper", QString(m_dir + QLatin1String( "Beach_Reflecting_Clouds/" )));
    m_weatherMap[QLatin1String( "weather-showers" )] = config.readEntry("showersPaper", QString(m_dir + QLatin1String( "There_is_Rain_on_the_Table/" )));
    m_weatherMap[QLatin1String( "weather-showers-scattered" )] = config.readEntry("showersScatteredPaper", QString(m_dir + QLatin1String( "There_is_Rain_on_the_Table/" )));
    m_weatherMap[QLatin1String( "weather-rain" )] = config.readEntry("rainPaper", QString(m_dir + QLatin1String( "There_is_Rain_on_the_Table/" )));
    m_weatherMap[QLatin1String( "weather-mist" )] = config.readEntry("mistPaper", QString(m_dir + QLatin1String( "Fresh_Morning/" )));
    m_weatherMap[QLatin1String( "weather-storm" )] = config.readEntry("stormPaper", QString(m_dir + QLatin1String( "Storm/" )));
    m_weatherMap[QLatin1String( "weather-scattered-storms" )] = m_weatherMap[QLatin1String( "weather-storm" )];
    m_weatherMap[QLatin1String( "weather-hail" )] = config.readEntry("hailPaper", QString(m_dir + QLatin1String( "Storm/" )));
    m_weatherMap[QLatin1String( "weather-snow" )] = config.readEntry("snowPaper", QString(m_dir + QLatin1String( "Winter_Track/" )));
    m_weatherMap[QLatin1String( "weather-snow-scattered" )] = config.readEntry("snowScatteredPaper", QString(m_dir + QLatin1String( "Winter_Track/" )));
    m_weatherMap[QLatin1String( "weather-few-clouds-night" )] = config.readEntry("partlyCloudyNightPaper", QString(m_dir + QLatin1String( "JK_Bridge_at_Night/" )));
    m_weatherMap[QLatin1String( "weather-clouds-night" )] = config.readEntry("cloudyNightPaper", QString(m_dir + QLatin1String( "JK_Bridge_at_Night/" )));
    m_weatherMap[QLatin1String( "weather-clear-night" )] = config.readEntry("clearNightPaper", QString(m_dir + QLatin1String( "City_at_Night/" )));
    m_weatherMap[QLatin1String( "weather-freezing-rain" )] = config.readEntry("freezingRainPaper", QString(m_dir + QLatin1String( "Icy_Tree/" )));
    m_weatherMap[QLatin1String( "weather-snow-rain" )] = config.readEntry("snowRainPaper", QString(m_dir + QLatin1String( "Icy_Tree/" )));

    calculateGeometry();
    connectWeatherSource();
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
            connectWeatherSource();
        }
    }
    config.writeEntry("source", m_source);
    config.writeEntry("updateWeather", m_weatherUpdateTime);
    config.writeEntry("wallpaperposition", (int)m_resizeMethod);
    config.writeEntry("wallpapercolor", m_color);
    config.writeEntry("userswallpapers", m_usersWallpapers);
    // Save custom wallpaper/weather pairings
    config.writeEntry("clearPaper", m_weatherMap[QLatin1String( "weather-clear" )]);
    config.writeEntry("partlyCloudyPaper", m_weatherMap[QLatin1String( "weather-few-clouds" )]);
    config.writeEntry("cloudyPaper", m_weatherMap[QLatin1String( "weather-clouds" )]);
    config.writeEntry("manyCloudsPaper", m_weatherMap[QLatin1String( "weather-many-clouds" )]);
    config.writeEntry("showersPaper", m_weatherMap[QLatin1String( "weather-showers" )]);
    config.writeEntry("showersScatteredPaper", m_weatherMap[QLatin1String( "weather-showers-scattered" )]);
    config.writeEntry("rainPaper", m_weatherMap[QLatin1String( "weather-rain" )]);
    config.writeEntry("mistPaper", m_weatherMap[QLatin1String( "weather-mist" )]);
    config.writeEntry("stormPaper", m_weatherMap[QLatin1String( "weather-storm" )]);
    config.writeEntry("hailPaper", m_weatherMap[QLatin1String( "weather-hail" )]);
    config.writeEntry("snowPaper", m_weatherMap[QLatin1String( "weather-snow" )]);
    config.writeEntry("snowScatteredPaper", m_weatherMap[QLatin1String( "weather-snow-scattered" )]);
    config.writeEntry("partlyCloudyNightPaper", m_weatherMap[QLatin1String( "weather-few-clouds-night" )]);
    config.writeEntry("cloudyNightPaper", m_weatherMap[QLatin1String( "weather-clouds-night" )]);
    config.writeEntry("clearNightPaper", m_weatherMap[QLatin1String( "weather-clear-night" )]);
    config.writeEntry("freezingRainPaper", m_weatherMap[QLatin1String( "weather-freezing-rain" )]);
    config.writeEntry("snowRainPaper", m_weatherMap[QLatin1String( "weather-snow-rain" )]);
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
    QWidget *top = new QWidget(parent);
    QVBoxLayout *layout = new QVBoxLayout(top);
    layout->setMargin(0);
    m_configWidget = new WeatherConfig(top);
    connect(m_configWidget, SIGNAL(destroyed(QObject*)), this, SLOT(configWidgetDestroyed()));
    m_configWidget->setDataEngine(weatherEngine);
    m_configWidget->setSource(m_source);
    m_configWidget->setUpdateInterval(m_weatherUpdateTime);
    m_configWidget->setConfigurableUnits(WeatherConfig::None);
    m_configWidget->setHeadersVisible(false);
    layout->addWidget(m_configWidget);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    KPushButton *buttonAdvanced = new KPushButton(m_configWidget);
    buttonAdvanced->setText( i18n( "&Advanced..." ) );
    m_configWidget->layout()->addWidget(buttonAdvanced);
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonAdvanced);
    layout->addLayout(buttonLayout);

    connect(buttonAdvanced, SIGNAL(clicked()), this, SLOT(showAdvancedDialog()));
    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
    connect(m_configWidget, SIGNAL(settingsChanged()), this, SIGNAL(settingsChanged()));

    return top;
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
    m_wallpaper = m_weatherMap.value(m_condition);

    if (m_wallpaper.isEmpty()) {
       QHashIterator<QString, QString> it(m_weatherMap);
       while (it.hasNext()) {
           it.next();
           if (m_condition.startsWith(it.key())) {
               m_wallpaper = it.value();
               break;
           }
       }
    }

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
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-clear" )), i18nc("weather condition", "Clear"), QLatin1String( "weather-clear" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-few-clouds" )), i18n("Partly Cloudy"), QLatin1String( "weather-few-clouds" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-clouds") ), i18n("Cloudy"), QLatin1String( "weather-clouds") );
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-many-clouds") ), i18n("Very Cloudy"), QLatin1String( "weather-many-clouds") );
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-showers") ), i18n("Showering"), QLatin1String( "weather-showers") );
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-showers-scattered") ), i18n("Scattered Showers"), QLatin1String( "weather-showers-scattered" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-showers") ), i18n("Rainy"), QLatin1String( "weather-rain") );
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-mist") ), i18n("Misty"), QLatin1String( "weather-mist") );
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-storm") ), i18n("Storming"), QLatin1String( "weather-storm" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-hail") ), i18n("Hailing"), QLatin1String( "weather-hail" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-snow") ), i18n("Snowing"), QLatin1String( "weather-snow" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-snow-scattered") ), i18n("Scattered Snow"), QLatin1String( "weather-snow-scattered" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-few-clouds-night") ), i18n("Partly Cloudy Night"), QLatin1String( "weather-few-clouds-night" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-clouds-night") ), i18n("Cloudy Night"), QLatin1String( "weather-clouds-night" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-clear-night") ), i18n("Clear Night"), QLatin1String( "weather-clear-night" ));
        m_advancedUi.m_conditionCombo->addItem(KIcon(QLatin1String( "weather-snow-rain") ), i18n("Mixed Precipitation"), QLatin1String( "weather-snow-rain" ));
        // Set to the current weather condition
        m_advancedUi.m_conditionCombo->setCurrentIndex(m_advancedUi.m_conditionCombo->findData(m_condition));


        connect(m_advancedUi.m_wallpaperView, SIGNAL(currentIndexChanged(int)), this, SLOT(pictureChanged(int)));

        m_advancedUi.m_pictureUrlButton->setIcon(KIcon(QLatin1String( "document-open" )));
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
        m_advancedUi.m_newStuff->setIcon(KIcon(QLatin1String( "get-hot-new-stuff" )));
        connect(m_advancedUi.m_color, SIGNAL(changed(QColor)), this, SLOT(colorChanged(QColor)));
        connect(m_advancedUi.m_newStuff, SIGNAL(clicked()), this, SLOT(getNewWallpaper()));
    }
    KDialog::centerOnScreen(m_advancedDialog);
    connect(m_advancedDialog, SIGNAL(destroyed(QObject*)), this, SLOT(advancedDialogDestroyed()));
    m_advancedDialog->show();
}

void WeatherWallpaper::getNewWallpaper()
{
    if (!m_newStuffDialog) {
        m_newStuffDialog = new KNS3::DownloadDialog( QLatin1String( "wallpaper.knsrc" ), m_configWidget );
        connect(m_newStuffDialog, SIGNAL(accepted()), SLOT(newStuffFinished()));
    }
    m_newStuffDialog->show();
}

void WeatherWallpaper::newStuffFinished()
{
    if (m_model && m_newStuffDialog->changedEntries().size() > 0) {
        m_model->reload();
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

    loadImage();
}

void WeatherWallpaper::conditionChanged(int index)
{
    if (index == -1) {
        return;
    }
    QString conditionIndexValue = m_advancedUi.m_conditionCombo->itemData(index).toString();
    QString paper = m_weatherMap[conditionIndexValue];

    int modelIndex = m_model->indexOf(paper);
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
        m_fileDialog = new KFileDialog(KUrl(), QLatin1String( "*.png *.jpeg *.jpg *.xcf *.svg *.svgz" ), m_advancedDialog);
        m_fileDialog->setOperationMode(KFileDialog::Opening);
        m_fileDialog->setInlinePreviewShown(true);
        m_fileDialog->setCaption(i18n("Select Wallpaper Image File"));
        m_fileDialog->setModal(false);
    }

    m_fileDialog->show();
    m_fileDialog->raise();
    m_fileDialog->activateWindow();

    connect(m_fileDialog, SIGNAL(okClicked()), this, SLOT(wallpaperBrowseCompleted()));
    connect(m_fileDialog, SIGNAL(destroyed(QObject*)), this, SLOT(fileDialogFinished()));
}

void WeatherWallpaper::fileDialogFinished()
{
    m_fileDialog = 0;
}

void WeatherWallpaper::wallpaperBrowseCompleted()
{
    Q_ASSERT(m_model);

    const QFileInfo info(m_fileDialog->selectedFile());

    //the full file path, so it isn't broken when dealing with symlinks
    const QString wallpaper = info.canonicalFilePath();

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

void WeatherWallpaper::connectWeatherSource()
{
    if (m_source.isEmpty()) {
        // A location probably hasn't been configured, so call loadImage to load
        // the default wallpaper in case we can't guess later
        loadImage();
        // We can see if we can be nice and figure out where the user is
        m_weatherLocation = new WeatherLocation(this);
        connect(m_weatherLocation, SIGNAL(finished(QString)),
                this, SLOT(locationReady(QString)));
        m_weatherLocation->setDataEngines(dataEngine(QLatin1String( "geolocation" )), weatherEngine);
        m_weatherLocation->getDefault();
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
        connectWeatherSource();
    }
}

void WeatherWallpaper::updateBackground(const QImage &img)
{
    m_oldPixmap = m_pixmap;
    m_oldFadedPixmap = m_oldPixmap;
    m_pixmap = QPixmap::fromImage(img);

    if (!m_oldPixmap.isNull()) {
        m_animation->start();
    } else {
        emit update(boundingRect());
    }
}

void WeatherWallpaper::updateScreenshot(QPersistentModelIndex index)
{
    if (m_advancedDialog) {
        m_advancedUi.m_wallpaperView->view()->update(index);
    }
}

qreal WeatherWallpaper::fadeValue()
{
    return m_fadeValue;
}

void WeatherWallpaper::setFadeValue(qreal value)
{
    m_fadeValue = value;

    //If we are done, delete the pixmaps and don't draw.
    if (qFuzzyCompare(m_fadeValue, qreal(1.0))) {
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
    p.fillRect(m_oldFadedPixmap.rect(), QColor(0, 0, 0, 254 * (1-m_fadeValue)));//255*((150 - m_fadeValue)/150)));

    p.end();

    emit update(boundingRect());
}

void WeatherWallpaper::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    if (data.isEmpty()) {
        return;
    }

    m_condition = data[QLatin1String( "Condition Icon" )].toString();

    loadImage();
}

#include "weatherwallpaper.moc"
