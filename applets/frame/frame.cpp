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

#include <QApplication>
#include <QBitmap>
#include <QGraphicsScene>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QGridLayout>
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
#include <KIcon>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <KSharedConfig>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <kio/netaccess.h>
#include <kurl.h>
#include <QPicture>
#include <plasma/dataenginemanager.h>
#include <plasma/widgets/vboxlayout.h>
#include <plasma/phase.h>
#include <plasma/theme.h>
#include "math.h"


Frame::Frame(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), 
      m_dialog(0),
      m_liveTransform(false)
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);
    setDrawStandardBackground(false);
    setCachePaintMode(NoCacheMode);

    m_dialog = 0;
    m_slideNumber = 0;
    m_skipPaint = 0;
    m_handle1AnimId = 0;
    // Get config values
    KConfigGroup cg = config();
    m_frameColor = cg.readEntry("frameColor", QColor(70, 90, 130));
    m_frame = cg.readEntry("frame", false);
    m_shadow = cg.readEntry("shadow", true);
    m_squareCorners = cg.readEntry("squareCorners", true);
    m_roundCorners = cg.readEntry("roundCorners", false);
    m_pixelSize = cg.readEntry("size", 350);
    m_rotation = cg.readEntry("rotation",0);
    m_slideShow = cg.readEntry("slideshow", false);
    m_slideShowUrl = cg.readEntry("slideshow url");
    m_slideshowTime = cg.readEntry("slideshow time", 10); // default to 10 seconds
    m_currentUrl = cg.readEntry("url", "default");
    /*/ m_layout is unused for now.
    m_layout = new Plasma::VBoxLayout(0);
    m_layout->setGeometry(QRectF(0, 0, 400, 800));
    m_layout->setMargin(12);*/
 
    //Frame & Shadow dimensions
    m_frameOutline = 8;
    m_swOutline = 8;

    //Initialize handles
    m_handle1 = KIcon("transform-rotate").pixmap(32,32);

    //Initialize the slideshow timer
    slideShowTimer = new QTimer(this);
    connect(slideShowTimer, SIGNAL(timeout()), this, SLOT(setSlideShow()));
    slideShowTimer->setInterval(m_slideshowTime*1000);

    //Initialize the picture
    m_cmpPicture = NULL;
    if (m_slideShow) {
        setSlideShow();
        slideShowTimer->start();
    } 
	else {   
        choosePicture(m_currentUrl);
    }
}

QImage Frame::loadDefaultImage(QString message)
{

    //Create a QImage with same axpect ratio of default svg and current pixelSize 
    QString svgFile = Plasma::Theme::self()->image("widgets/picture-frame-default");
    QSvgRenderer sr(svgFile);
    double scale = (double)m_pixelSize/sr.boundsOnElement("boundingRect").size().width();
    QImage imload(m_pixelSize, (int) (sr.boundsOnElement("boundingRect").size().height()*scale),QImage::Format_ARGB32);
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

void Frame::resizeEvent(QResizeEvent *)
{
    prepareGeometryChange();
}

void Frame::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    Q_UNUSED(data);
    QGraphicsItem::update();
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
        myPicture = loadDefaultImage("Put your photo here\nor drop a folder for starting a slideshow");
    } 
	else {
        //QString tmpFile(currentUrl.path());
        QImage tempImage(currentUrl.path());
        if (tempImage.isNull()){
            myPicture = loadDefaultImage("Error loading image");
        } 
		else { //Load success! Scale the image if it is too big 
            if (tempImage.width() > m_maxDimension || tempImage.height() > m_maxDimension) { 
                myPicture = tempImage.scaled(m_maxDimension,m_maxDimension, 
                                             Qt::KeepAspectRatio,Qt::SmoothTransformation);
            } 
			else {
                myPicture=tempImage;
            }
        }
    }

    prepareGeometryChange();
    updateSizes();
    composePicture();
    update();
}

void Frame::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18n("Configure Frame") );
        ui.setupUi(m_dialog->mainWidget());
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
    cg.writeEntry("size", m_pixelSize);
    cg.writeEntry("rotation", m_rotation);
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
    
    cg.config()->sync();
}

QRectF Frame::boundingRect() const
{
    // return m_layout->geometry();
    if (m_liveTransform) {
        //If the user is transforming the picture, this set the bouding rect to the widest area
        //So we don't need to call prepareGeometryChange each mouse movement.
        QSize tmp = myPicture.size();
        tmp.scale(m_maxDimension,m_maxDimension,Qt::KeepAspectRatio);
        tmp+=QSize(2*(m_swOutline+m_frameOutline),2*(m_swOutline+m_frameOutline));
        qreal diagonal = sqrt(tmp.width()*tmp.width()+tmp.height()*tmp.height());
        return QRectF(-diagonal/2,-diagonal/2,diagonal,diagonal);
    }
    return m_boundingRect;
}

QPainterPath Frame::shape() const
{
    QPainterPath path;
    path.addPolygon(mapToPicture(QPolygon(m_pixmapOutlineRect)));
    return path;
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
    cg.config()->sync();
}

Frame::~Frame()
{
    //delete m_layout;
    delete m_cmpPicture;
}

//if *painter is NULL, it draws to m_cmpPicture.
void Frame::composePicture(QPainter* painter)
{

    int roundingFactor = 12 * m_roundCorners;
    int swRoundness = roundingFactor+m_frameOutline/2*m_frame*m_roundCorners;

    QRect frameRect = m_pixmapRect; //Pretty useless.
    qreal ratio = (double)m_pixmapRect.width()/(double)myPicture.width();

    QRect shadowRect;
    if (m_frame) {
        shadowRect = frameRect.adjusted(-m_frameOutline+1,-m_frameOutline+1,m_frameOutline-1,m_frameOutline-1);
	}
    else  {
        shadowRect = frameRect;
	}
    //choose where to draw.
    QPainter *p;
    if (painter == NULL) {
        delete m_cmpPicture;
        m_cmpPicture = new QPixmap(m_boundingRect.size());
        m_cmpPicture->fill(Qt::transparent);
        p = new QPainter(m_cmpPicture);
    } 
	else  {
        p = painter;
	}
    
    ///The frame path. It will be used to draw the frame and clip the image.
    QPainterPath framePath;
    framePath.addRoundRect(frameRect, roundingFactor);

    p->save();
    p->setRenderHint(QPainter::SmoothPixmapTransform, !m_liveTransform); 
    p->setRenderHint(QPainter::Antialiasing,!m_liveTransform); 

    //If we draw on the pixmap, we can't use negative coordinates, so ...
    if (painter==NULL) {
		p->translate(m_boundingRect.width()/2,m_boundingRect.height()/2);
	}
    
    //Rotation
    p->rotate(m_rotation);

    //Shadow 
    //TODO faster. I'd like to use it on liveTransform.
    if (m_shadow && !m_liveTransform) {
        p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap,Qt::RoundJoin));
        p->setBrush(Qt::NoBrush);
        for (int i = 0; i <= m_swOutline; i+=1) {
            p->setOpacity(0.7*exp(-(i/(double)(m_swOutline/3))));
            QPainterPath tr; //I use this because p.drawRoundRect is different(and ugly)
            tr.addRoundRect(shadowRect,swRoundness+i);
            p->drawPath(tr);
            shadowRect.adjust(-1,-1,+1,+1);
        }
    } 
    
    p->setBrush(Qt::NoBrush);

    //Frame
    if (m_frame) {
        p->setOpacity(0.5);
        p->setPen(QPen(m_frameColor, m_frameOutline*2, Qt::SolidLine, Qt::FlatCap,Qt::MiterJoin));
        p->drawPath(framePath);
    }

    p->setOpacity(1);

    //Picture
    p->save();
    if (m_roundCorners) {
		p->setClipPath(framePath);
    }
    p->translate(frameRect.x(),frameRect.y());
    p->scale(ratio,ratio);
    p->drawImage(0,0,myPicture); 
    p->restore();

    // black frame
    if (m_frame) {
		p->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
		p->drawPath(framePath);
    } 
    else if (m_roundCorners) {
        p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->drawPath(framePath);
    }

    p->restore();

    if (painter == NULL) {
        delete p;
    }
}

void Frame::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);
 
    if (m_liveTransform) {
        //if true draw directly  
        composePicture(p);
    }
    else {
        //draw the cached pixmap
        p->drawPixmap (m_boundingRect.x(),m_boundingRect.y(), *m_cmpPicture);
    }

    if (m_handle1AnimId) {
        p->setRenderHint(QPainter::SmoothPixmapTransform, !m_liveTransform); 
        p->rotate(m_rotation);
        QPoint pos = QPoint(m_pixmapRect.right()-32,m_pixmapRect.bottom()-32);
        p->drawPixmap(pos.x(), pos.y(), Plasma::Phase::self()->animationResult(m_handle1AnimId));
    }
}
	
double Frame::angleForPos(QPointF in)
{
    return atan2(0-in.y(),in.x());
}

double Frame::distanceForPos(QPointF in)
{
    return sqrt(in.x()*in.x()+in.y()*in.y());
}

void Frame::updateSizes()
{
    QSize pixSize = myPicture.size();
    pixSize.scale(m_pixelSize,m_pixelSize,Qt::KeepAspectRatio);
    m_pixmapRect = QRect(QPoint(0,0),pixSize);
    m_pixmapRect.translate(-m_pixmapRect.width()/2,-m_pixmapRect.height()/2);
    m_pixmapOutlineRect = m_pixmapRect;

    if (m_frame) { 
        m_pixmapOutlineRect.adjust(-m_frameOutline,-m_frameOutline,m_frameOutline,m_frameOutline);
    }

    if (m_shadow) {
        m_pixmapOutlineRect.adjust(-m_swOutline,-m_swOutline,m_swOutline,m_swOutline);
    }

    m_boundingRect = mapToPicture(QPolygon(m_pixmapOutlineRect)).boundingRect();
}

///transform a polygon to the "logical coordinates" of the picture, accounting its rotation. 
//(rotation isn't done by QGraphicsItem, so we must use this)
QPolygon Frame::mapToPicture(const QPolygon in) const
{
    QMatrix matrix;
    matrix.rotate(m_rotation);
    return matrix.map(in);
}

void Frame::mousePressEvent ( QGraphicsSceneMouseEvent * event ) 
{


    //Start the live transformation mode if the user clicks on bottom right corner
    QPolygon activeArea = QPolygon(QRect(m_pixmapRect.right() - 32, m_pixmapRect.bottom() - 32, 32,32));

    activeArea = mapToPicture(activeArea);

    if ((event->button() ==  Qt::LeftButton) && (activeArea.containsPoint(event->pos().toPoint(),Qt::OddEvenFill)))
    {
        m_liveTransform = true;
        m_ltReferenceRotation = m_rotation;
        m_ltReferencePixelSize = m_pixelSize;
    }

}

void Frame::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    if (!m_liveTransform) {
        return QGraphicsItem::mouseReleaseEvent(event);
    }
    //Exit from live transformation mode and store the changes
    m_liveTransform = false;
    //If the rotation is small is set to zero
    m_rotation = m_rotation%360;
    if (m_rotation < 5 && m_rotation > -5) {
        m_rotation = 0;
    }
    KConfigGroup cg = config();
    cg.writeEntry("rotation", m_rotation);
    cg.writeEntry("size", m_pixelSize);
    cg.config()->sync();
    updateSizes();
    composePicture(); //Cache the composed & transformed pixmap.
    update();
}

void Frame::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_handle1AnimId = Plasma::Phase::self()->animateElement(this, Plasma::Phase::ElementAppear);
    Plasma::Phase::self()->setAnimationPixmap(m_handle1AnimId, m_handle1);
}

void Frame::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    
    if (m_handle1AnimId) {
        m_handle1AnimId = Plasma::Phase::self()->animateElement(this, Plasma::Phase::ElementDisappear);
        Plasma::Phase::self()->setAnimationPixmap(m_handle1AnimId, m_handle1);
    }
}
void Frame::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

    if (!m_liveTransform) {
        return QGraphicsItem::mouseMoveEvent(event);
    }
    //The hand is faster than the eye. So we skip some mouse movements ^^''
    if (m_skipPaint != 4){
        m_skipPaint++;
        return;
    } 
    else {
      m_skipPaint = 0;
    }

    // Map the original button-down position back to local coordinates.
    // Since transformations aren't done by setTransform(), it is redundant now, 
    // But it works anyway and could be useful in the future.
    QPointF buttonDownPos = mapFromScene(event->buttonDownScenePos(Qt::LeftButton));

    const double pi = 3.14159265;
    qreal oldAngle = (180 * angleForPos(buttonDownPos)) / pi;
    qreal newAngle = (180 * angleForPos(event->pos())) / pi;
    qreal scaleFactor = distanceForPos(event->pos()) /  distanceForPos(buttonDownPos);

    // Determine the item's new rotation
    m_rotation = (int)(m_ltReferenceRotation - newAngle + oldAngle);
    m_pixelSize = (int) (m_ltReferencePixelSize * scaleFactor);
    // Don't allow to go over maxDimension
    if (m_pixelSize > m_maxDimension) {
      m_pixelSize = m_maxDimension;
    }
    // Update pixmap geometry & redraw. 
    updateSizes();
    update();
}

#include "frame.moc"
