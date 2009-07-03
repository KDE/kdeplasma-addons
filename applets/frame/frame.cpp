/***************************************************************************
 *   Copyright  2007 by Anne-Marie Mahfouf <annma@kde.org>                 *
 *   Copyright  2007 by Antonio Vinci <mercurio@personellarete.it>         *
 *   Copyright  2008 by Thomas Coopman <thomas.coopman@gmail.com>          *
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

#include "frame.h"

#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneDragDropEvent>
#include <QPen>
#include <QCheckBox>
#include <QTimer>
#include <QFileInfo>
#include <QStandardItemModel>

#include <KDebug>
#include <KConfigDialog>
#include <KSharedConfig>
#include <KRun>
#include <KUrl>
#include <KDirSelectDialog>
#include <KServiceTypeTrader>
#include <kglobalsettings.h>

#include <Plasma/PaintUtils>
#include <Plasma/DataEngine>

#include <math.h>

#include "configdialog.h"
#include "picture.h"
#include "slideshow.h"

Frame::Frame(QObject *parent, const QVariantList &args)
        : Plasma::Applet(parent, args),
        m_configDialog(0),
        m_openPicture(0)
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    resize(400, 300);
    m_mySlideShow = new SlideShow(this);
    if (args.count()) {
        m_currentUrl = args.value(0).toString();
    } else {
        m_currentUrl = KUrl("Default");
    }
}

Frame::~Frame()
{
}

void Frame::init()
{
    bool frameReceivedUrlArgs = false;
    if (m_currentUrl != KUrl("Default")) {
        frameReceivedUrlArgs = true;
    }

    m_slideNumber = 0;
    // Get config values
    KConfigGroup cg = config();
    m_frameColor = cg.readEntry("frameColor", QColor(70, 90, 130)); //theme?
    m_frame = cg.readEntry("frame", false);
    m_shadow = cg.readEntry("shadow", true);
    m_roundCorners = cg.readEntry("roundCorners", false);
    m_smoothScaling = cg.readEntry("smoothScaling", true);
    m_slideShow = cg.readEntry("slideshow", false);
    m_random = cg.readEntry("random", false);
    m_recursiveSlideShow = cg.readEntry("recursive slideshow", false);
    m_slideShowPaths = cg.readEntry("slideshow paths", QStringList());
    m_slideshowTime = cg.readEntry("slideshow time", 60); // default to 1 minute
    m_currentUrl = cg.readEntry("url", m_currentUrl);
    m_potdProvider = cg.readEntry("potdProvider", "");
    m_potd = cg.readEntry("potd", false);

    // Frame & Shadow dimensions
    m_frameOutline = 8;
    m_swOutline = 8;

    // Initialize the slideshow timer
    connect(m_mySlideShow, SIGNAL(pictureUpdated()), this, SLOT(updatePicture()));

    initSlideShow();
    if (frameReceivedUrlArgs) {
        cg.writeEntry("url", m_currentUrl);
        emit configNeedsSaving();
    }

    m_menuPresent = false;
}

void Frame::updateMenu()
{
    if (hasAuthorization("LaunchApp") && ! (m_menuPresent || m_potd || (m_currentUrl.path() == "Default" && m_mySlideShow->currentUrl() == "Default"))) {
        kDebug() << "Current url: " << m_currentUrl.url();
        m_openPicture = new QAction(SmallIcon("image-x-generic"), i18n("&Open Picture..."), this);       
        m_actions.append(m_openPicture);
        connect(m_openPicture, SIGNAL(triggered(bool)), this , SLOT(slotOpenPicture()));
        m_menuPresent = true;
    } else {
        if (m_menuPresent && m_potd) {
            m_actions.removeAll(m_openPicture);
            delete m_openPicture;
            m_openPicture = 0;
            m_menuPresent = false;
        }
    }
}

QList<QAction*> Frame::contextualActions()
{
    return m_actions;
}

void Frame::slotOpenPicture()
{
    if (!hasAuthorization("LaunchApp")) {
        return;
    }
    KUrl url;

    if (m_slideShow) {
        url = m_mySlideShow->currentUrl();
    } else {
        url = m_currentUrl;
    }

    if (url.path() != "Default") {
        new KRun(url, 0);
    }
}

void Frame::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        setBackgroundHints(Plasma::Applet::NoBackground);
        if (formFactor() == Plasma::Horizontal) {
            m_frameOutline = 0;
            m_swOutline = 4;
        } else if (formFactor() == Plasma::Vertical) {
            m_frameOutline = 0;
            m_swOutline = 4;
        } else {
            m_frameOutline = 8;
            m_swOutline = 8;
            //Restore widget geometry to image proportions
            resize(contentSizeHint());
            emit appletTransformedItself();
        }

        updatePicture();
    }

    if (constraints & Plasma::SizeConstraint) {
        //If on panel, keep geometry to 4:3 ratio
        if(formFactor() == Plasma::Vertical) {
            setMinimumSize(QSizeF(0, boundingRect().width()/1.33));
            setMaximumSize(QSizeF(-1, boundingRect().width()/1.33));
        } else if(formFactor() == Plasma::Horizontal) {
            setMinimumSize(QSizeF(boundingRect().height()*1.33,0));
            setMaximumSize(QSizeF(boundingRect().height()*1.33,-1));
        } else {
            setMinimumSize(QSizeF());
            setMaximumSize(QSizeF());
        }
    }
}

QSizeF Frame::contentSizeHint() const
{
    if (!m_pictureSize.isEmpty() && (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter)){
        const qreal maxSize = geometry().width() > geometry().height() ? geometry().width() : geometry().height();
        QSize size = m_pictureSize;
        size.scale(maxSize, maxSize, Qt::KeepAspectRatio);
        return size;
    } else {
        return geometry().size();
    }
}

void Frame::updatePicture()
{
    QImage picture = m_mySlideShow->image();
    m_pictureSize = picture.size();
    QSizeF sizeHint = contentSizeHint();
    if (geometry().size() != sizeHint) {
        resize(sizeHint);
        emit appletTransformedItself();
    }
    updateMenu();
    update();
}

void Frame::addDir()
{
    KDirSelectDialog dialog(KUrl(), true);
    if (dialog.exec()) {
        QString path = dialog.url().url();
        if (!m_slideShowPaths.contains(path)) {
            m_configDialog->imageUi.slideShowDirList->addItem(path);
        }
        updateButtons();
    }
}

void Frame::removeDir()
{
    int row = m_configDialog->imageUi.slideShowDirList->currentRow();
    if (row != -1) {
        m_configDialog->imageUi.slideShowDirList->takeItem(row);
        updateButtons();
    }
}

void Frame::updateButtons()
{
    int row = m_configDialog->imageUi.slideShowDirList->currentRow();
    m_configDialog->imageUi.removeDirButton->setEnabled(row != -1);
}

void Frame::createConfigurationInterface(KConfigDialog *parent)
{
    m_configDialog = new ConfigDialog(parent);

    KService::List services = KServiceTypeTrader::self()->query("PlasmaPoTD/Plugin");
    foreach(const KService::Ptr &service, services) {
        const QString service_name(service->name());
        const QVariant service_identifier(service->property("X-KDE-PlasmaPoTDProvider-Identifier", QVariant::String).toString());
        m_configDialog->imageUi.potdComboBox->insertItem(m_configDialog->imageUi.potdComboBox->count(), service_name, service_identifier);
    }

    QStandardItemModel* model = static_cast<QStandardItemModel*>(m_configDialog->imageUi.pictureComboBox->model());
    QStandardItem* item = model->item(2);

    if (item) {
        if (services.isEmpty())
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        else
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
    }

    parent->addPage(m_configDialog->imageSettings, i18n("Image"), icon());
    parent->addPage(m_configDialog->appearanceSettings, i18n("Appearance"), icon());
    parent->setDefaultButton(KDialog::Ok);
    parent->showButtonSeparator(true);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    connect(m_configDialog->imageUi.removeDirButton, SIGNAL(clicked()), this, SLOT(removeDir()));
    connect(m_configDialog->imageUi.addDirButton, SIGNAL(clicked()), this, SLOT(addDir()));
    connect(m_configDialog->imageUi.slideShowDirList, SIGNAL(currentRowChanged(int)), this, SLOT(updateButtons()));

    m_configDialog->setRoundCorners(m_roundCorners);
    m_configDialog->setSmoothScaling(m_smoothScaling);
    m_configDialog->setShadow(m_shadow);
    m_configDialog->setShowFrame(m_frame);
    m_configDialog->setFrameColor(m_frameColor);

    if (m_slideShow) {
        m_configDialog->imageUi.pictureComboBox->setCurrentIndex(1);
    } else if (m_potd) {
        m_configDialog->imageUi.pictureComboBox->setCurrentIndex(2);
    } else {
        m_configDialog->imageUi.pictureComboBox->setCurrentIndex(0);
    }

    m_configDialog->imageUi.randomCheckBox->setCheckState(m_random ? Qt::Checked : Qt::Unchecked);
    m_configDialog->imageUi.recursiveCheckBox->setCheckState(m_recursiveSlideShow ? Qt::Checked : Qt::Unchecked);

    m_configDialog->imageUi.potdComboBox->setCurrentIndex(m_configDialog->imageUi.potdComboBox->findData(m_potdProvider));

    m_configDialog->setCurrentUrl(m_currentUrl);
    m_configDialog->imageUi.slideShowDirList->clear();
    m_configDialog->imageUi.slideShowDirList->addItems(m_slideShowPaths);
    m_configDialog->imageUi.removeDirButton->setEnabled(!m_slideShowPaths.isEmpty());
    m_configDialog->imageUi.slideShowDelay->setTime(QTime(m_slideshowTime / 3600, (m_slideshowTime / 60) % 60, m_slideshowTime % 60));
    m_configDialog->previewPicture(m_mySlideShow->image());
}

void Frame::configAccepted()
{
    KConfigGroup cg = config();
    // Appearance
    m_roundCorners = m_configDialog->roundCorners();
    cg.writeEntry("roundCorners", m_roundCorners);
    m_smoothScaling = m_configDialog->smoothScaling();
    cg.writeEntry("smoothScaling", m_smoothScaling);
    m_shadow = m_configDialog->shadow();
    cg.writeEntry("shadow", m_shadow);
    m_frame = m_configDialog->showFrame();
    cg.writeEntry("frame", m_frame);
    m_frameColor = m_configDialog->frameColor();
    cg.writeEntry("frameColor", m_frameColor);

    bool wasPotd = m_potd;

    if (m_configDialog->imageUi.pictureComboBox->currentIndex() == 1) {
        m_slideShow = true;
        m_potd = false;
    }  else if (m_configDialog->imageUi.pictureComboBox->currentIndex() == 2)   {
        m_slideShow = false;
        m_potd = true;
    }  else {
        m_slideShow = false;
        m_potd = false;
    }

    m_random = m_configDialog->random();
    cg.writeEntry("random", m_random);
    m_currentUrl = m_configDialog->currentUrl();
    cg.writeEntry("url", m_currentUrl);
    cg.writeEntry("slideshow", m_slideShow);
    m_recursiveSlideShow = m_configDialog->imageUi.recursiveCheckBox->checkState() == Qt::Checked ? true : false;
    cg.writeEntry("recursive slideshow", m_recursiveSlideShow);
    m_slideShowPaths.clear();
    QStringList dirs;
    for (int i = 0; i < m_configDialog->imageUi.slideShowDirList->count(); i++) {
        m_slideShowPaths << m_configDialog->imageUi.slideShowDirList->item(i)->text();
    }
    cg.writeEntry("slideshow paths", m_slideShowPaths);

    QTime timerTime = m_configDialog->imageUi.slideShowDelay->time();
    m_slideshowTime = timerTime.second() + timerTime.minute() * 60 + timerTime.hour() * 3600;
    cg.writeEntry("slideshow time", m_slideshowTime);

    QString potdProvider = m_configDialog->imageUi.potdComboBox->itemData(m_configDialog->imageUi.potdComboBox->currentIndex()).toString();

    if ((wasPotd && !m_potd) || (m_potd && potdProvider != m_potdProvider)) {
        // if we go from potd to no potd, or if the provider changes, then we first want to
        // stop the potd engine
        stopPotd();
    }

    m_potdProvider = potdProvider;
    cg.writeEntry("potdProvider", m_potdProvider);
    cg.writeEntry("potd", m_potd);

    initSlideShow();
    
    // Creates the menu if the settings have changed from "Default" to sth. else
    updateMenu();

    emit configNeedsSaving();
}

void Frame::stopPotd()
{
    Plasma::DataEngine *engine = dataEngine("potd");
    QDate mCurrentDate = QDate::currentDate();
    const QString identifier = m_potdProvider + ':' + mCurrentDate.toString(Qt::ISODate);
    engine->disconnectSource(identifier, m_mySlideShow);
    m_potd = false;
}

void Frame::initSlideShow()
{
    m_mySlideShow->setUpdateInterval(0);

    if (m_slideShow) {
        m_mySlideShow->setDirs(m_slideShowPaths, m_recursiveSlideShow);
        m_mySlideShow->setRandom(m_random);
        m_mySlideShow->setUpdateInterval(m_slideshowTime * 1000);
    } else if (m_potd) {
        Plasma::DataEngine *engine = dataEngine("potd");
        QDate mCurrentDate = QDate::currentDate();
        const QString identifier = m_potdProvider + ':' + mCurrentDate.toString(Qt::ISODate);

        engine->connectSource(identifier, m_mySlideShow);

//FIXME: why is there a manual kicking of the engine?        const Plasma::DataEngine::Data data = engine->query(identifier);
    } else { //no slideshow so no random stuff
        m_mySlideShow->setRandom(false);
        m_mySlideShow->setImage(m_currentUrl.url());
    }

    if (!m_potd) {
        updatePicture();
    }
}

void Frame::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    // kDebug() << event->mimeData()->formats();
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void Frame::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    KUrl droppedUrl = (KUrl::List::fromMimeData(event->mimeData())).at(0);
    kDebug() << "dropped URL" << droppedUrl.url();
    if (droppedUrl.protocol() == "desktop") {
        KUrl tmpUrl = KGlobalSettings::desktopPath() + droppedUrl.path();
        droppedUrl = tmpUrl;
    }
    // If the url is a local directory start slideshowmode
    if (droppedUrl.isLocalFile() && QFileInfo(droppedUrl.path()).isDir()) {
        m_slideShowPaths.clear();
        m_slideShowPaths.append(droppedUrl.path());
        if (!m_slideShow) {
            m_slideShow = true;
        }
    } else {
        kDebug() << "Remote URL" << droppedUrl.url();
        m_currentUrl = droppedUrl;
        if (m_slideShow) {
            m_slideShow = false;
        }
    }

    stopPotd();
    initSlideShow();

    KConfigGroup cg = config();
    cg.writeEntry("url", m_currentUrl);
    cg.writeEntry("slideshow", m_slideShow);
    cg.writeEntry("slideshow paths", m_slideShowPaths);
    emit configNeedsSaving();
}

void Frame::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Q_UNUSED(option)
    if (m_slideShow) {
        // temporarily suspend the slideshow to allow time for loading the image
        m_mySlideShow->setUpdateInterval(0);
    }

    QImage picture = m_mySlideShow->image();
    const QSize contentsSize = rect.size();

    if (picture.isNull()) {
        if (m_slideShow) {
            // unsuspend the slideshow to allow time for loading the image
            m_mySlideShow->setUpdateInterval(m_slideshowTime * 1000);
        }
        return;
    }

    int roundingFactor = 12 * m_roundCorners;
    int swRoundness = roundingFactor + m_frameOutline / 2 * m_frame * m_roundCorners;

    QRect frameRect(QPoint(0, 0), contentsSize);
    frameRect.adjust(m_swOutline, m_swOutline, -m_swOutline, -m_swOutline); //Pretty useless.

    Qt::TransformationMode transformationMode = m_smoothScaling ? Qt::SmoothTransformation : Qt::FastTransformation;
    //TODO check if correct
    QImage scaledImage = picture.scaled(frameRect.size(), Qt::KeepAspectRatio, transformationMode);
    frameRect = QRect(QPoint(frameRect.x() + (frameRect.width() - scaledImage.width()) / 2,
                             frameRect.y() + (frameRect.height() - scaledImage.height()) / 2), scaledImage.size());

    QRect shadowRect;
    if (m_frame) {
        shadowRect = frameRect.adjusted(-m_frameOutline + 1, -m_frameOutline + 1,
                                        m_frameOutline - 1, m_frameOutline - 1);
    } else {
        shadowRect = frameRect;
    }

    // choose where to draw.

    // The frame path. It will be used to draw the frame and clip the image.
    QPainterPath framePath = Plasma::PaintUtils::roundedRectangle(frameRect, roundingFactor);

    p->setRenderHint(QPainter::SmoothPixmapTransform, true);
    p->setRenderHint(QPainter::Antialiasing, true);

    // Shadow
    // TODO faster. I'd like to use it on liveTransform.
    if (m_shadow) {
        p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
        p->setBrush(Qt::NoBrush);
        for (int i = 0; i <= m_swOutline; i += 1) {
            p->setOpacity(0.7 * exp(-(i / (double)(m_swOutline / 3))));
            QPainterPath tr = Plasma::PaintUtils::roundedRectangle(shadowRect, swRoundness + i);
            p->drawPath(tr);
            shadowRect.adjust(-1, -1, + 1, + 1);
        }
    }

    p->setBrush(Qt::NoBrush);

    // Frame
    if (m_frame) {
        p->setOpacity(0.5);
        p->setPen(QPen(m_frameColor, m_frameOutline * 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        p->drawPath(framePath);
    }

    p->setOpacity(1);

    // Picture
    p->save();
    if (m_roundCorners) {
        p->setClipPath(framePath);
    }

    // scale and center
    p->drawImage(frameRect, scaledImage);
    p->restore();

    // black frame
    if (m_frame) {
        p->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->drawPath(framePath);
    } else if (m_roundCorners) {
        p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->drawPath(framePath);
    }

    if (m_slideShow) {
        // unsuspend the slideshow to allow time for loading the image
        m_mySlideShow->setUpdateInterval(m_slideshowTime * 1000);
    }
}

#include "frame.moc"
