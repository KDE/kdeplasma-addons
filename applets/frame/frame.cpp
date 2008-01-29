/***************************************************************************
 *   Copyright  2007 by Anne-Marie Mahfouf <annma@kde.org>              *
 *   Copyright  2007 by Antonio Vinci <mercurio@personellarete.it>      *
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
#include <KDialog>
#include <KLocale>
#include <KColorButton>
#include <KComboBox>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KUrlRequester>
#include <KUrl>
#include <KDirSelectDialog>

#include <plasma/theme.h>

#include <math.h>
#include "picture.h"

Frame::Frame(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0)
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);
    setCachePaintMode(NoCacheMode);
    setSize(350, 350);
}

void Frame::init()
{
    m_dialog = 0;
    m_slideNumber = 0;
    // Get config values
    KConfigGroup cg = config();
    m_frameColor = cg.readEntry("frameColor", QColor(70, 90, 130));
    m_frame = cg.readEntry("frame", false);
    m_shadow = cg.readEntry("shadow", true);
    m_roundCorners = cg.readEntry("roundCorners", false);
    m_slideShow = cg.readEntry("slideshow", false);
    m_slideShowPaths = cg.readEntry("slideshow paths", QStringList());
    m_slideshowTime = cg.readEntry("slideshow time", 10); // default to 10 seconds
    m_currentUrl = cg.readEntry("url", QString());

    // Frame & Shadow dimensions
    m_frameOutline = 8;
    m_swOutline = 8;

    // Initialize the slideshow timer
    m_slideShowTimer = new QTimer(this);
    connect(m_slideShowTimer, SIGNAL(timeout()), this, SLOT(setSlideShow()));
    m_slideShowTimer->setInterval(m_slideshowTime * 1000);

    if (m_slideShow) {
        setSlideShow();
        m_slideShowTimer->start();
    } else {
        choosePicture(m_currentUrl);
    }
}

void Frame::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        setDrawStandardBackground(false);
    }
}

void Frame::setSlideShow()
{
    Picture myPicture;
    QStringList picList=myPicture.findSlideShowPics(m_slideShowPaths);
    kDebug() <<"picList = " << picList <<endl;
    if (!picList.isEmpty()) {
        KUrl currentUrl(picList.at(m_slideNumber % picList.count()));
        m_slideNumber++;
        choosePicture(currentUrl);
    } else {
        choosePicture(m_currentUrl);
    }
}

void Frame::choosePicture(const KUrl& currentUrl)
{
    Picture myPicture;
    m_picture = myPicture.setPicture(contentSize().toSize().width(), currentUrl);

    m_pixmapCache = QPixmap();
    update();
}

void Frame::addDir()
{
    KDirSelectDialog dialog(KUrl(), true);
    if (dialog.exec()) {
        QString path = dialog.url().path();
        if (!m_slideShowPaths.contains(path)) {
            ui.slideShowDirList->addItem(path);
        }
        ui.removeDirButton->setEnabled(true);
    }
}

void Frame::removeDir()
{
    int row = ui.slideShowDirList->currentRow();
    if (row != -1) {
        ui.slideShowDirList->takeItem(row);
        if (ui.slideShowDirList->count() == 0) {
            ui.removeDirButton->setEnabled(false);
        }
    }
}

void Frame::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption(i18nc("@title:window", "Configure Frame"));
        QWidget *widget = new QWidget();
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect(m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));
        ui.addDirButton->setIcon(KIcon("list-add"));
        connect(ui.addDirButton, SIGNAL(clicked()), this, SLOT(addDir()));
        ui.removeDirButton->setIcon(KIcon("list-remove"));
        connect(ui.removeDirButton, SIGNAL(clicked()), this, SLOT(removeDir()));
        ui.slideShowDelay->setMinimumTime(QTime(0, 0, 1)); // minimum to 1 seconds
    }
    ui.frameCheckBox->setChecked(m_frame);
    ui.shadowCheckBox->setChecked(m_shadow);
    ui.roundCheckBox->setChecked(m_roundCorners);
    ui.pictureComboBox->setCurrentIndex(m_slideShow);
    ui.picRequester->setUrl(m_currentUrl);
    ui.slideShowDirList->clear();
    ui.slideShowDirList->addItems(m_slideShowPaths);
    ui.removeDirButton->setEnabled(!m_slideShowPaths.isEmpty());
    ui.slideShowDelay->setTime(QTime(m_slideshowTime / 3600, (m_slideshowTime / 60) % 60, m_slideshowTime % 60));
    m_dialog->show();
}

void Frame::configAccepted()
{
    prepareGeometryChange();
    KConfigGroup cg = config();
    m_frameColor = ui.changeFrameColor->color();
    cg.writeEntry("frameColor", m_frameColor);
    m_frame = ui.frameCheckBox->isChecked();
    cg.writeEntry("frame", m_frame);
    m_shadow = ui.shadowCheckBox->isChecked();
    cg.writeEntry("shadow", m_shadow);
    m_roundCorners = ui.roundCheckBox->isChecked();
    cg.writeEntry("roundCorners", m_roundCorners);
    m_currentUrl = ui.picRequester->url();
    cg.writeEntry("url", m_currentUrl);
    m_slideShow = ui.pictureComboBox->currentIndex();
    cg.writeEntry("slideshow", m_slideShow);
    m_slideShowPaths.clear();
    QStringList dirs;
    for (int i = 0; i < ui.slideShowDirList->count(); i++) {
        m_slideShowPaths << ui.slideShowDirList->item(i)->text();
    }
    cg.writeEntry("slideshow paths", m_slideShowPaths);

    QTime timerTime = ui.slideShowDelay->time();
    m_slideshowTime = timerTime.second() + timerTime.minute() * 60 + timerTime.hour() * 3600;
    m_slideShowTimer->setInterval(m_slideshowTime * 1000);
    cg.writeEntry("slideshow time", m_slideshowTime);

    if (m_slideShow) {
        setSlideShow();
        m_slideShowTimer->start();
    } else {
        m_slideShowTimer->stop();
        choosePicture(m_currentUrl);
    }

    emit configNeedsSaving();
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
        if (!m_slideShowPaths.contains(droppedUrl.path())) {
            m_slideShowPaths.append(droppedUrl.path());
            if (!m_slideShow) {
                m_slideShow = true;
                setSlideShow();
                m_slideShowTimer->start();
            }
        }
    } else {
        m_currentUrl = droppedUrl;
        choosePicture(m_currentUrl);
        if (m_slideShow) {
            m_slideShowTimer->stop();
            m_slideShow = false;
        }
    }

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
