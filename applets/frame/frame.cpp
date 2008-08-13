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
#include <KUrl>
#include <KDirSelectDialog>
#include <KServiceTypeTrader>

#include <plasma/paintutils.h>

#include <math.h>

#include "configdialog.h"
#include "picture.h"
#include "slideshow.h"

Frame::Frame(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_configDialog( 0 )
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);
    setCacheMode(QGraphicsItem::NoCache);
    resize(400, 300);
    m_mySlideShow = new SlideShow();
    if ( args.count() ) {
        m_currentUrl = args.value(0).toString();
    } else {
        m_currentUrl = KUrl("Default");
    }
}

Frame::~Frame()
{
    delete m_mySlideShow;
}

void Frame::dataUpdated( const QString &name, const Plasma::DataEngine::Data &data )
{
    QDate mCurrentDate = QDate::currentDate();
    const QString identifier = m_potdProvider + ':' + mCurrentDate.toString( Qt::ISODate );

    QImage _picture = data[ identifier ].value<QImage>();

    if ( !_picture.isNull() ) {
	m_picture = _picture;
	resize(contentSizeHint());
	m_pixmapCache = QPixmap();
        prepareGeometryChange();
        updateGeometry();
        update();
    }
}

void Frame::init()
{
    bool frameReceivedUrlArgs = false;
    if (m_currentUrl != KUrl("Default") ) {
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
    m_slideshowTime = cg.readEntry("slideshow time", 10); // default to 10 seconds
    m_currentUrl = cg.readEntry("url", m_currentUrl);
    m_potdProvider = cg.readEntry("potdProvider", "");
    m_potd = cg.readEntry("potd", false);

    // Frame & Shadow dimensions
    m_frameOutline = 8;
    m_swOutline = 8;

    // Initialize the slideshow timer
    m_slideShowTimer = new QTimer(this);
    connect(m_slideShowTimer, SIGNAL(timeout()), this, SLOT(updatePicture()));
    m_slideShowTimer->setInterval(m_slideshowTime * 1000);

    initSlideShow();
    if (frameReceivedUrlArgs) {
        cg.writeEntry("url", m_currentUrl);
        emit configNeedsSaving();
    }
}

void Frame::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        setBackgroundHints(Plasma::Applet::NoBackground);
    }
}

QSizeF Frame::contentSizeHint() const {
    if (!m_picture.isNull()) {
        QSizeF sizeHint;
        qreal maxSize = geometry().width() > geometry().height() ? geometry().width() : geometry().height();
        if (m_picture.width() > m_picture.height()) {
            sizeHint = QSizeF( maxSize, (maxSize / m_picture.width()) * m_picture.height() );
        } else {
            sizeHint = QSizeF( (maxSize / m_picture.height()) * m_picture.width(), maxSize );
        }
        return sizeHint;
    } else {
        return geometry().size();
    }
}

void Frame::updatePicture()
{
    if ( !m_picture.isNull() ) {
        m_picture = m_mySlideShow->getImage();
        resize(contentSizeHint());
        m_pixmapCache = QPixmap();
        update();
    }
}

void Frame::addDir()
{
    KDirSelectDialog dialog(KUrl(), true);
    if (dialog.exec()) {
        QString path = dialog.url().path();
        if (!m_slideShowPaths.contains(path)) {
            m_configDialog->ui.slideShowDirList->addItem(path);
        }
        m_configDialog->ui.removeDirButton->setEnabled(true);
    }
}

void Frame::removeDir()
{
    int row = m_configDialog->ui.slideShowDirList->currentRow();
    if (row != -1) {
        m_configDialog->ui.slideShowDirList->takeItem(row);
        if (m_configDialog->ui.slideShowDirList->count() == 0) {
            m_configDialog->ui.removeDirButton->setEnabled(false);
        }
    }
}

void Frame::createConfigurationInterface(KConfigDialog *parent)
{
    m_configDialog = new ConfigDialog( parent );

    KService::List services = KServiceTypeTrader::self()->query( "PlasmaPoTD/Plugin");
    foreach (const KService::Ptr &service, services) {
	const QString service_name( service->name() );
	const QVariant service_identifier( service->property( "X-KDE-PlasmaPoTDProvider-Identifier", QVariant::String ).toString() );
	m_configDialog->ui.potdComboBox->insertItem( m_configDialog->ui.potdComboBox->count(), service_name, service_identifier );
    }

    QStandardItemModel* model = static_cast<QStandardItemModel*>( m_configDialog->ui.pictureComboBox->model() );
    QStandardItem* item = model->item( 2 );
    if ( services.isEmpty() )
        item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
    else
        item->setFlags( item->flags() | Qt::ItemIsEnabled );

    parent->setButtons(  KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    parent->addPage( m_configDialog, parent->windowTitle(), icon() );
    parent->setDefaultButton( KDialog::Ok );
    parent->showButtonSeparator( true );
    connect( parent, SIGNAL( applyClicked() ), this, SLOT( configAccepted() ) );
    connect( parent, SIGNAL( okClicked() ), this, SLOT( configAccepted() ) );

    connect(m_configDialog->ui.removeDirButton, SIGNAL(clicked()), this, SLOT(removeDir()));
    connect(m_configDialog->ui.addDirButton, SIGNAL(clicked()), this, SLOT(addDir()));

    m_configDialog->setRoundCorners( m_roundCorners );
    m_configDialog->setSmoothScaling( m_smoothScaling );
    m_configDialog->setShadow(m_shadow);
    m_configDialog->setShowFrame(m_frame);
    m_configDialog->setFrameColor(m_frameColor);

    if (m_slideShow)
	m_configDialog->ui.pictureComboBox->setCurrentIndex(1);
    else if (m_potd)
	m_configDialog->ui.pictureComboBox->setCurrentIndex(2);
    else
	m_configDialog->ui.pictureComboBox->setCurrentIndex(0);

    m_configDialog->ui.randomCheckBox->setCheckState(m_random ? Qt::Checked : Qt::Unchecked);
    m_configDialog->ui.recursiveCheckBox->setCheckState(m_recursiveSlideShow ? Qt::Checked : Qt::Unchecked);

    m_configDialog->ui.potdComboBox->setCurrentIndex( m_configDialog->ui.potdComboBox->findData(m_potdProvider) );

    m_configDialog->setCurrentUrl(m_currentUrl);
    m_configDialog->ui.slideShowDirList->clear();
    m_configDialog->ui.slideShowDirList->addItems(m_slideShowPaths);
    m_configDialog->ui.removeDirButton->setEnabled(!m_slideShowPaths.isEmpty());
    m_configDialog->ui.slideShowDelay->setTime(QTime(m_slideshowTime / 3600, (m_slideshowTime / 60) % 60, m_slideshowTime % 60));
    m_configDialog->previewPicture(m_picture);
    m_configDialog->show();
    m_configDialog->raise();
}

void Frame::configAccepted()
{
    prepareGeometryChange();
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

    if (m_configDialog->ui.pictureComboBox->currentIndex() == 1)
    {
	m_slideShow = true;
	m_potd = false;
    }  else if (m_configDialog->ui.pictureComboBox->currentIndex() == 2)   {
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
    m_recursiveSlideShow = m_configDialog->ui.recursiveCheckBox->checkState() == Qt::Checked ? true : false;
    cg.writeEntry("recursive slideshow", m_recursiveSlideShow);
    m_slideShowPaths.clear();
    QStringList dirs;
    for (int i = 0; i < m_configDialog->ui.slideShowDirList->count(); i++) {
        m_slideShowPaths << m_configDialog->ui.slideShowDirList->item(i)->text();
    }
    cg.writeEntry("slideshow paths", m_slideShowPaths);

    QTime timerTime = m_configDialog->ui.slideShowDelay->time();
    m_slideshowTime = timerTime.second() + timerTime.minute() * 60 + timerTime.hour() * 3600;
    m_slideShowTimer->setInterval(m_slideshowTime * 1000);
    cg.writeEntry("slideshow time", m_slideshowTime);

    m_potdProvider = m_configDialog->ui.potdComboBox->itemData(m_configDialog->ui.potdComboBox->currentIndex()).toString();
    cg.writeEntry("potdProvider", m_potdProvider);
    cg.writeEntry("potd", m_potd);

    initSlideShow();

    emit configNeedsSaving();
}

void Frame::initSlideShow()
{
    if (m_slideShow) {
	    m_mySlideShow->setDirs(m_slideShowPaths, m_recursiveSlideShow);
            m_mySlideShow->setRandom(m_random);
	    m_slideShowTimer->start();
    } else if (m_potd) {
	Plasma::DataEngine *engine = dataEngine( "potd" );
	if ( !engine )
	    return;

	QDate mCurrentDate = QDate::currentDate();
	const QString identifier = m_potdProvider + ':' + mCurrentDate.toString( Qt::ISODate );

	engine->disconnectSource( identifier, this );
	engine->connectSource( identifier, this );

	const Plasma::DataEngine::Data data = engine->query( identifier );
    } else {
	    m_mySlideShow->setImage(m_currentUrl.path());
	    m_slideShowTimer->stop();
    }

    if (!m_potd)
	updatePicture();
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
    // If the url is a local directory start slideshowmode
    if (droppedUrl.isLocalFile() && QFileInfo(droppedUrl.path()).isDir()) {
    	m_slideShowPaths.append(droppedUrl.path());
        if (!m_slideShow) {
            m_slideShow = true;
        }
    } else {
        m_currentUrl = droppedUrl;
        if (m_slideShow) {
            m_slideShow = false;
        }
    }
    initSlideShow();

    KConfigGroup cg = config();
    cg.writeEntry("url", m_currentUrl);
    cg.writeEntry("slideshow", m_slideShow);
    cg.writeEntry("slideshow paths", m_slideShowPaths);
    emit configNeedsSaving();
}

void Frame::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option,
                           const QRect &rect)
{
    if (m_pixmapCache.isNull() ||
        geometry().toRect().size() != m_pixmapCache.size()) {
        updateGeometry();
        paintCache(option, geometry().toRect().size());
    }

    p->drawPixmap(rect, m_pixmapCache, rect);
}

void Frame::paintCache(const QStyleOptionGraphicsItem *option,
                       const QSize &contentsSize)
{
    Q_UNUSED(option);
    m_pixmapCache = QPixmap(contentsSize);
    m_pixmapCache.fill(Qt::transparent);


    int roundingFactor = 12 * m_roundCorners;
    int swRoundness = roundingFactor + m_frameOutline / 2 * m_frame * m_roundCorners;

    QRect frameRect = m_pixmapCache.rect().adjusted(m_swOutline, m_swOutline,
                                                    -m_swOutline, -m_swOutline); //Pretty useless.

    Qt::TransformationMode transformationMode = m_smoothScaling ? Qt::SmoothTransformation : Qt::FastTransformation;
    //TODO check if correct
    QImage scaledImage = m_picture.scaled(frameRect.size(), Qt::KeepAspectRatio, transformationMode);
    frameRect = QRect(QPoint(frameRect.x() + (frameRect.width() - scaledImage.width()) / 2,
                      frameRect.y() + (frameRect.height() - scaledImage.height()) / 2), scaledImage.size());

    QPainter *p = new QPainter(&m_pixmapCache);
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
            shadowRect.adjust(-1, -1, +1, +1);
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

    delete p;
}

#include "frame.moc"
