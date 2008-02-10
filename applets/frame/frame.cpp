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
#include <QRadioButton>
#include <QGroupBox>
#include <QGraphicsSceneDragDropEvent>
#include <QPen>
#include <QCheckBox>
#include <QSpinBox>
#include <QTimer>
#include <QFileInfo>
#include <QSvgRenderer>

#include <KDebug>
#include <KLocale>
#include <KColorButton>
#include <KComboBox>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KUrlRequester>
#include <KUrl>
#include <KDirSelectDialog>

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
    setCachePaintMode(NoCacheMode);
    setSize(350, 350);
    m_mySlideShow = new SlideShow();
}

Frame::~Frame()
{
    delete m_mySlideShow;
    delete m_configDialog;
}

void Frame::init()
{
    m_slideNumber = 0;
    // Get config values
    KConfigGroup cg = config();
    m_frameColor = cg.readEntry("frameColor", QColor(70, 90, 130)); //theme?
    m_frame = cg.readEntry("frame", false);
    m_shadow = cg.readEntry("shadow", true);
    m_roundCorners = cg.readEntry("roundCorners", false);
    m_slideShow = cg.readEntry("slideshow", false);
    m_slideShowPaths = cg.readEntry("slideshow paths", QStringList());
    m_slideshowTime = cg.readEntry("slideshow time", 10); // default to 10 seconds
    m_currentUrl = cg.readEntry("url", "Default");

    // Frame & Shadow dimensions
    m_frameOutline = 8;
    m_swOutline = 8;

    // Initialize the slideshow timer
    m_slideShowTimer = new QTimer(this);
    connect(m_slideShowTimer, SIGNAL(timeout()), this, SLOT(updatePicture()));
    m_slideShowTimer->setInterval(m_slideshowTime * 1000);

    initSlideShow();
}

void Frame::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        setDrawStandardBackground(false);
    }
}

void Frame::updatePicture()
{
    m_picture = m_mySlideShow->getImage();
    m_pixmapCache = QPixmap();
    update();
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

void Frame::showConfigurationInterface()
{
   if ( !m_configDialog ) {
        m_configDialog = new ConfigDialog( 0 );
        connect( m_configDialog, SIGNAL( applyClicked() ), this, SLOT( configAccepted() ) );
        connect( m_configDialog, SIGNAL( okClicked() ), this, SLOT( configAccepted() ) );
    }
    connect(m_configDialog->ui.removeDirButton, SIGNAL(clicked()), this, SLOT(removeDir()));
    connect(m_configDialog->ui.addDirButton, SIGNAL(clicked()), this, SLOT(addDir()));
   
    m_configDialog->setRoundCorners( m_roundCorners );
    m_configDialog->setShadow(m_shadow);
    m_configDialog->setShowFrame(m_frame);
    m_configDialog->setFrameColor(m_frameColor);

    m_configDialog->ui.pictureComboBox->setCurrentIndex(m_slideShow);//to change when adding PoTD
    m_configDialog->setCurrentUrl(m_currentUrl);
    m_configDialog->ui.slideShowDirList->clear();
    m_configDialog->ui.slideShowDirList->addItems(m_slideShowPaths);
    m_configDialog->ui.removeDirButton->setEnabled(!m_slideShowPaths.isEmpty());
    m_configDialog->ui.slideShowDelay->setTime(QTime(m_slideshowTime / 3600, (m_slideshowTime / 60) % 60, m_slideshowTime % 60));

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
    m_shadow = m_configDialog->shadow();
    cg.writeEntry("shadow", m_shadow);
    m_frame = m_configDialog->showFrame();
    cg.writeEntry("frame", m_frame);
    m_frameColor = m_configDialog->frameColor();
    cg.writeEntry("frameColor", m_frameColor);

    m_currentUrl = m_configDialog->currentUrl();
    cg.writeEntry("url", m_currentUrl);
    m_slideShow = m_configDialog->ui.pictureComboBox->currentIndex();
    cg.writeEntry("slideshow", m_slideShow);
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

    initSlideShow();
    
    
    emit configNeedsSaving();
}

void Frame::initSlideShow()
{
	if (m_slideShow) {
		m_mySlideShow->setDirs(m_slideShowPaths);
		m_slideShowTimer->start();
	} else {
		m_mySlideShow->setImage(m_currentUrl.path());
		m_slideShowTimer->stop();
	}
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
    cg.writeEntry("slideshow paths", m_slideShowPaths);
    emit configNeedsSaving();
}

void Frame::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option,
                           const QRect &rect)
{
    if (m_pixmapCache.isNull() ||
        contentRect().toRect().size() != m_pixmapCache.size()) {
        paintCache(option, contentRect().toRect().size());
    }

    p->drawPixmap(rect, m_pixmapCache, rect);
}

void Frame::paintCache(const QStyleOptionGraphicsItem *option,
                       const QSize &contentsSize)
{
    Q_UNUSED(option);

    m_pixmapCache = QPixmap(contentsSize);
    m_pixmapCache.fill(Qt::transparent);

    QPainter *p = new QPainter(&m_pixmapCache);

    int roundingFactor = 12 * m_roundCorners;
    int swRoundness = roundingFactor + m_frameOutline / 2 * m_frame * m_roundCorners;

    QRect frameRect = m_pixmapCache.rect().adjusted(m_swOutline, m_swOutline,
                                                    -m_swOutline, -m_swOutline); //Pretty useless.
    
    //TODO check if correct
    QImage scaledImage = m_picture.scaled(frameRect.size(), Qt::KeepAspectRatio, Qt::FastTransformation);
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
    QPainterPath framePath = Plasma::roundedRectangle(frameRect, roundingFactor);

    p->setRenderHint(QPainter::SmoothPixmapTransform, true);
    p->setRenderHint(QPainter::Antialiasing, true);

    // Shadow
    // TODO faster. I'd like to use it on liveTransform.
    if (m_shadow) {
        p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
        p->setBrush(Qt::NoBrush);
        for (int i = 0; i <= m_swOutline; i += 1) {
            p->setOpacity(0.7 * exp(-(i / (double)(m_swOutline / 3))));
            QPainterPath tr = Plasma::roundedRectangle(shadowRect, swRoundness + i);
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
