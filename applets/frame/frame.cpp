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
#include <kdialog.h>
#include <KLocale>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <KSharedConfig>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <kurl.h>
#include <plasma/theme.h>

#include "math.h"

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
    m_squareCorners = cg.readEntry("squareCorners", true);
    m_roundCorners = cg.readEntry("roundCorners", false);
    m_slideShow = cg.readEntry("slideshow", false);
    m_slideShowUrl = cg.readEntry("slideshow url");
    m_slideshowTime = cg.readEntry("slideshow time", 10); // default to 10 seconds
    m_currentUrl = cg.readEntry("url", "default");

    //Frame & Shadow dimensions
    m_frameOutline = 8;
    m_swOutline = 8;

    //Initialize the slideshow timer
    slideShowTimer = new QTimer(this);
    connect(slideShowTimer, SIGNAL(timeout()), this, SLOT(setSlideShow()));
    slideShowTimer->setInterval(m_slideshowTime*1000);

    if (m_slideShow) {
        setSlideShow();
        slideShowTimer->start();
    }
    else {
        choosePicture(m_currentUrl);
    }
}

void Frame::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        setDrawStandardBackground(false);
    }
}

QImage Frame::loadDefaultImage(QString message)
{
    //Create a QImage with same axpect ratio of default svg and current pixelSize
    QString svgFile = Plasma::Theme::self()->image("widgets/picture-frame-default");
    QSvgRenderer sr(svgFile);
    int pixelSize = contentSize().toSize().width();
    double scale = (double)pixelSize/sr.boundsOnElement("boundingRect").size().width();
    QImage imload(pixelSize, (int) (sr.boundsOnElement("boundingRect").size().height()*scale),QImage::Format_ARGB32);
    imload.fill(Qt::white);
    QPainter p(&imload);
    sr.render(&p,QRect(QPoint(0,0),imload.size()));

    //Set the font and draw text in a textRect
    QRectF textRect = sr.boundsOnElement("textArea");
    textRect = QRectF(textRect.x()*scale,textRect.y()*scale,textRect.width()*scale,textRect.height()*scale);
    QFont textFont;
    textFont.setItalic(true);
    textFont.setBold(true);
    textFont.setPixelSize(imload.height()/15);
    p.setFont(textFont);
    p.drawText(textRect.toRect(),message);
    p.end();
    return imload;
}

void Frame::setSlideShow()
{
    QDir dir(m_slideShowUrl.path());
    QStringList filters;
    filters << "*.jpeg" << "*.jpg" << "*.png";//use mime types?
    dir.setNameFilters(filters);
    QStringList picList = dir.entryList(QDir::Files);
    if ( !picList.isEmpty() ) {
        KUrl currentUrl = QString("%1/%2").arg(m_slideShowUrl.path()).arg(picList.at(m_slideNumber%picList.count()));
        m_slideNumber++;
        choosePicture(currentUrl);
    }
    else return;
}

void Frame::choosePicture(const KUrl& currentUrl)
{
    //FIXME this method hangs Plasma while it downloads & scale the picture

    /* KIO::NetAccess is useless for now.
    if ( !KIO::NetAccess::download( currentUrl, tmpFile, 0L ) ) {
    kDebug() << "Load Error!\n";
    } else*/

    //ugly nested if..else :/
    if (currentUrl.url() == "default") {
        m_picture = loadDefaultImage("Put your photo here\nor drop a folder for starting a slideshow");
    }
	else {
        //QString tmpFile(currentUrl.path());
        QImage tempImage(currentUrl.path());
        if (tempImage.isNull()){
            m_picture = loadDefaultImage("Error loading image");
        }
		else { //Load success! Scale the image if it is too big
            if (tempImage.width() > m_maxDimension || tempImage.height() > m_maxDimension) {
                m_picture = tempImage.scaled(m_maxDimension,m_maxDimension,
                                             Qt::KeepAspectRatio,Qt::SmoothTransformation);
            }
			else {
                m_picture=tempImage;
            }
        }
    }

    m_pixmapCache = QPixmap();
    update();
}

void Frame::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18n("Configure Frame") );
        QWidget *widget = new QWidget();
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
        ui.picRequester->comboBox()->insertItem(0, "http://tools.wikimedia.de/~daniel/potd/potd.php/commons/400x300");
        ui.slideShowRequester->setMode(KFile::Directory);
        ui.slideShowRequester->setGeometry(ui.picRequester->frameGeometry());
        ui.TimeSpinner->setMinimumTime(QTime(0,0,1)); // minimum to 1 seconds
    }
    ui.frameCheckBox->setChecked(m_frame);
    ui.shadowCheckBox->setChecked(m_shadow);
    ui.squareButton->setChecked(m_squareCorners);
    ui.roundButton->setChecked(m_roundCorners);
    ui.pictureComboBox->setCurrentIndex(m_slideShow);
    //ui.stackedWidget->setCurrentIndex(m_slideShow*2);
    ui.picRequester->setUrl(m_currentUrl);
    ui.slideShowRequester->setUrl(m_slideShowUrl);
    ui.TimeSpinner->setTime(QTime(m_slideshowTime / 3600, (m_slideshowTime / 60) % 60, m_slideshowTime % 60));
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
    m_squareCorners = ui.squareButton->isChecked();
    cg.writeEntry("squareCorners", m_squareCorners);
    m_roundCorners = ui.roundButton->isChecked();
    cg.writeEntry("roundCorners", m_roundCorners);
    m_currentUrl = ui.picRequester->url();
    cg.writeEntry("url", m_currentUrl);
    m_slideShow = ui.pictureComboBox->currentIndex();
    cg.writeEntry("slideshow", m_slideShow);
    m_slideShowUrl = ui.slideShowRequester->url();
    cg.writeEntry("slideshow url", m_slideShowUrl);

    QTime timerTime = ui.TimeSpinner->time();
    m_slideshowTime = timerTime.second() + timerTime.minute() * 60 + timerTime.hour() * 3600;
    slideShowTimer->setInterval(m_slideshowTime*1000);
    cg.writeEntry("slideshow time", m_slideshowTime);

    if (m_slideShow) {
        setSlideShow();
        slideShowTimer->start();
    }
    else {
        slideShowTimer->stop();
        choosePicture(m_currentUrl);
    }

    emit configNeedsSaving();
}

void Frame::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
	kDebug() << event->mimeData()->formats();
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
	else {
         event->ignore();
	}
 }

void Frame::dropEvent(QGraphicsSceneDragDropEvent *event)
{
	KUrl droppedUrl = (KUrl::List::fromMimeData(event->mimeData())).at(0);
    //If the url is a local directory start slideshowmode
    if (droppedUrl.isLocalFile() && QFileInfo(droppedUrl.path()).isDir()) {
        m_slideShowUrl = droppedUrl;
        m_slideShow = true;
        setSlideShow();
        slideShowTimer->start();
    }
	else {
        m_currentUrl = droppedUrl;
        choosePicture(m_currentUrl);
		slideShowTimer->stop();
		m_slideShow = false;
    }
    KConfigGroup cg = config();
    cg.writeEntry("url", m_currentUrl);
    cg.writeEntry("slideshow url", m_slideShowUrl);
    emit configNeedsSaving();
}

Frame::~Frame()
{
}

void Frame::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option,
                           const QRect &rect)
{
    if (m_pixmapCache.isNull() || contentRect().toRect().size()!=m_pixmapCache.size()) {
        paintCache(option, contentRect().toRect().size());
    }

//    p->save();
//    p->setCompositionMode(QPainter::CompositionMode_Source);
    p->drawPixmap(rect, m_pixmapCache, rect);
//    p->restore();
}

void Frame::paintCache(const QStyleOptionGraphicsItem *option,
                       const QSize &contentsSize)
{
    Q_UNUSED(option);

    m_pixmapCache = QPixmap(contentsSize);
    m_pixmapCache.fill(Qt::transparent);

    QPainter *p = new QPainter(&m_pixmapCache);

    int roundingFactor = 12 * m_roundCorners;
    int swRoundness = roundingFactor+m_frameOutline/2*m_frame*m_roundCorners;

    QRect frameRect = m_pixmapCache.rect().adjusted(m_swOutline, m_swOutline,
                                                    -m_swOutline, -m_swOutline); //Pretty useless.

    QImage scaledImage = m_picture.scaled(frameRect.size(), Qt::KeepAspectRatio, Qt::FastTransformation);
    frameRect = QRect(QPoint(frameRect.x() + (frameRect.width() - scaledImage.width()) / 2,
                      frameRect.y() + (frameRect.height() - scaledImage.height()) / 2), scaledImage.size());

    QRect shadowRect;
    if (m_frame) {
        shadowRect = frameRect.adjusted(-m_frameOutline+1, -m_frameOutline+1,
                                        m_frameOutline-1, m_frameOutline-1);
    } else {
        shadowRect = frameRect;
    }

    //choose where to draw.

    ///The frame path. It will be used to draw the frame and clip the image.
    QPainterPath framePath = Plasma::roundedRectangle(frameRect, roundingFactor);

    p->setRenderHint(QPainter::SmoothPixmapTransform, true);
    p->setRenderHint(QPainter::Antialiasing, true);

    //Shadow
    //TODO faster. I'd like to use it on liveTransform.
    if (m_shadow) {
        p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap,Qt::RoundJoin));
        p->setBrush(Qt::NoBrush);
        for (int i = 0; i <= m_swOutline; i+=1) {
            p->setOpacity(0.7*exp(-(i/(double)(m_swOutline/3))));
            QPainterPath tr=Plasma::roundedRectangle(shadowRect, swRoundness+i);
            p->drawPath(tr);
            shadowRect.adjust(-1, -1, +1, +1);
        }
    }

    p->setBrush(Qt::NoBrush);

    //Frame
    if (m_frame) {
        p->setOpacity(0.5);
        p->setPen(QPen(m_frameColor, m_frameOutline*2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        p->drawPath(framePath);
    }

    p->setOpacity(1);

    //Picture
    p->save();
    if (m_roundCorners) {
        p->setClipPath(framePath);
    }

    // scale and center
//    kDebug() << "painting to" << frameRect << imageTarget << "for image of size" << scaledImage.size();
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
