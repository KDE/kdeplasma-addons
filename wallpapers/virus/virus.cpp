/*
  Copyright (c) 2009 by Beat Wolf <asraniel@fryx.ch>
  Copyright (c) 2007 by Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (c) 2007 by Aaron Seigo <aseigo@kde.org>
  Copyright (c) 2008 by Alexis Ménard <darktears31@gmail.com>
  Copyright (c) 2008 by Petri Damsten <damu@iki.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "virus.h"

#include <QPainter>
#include <QFile>

#include <KDirSelectDialog>
#include <KDirWatch>
#include <KFileDialog>
#include <KGlobalSettings>
#include <KImageFilePreview>
#include <KNS/Engine>
#include <KProgressDialog>
#include <KRandom>
#include <KStandardDirs>

#include <Plasma/Theme>
#include <Plasma/Animator>
#include "backgroundlistmodel.h"
#include "backgrounddelegate.h"
//#include "ksmserver_interface.h"

K_EXPORT_PLASMA_WALLPAPER(virus, Virus)

Virus::Virus(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args),
      m_configWidget(0),
      m_currentSlide(-1),
      m_model(0),
      m_dialog(0),
      m_randomize(true),
      m_startupResumed(false)
{
    connect(this, SIGNAL(renderCompleted(QImage)), this, SLOT(updateBackground(QImage)));
    connect(&alife, SIGNAL(finished()), this, SLOT(virusUpdated()));
    
    //m_timer = QTimer(this);
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(requestUpdate()));
}

Virus::~Virus()
{
    alife.exit(0);
    alife.wait();
}

void Virus::init(const KConfigGroup &config)
{
    m_timer.stop();
    m_mode = "SingleImage";//renderingMode().name();
    calculateGeometry();

    m_resizeMethod = (ResizeMethod)config.readEntry("wallpaperposition", (int)ScaledResize);
    m_wallpaper = config.readEntry("wallpaper", QString());
    if (m_wallpaper.isEmpty()) {
        m_wallpaper = Plasma::Theme::defaultTheme()->wallpaperPath();
        int index = m_wallpaper.indexOf("/contents/images/");
        if (index > -1) { // We have file from package -> get path to package
            m_wallpaper = m_wallpaper.left(index);
        }
    }

    m_color = config.readEntry("wallpapercolor", QColor(56, 111, 150));
    m_usersWallpapers = config.readEntry("userswallpapers", QStringList());

    alife.setUpdateInterval(config.readEntry("updateinterval", 200));
    alife.setMaxViruses(config.readEntry("maxcells", 2000));
    alife.setShowCells(config.readEntry("showcells", true));

    if (m_dirs.isEmpty()) {
        m_dirs << KStandardDirs::installPath("wallpaper");
    }

    setUsingRenderingCache(false);

    setSingleImage();
}

void Virus::requestUpdate()
{
    alife.start();
}

void Virus::virusUpdated()
{
    m_pixmap = QPixmap::fromImage(alife.currentImage());
    emit update(boundingRect());
    m_timer.start(alife.getUpdateInterval());
}


void Virus::save(KConfigGroup &config)
{
    config.writeEntry("wallpaperposition", (int)m_resizeMethod);
    config.writeEntry("wallpaper", m_wallpaper);
    config.writeEntry("wallpapercolor", m_color);
    config.writeEntry("userswallpapers", m_usersWallpapers);
    config.writeEntry("updateinterval", alife.getUpdateInterval());
    config.writeEntry("maxcells",alife.getMaxViruses());
    config.writeEntry("showcells",alife.showCells());
}

void Virus::configWidgetDestroyed()
{
    m_configWidget = 0;
}

QWidget* Virus::createConfigurationInterface(QWidget* parent)
{
    m_configWidget = new QWidget(parent);
    connect(m_configWidget, SIGNAL(destroyed(QObject*)), this, SLOT(configWidgetDestroyed()));

    m_uiVirus.setupUi(m_configWidget);

    qreal ratio = m_size.isEmpty() ? 1.0 : m_size.width() / qreal(m_size.height());
    m_model = new BackgroundListModel(ratio, this);
    m_model->setResizeMethod(m_resizeMethod);
    m_model->setWallpaperSize(m_size);
    m_model->reload(m_usersWallpapers);
    m_uiVirus.m_view->setModel(m_model);
    m_uiVirus.m_view->setItemDelegate(new BackgroundDelegate(m_uiVirus.m_view->view(),
							      ratio, this));
    m_uiVirus.m_view->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    int index = m_model->indexOf(m_wallpaper);
    if (index != -1) {
	m_uiVirus.m_view->setCurrentIndex(index);
	Plasma::Package *b = m_model->package(index);
	if (b) {
	    fillMetaInfo(b);
	}
    }
    connect(m_uiVirus.m_view, SIGNAL(currentIndexChanged(int)), this, SLOT(pictureChanged(int)));

    m_uiVirus.m_pictureUrlButton->setIcon(KIcon("document-open"));
    connect(m_uiVirus.m_pictureUrlButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));

    m_uiVirus.m_emailLine->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_uiVirus.m_resizeMethod->addItem(i18n("Scaled & Cropped"), ScaledAndCroppedResize);
    m_uiVirus.m_resizeMethod->addItem(i18n("Scaled"), ScaledResize);
    m_uiVirus.m_resizeMethod->addItem(i18n("Scaled, keep proportions"), MaxpectResize);
    m_uiVirus.m_resizeMethod->addItem(i18n("Centered"), CenteredResize);
    m_uiVirus.m_resizeMethod->addItem(i18n("Tiled"), TiledResize);
    m_uiVirus.m_resizeMethod->addItem(i18n("Center Tiled"), CenterTiledResize);
    for (int i = 0; i < m_uiVirus.m_resizeMethod->count(); ++i) {
	if (m_resizeMethod == m_uiVirus.m_resizeMethod->itemData(i).value<int>()) {
	    m_uiVirus.m_resizeMethod->setCurrentIndex(i);
	    break;
	}
    }
    connect(m_uiVirus.m_resizeMethod, SIGNAL(currentIndexChanged(int)),
	    this, SLOT(positioningChanged(int)));

    m_uiVirus.m_color->setColor(m_color);
    connect(m_uiVirus.m_color, SIGNAL(changed(const QColor&)), this, SLOT(colorChanged(const QColor&)));

    m_uiVirus.m_maxCells->setValue(alife.getMaxViruses());
    connect(m_uiVirus.m_maxCells, SIGNAL(valueChanged(int)), this, SLOT(maxCellsChanged(int)));

    m_uiVirus.m_updateInterval->setValue(alife.getUpdateInterval());
    connect(m_uiVirus.m_updateInterval, SIGNAL(valueChanged(int)), this, SLOT(intervalChanged(int)));

    m_uiVirus.m_showCells->setChecked(alife.showCells());
    connect(m_uiVirus.m_showCells, SIGNAL(stateChanged(int)), this, SLOT(showCellsChanged(int)));

    connect(m_uiVirus.m_newStuff, SIGNAL(clicked()), this, SLOT(getNewWallpaper()));
    
    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
    
    return m_configWidget;
}

void Virus::calculateGeometry()
{
    m_size = boundingRect().size().toSize();

    if (m_model) {
        m_model->setWallpaperSize(m_size);
    }
}

void Virus::paint(QPainter *painter, const QRectF& exposedRect)
{
    // Check if geometry changed
    //kDebug() << m_size << boundingRect().size().toSize();
    if (m_size != boundingRect().size().toSize()) {
        calculateGeometry();
        if (!m_size.isEmpty() && !m_img.isEmpty()) { // We have previous image
            renderWallpaper();
            //kDebug() << "re-rendering";
            return;
        }
    }

    if (m_pixmap.isNull()) {
        painter->fillRect(exposedRect, QBrush(m_color));
        //kDebug() << "pixmap null";
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
}

void Virus::setSingleImage()
{
    if (m_wallpaper.isEmpty()) {
        return;
    }

    QString img;
    Plasma::Package b(m_wallpaper, packageStructure(this));

    img = b.filePath("preferred");
    kDebug() << img << m_wallpaper;
    if (img.isEmpty()) {
        img = m_wallpaper;
    }

    if (!m_size.isEmpty()) {
        renderWallpaper(img);
    }
}

void Virus::getNewWallpaper()
{
    KNS::Engine engine(m_configWidget);
    if (engine.init("virus_wallpaper.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(m_configWidget);

        if (entries.size() > 0 && m_model) {
            m_model->reload();
        }
    }
}

void Virus::colorChanged(const QColor& color)
{
    m_color = color;
    setSingleImage();
    emit settingsChanged(true);
}

void Virus::pictureChanged(int index)
{
    if (index == -1 || !m_model) {
        return;
    }

    Plasma::Package *b = m_model->package(index);
    if (!b) {
        return;
    }

    fillMetaInfo(b);
    if (b->structure()->contentsPrefix().isEmpty()) {
        // it's not a full package, but a single paper
        m_wallpaper = b->filePath("preferred");
    } else {
        m_wallpaper = b->path();
    }

    setSingleImage();
}


void Virus::maxCellsChanged(int maxCells)
{
    alife.setMaxViruses(maxCells);
    emit settingsChanged(true);
}

void Virus::intervalChanged(int interval)
{
    alife.setUpdateInterval(interval);
    emit settingsChanged(true);
}

void Virus::showCellsChanged(int state)
{
    alife.setShowCells(state == Qt::Checked);
    emit settingsChanged(true);
}

void Virus::positioningChanged(int index)
{
    m_resizeMethod = (ResizeMethod)m_uiVirus.m_resizeMethod->itemData(index).value<int>();
    setSingleImage();

    setResizeMethodHint(m_resizeMethod);

    if (m_model) {
        m_model->setResizeMethod(m_resizeMethod);
    }
    emit settingsChanged(true);
}

void Virus::fillMetaInfo(Plasma::Package *b)
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
        setMetadata(m_uiVirus.m_authorLine, QString());
        m_uiVirus.m_authorLabel->setAlignment(Qt::AlignLeft);
    } else {
        QString authorIntl = i18nc("Wallpaper info, author name", "%1", author);
        m_uiVirus.m_authorLabel->setAlignment(Qt::AlignRight);
        setMetadata(m_uiVirus.m_authorLine, authorIntl);
    }
    setMetadata(m_uiVirus.m_licenseLine, QString());
    setMetadata(m_uiVirus.m_emailLine, QString());
    m_uiVirus.m_emailLabel->hide();
    m_uiVirus.m_licenseLabel->hide();
}

bool Virus::setMetadata(QLabel *label, const QString &text)
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

void Virus::showFileDialog()
{
    if (!m_dialog) {
        m_dialog = new KFileDialog(KUrl(), "*.png *.jpeg *.jpg *.xcf *.svg *.svgz", m_configWidget);
        m_dialog->setOperationMode(KFileDialog::Opening);
        m_dialog->setInlinePreviewShown(true);
        m_dialog->setCaption(i18n("Select Wallpaper Image File"));
        m_dialog->setModal(false);
    }

    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();

    connect(m_dialog, SIGNAL(okClicked()), this, SLOT(browse()));
}

void Virus::browse()
{
    Q_ASSERT(m_model);

    QString wallpaper = m_dialog->selectedFile();
    disconnect(m_dialog, SIGNAL(okClicked()), this, SLOT(browse()));

    if (wallpaper.isEmpty()) {
        return;
    }

    if (m_model->contains(wallpaper)) {
        m_uiVirus.m_view->setCurrentIndex(m_model->indexOf(wallpaper));
        return;
    }

    // add background to the model
    m_model->addBackground(wallpaper);

    // select it
    int index = m_model->indexOf(wallpaper);
    if (index != -1) {
        m_uiVirus.m_view->setCurrentIndex(index);
    }
    // save it
    m_usersWallpapers << wallpaper;
}

void Virus::renderWallpaper(const QString& image)
{
    if (!image.isEmpty()) {
        m_img = image;
    }

    if (m_img.isEmpty()) {
        return;
    }

    render(m_img, m_size, m_resizeMethod, m_color);
    suspendStartup(true); // during KDE startup, make ksmserver until the wallpaper is ready
}

QString Virus::cacheId() const
{
    QSize s = boundingRect().size().toSize();
    return QString("%5_%3_%4_%1x%2").arg(s.width()).arg(s.height()).arg(m_color.name()).arg(m_resizeMethod).arg(m_img);
}

void Virus::updateBackground(const QImage &img)
{
    m_pixmap = QPixmap::fromImage(img);
    alife.setImage(m_pixmap.toImage());
    m_timer.start(alife.getUpdateInterval());
    emit update(boundingRect());
}

void Virus::suspendStartup(bool suspend)
{
    //TODO: find out how to compile with that ksmserver
    /*if (m_startupResumed) {
        return;
    }

    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());
    const QString startupID("desktop wallaper");
    if (suspend) {
        ksmserver.suspendStartup(startupID);
    } else {
        m_startupResumed = true;
        ksmserver.resumeStartup(startupID);
    }*/
}

void Virus::updateScreenshot(QPersistentModelIndex index)
{
    m_uiVirus.m_view->view()->update(index);
}

#include "virus.moc"
