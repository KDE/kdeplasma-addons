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
#include <QGraphicsLinearLayout>
#include <QPen>
#include <QCheckBox>
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
#include <Plasma/ToolButton>
#include <Plasma/Frame>

#include <math.h>

#include "configdialog.h"
#include "picture.h"
#include "slideshow.h"

Frame::Frame(QObject *parent, const QVariantList &args)
        : Plasma::Applet(parent, args),
        m_configDialog(0),
        m_slideFrame(0)
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
        m_currentUrl = KUrl();
    }
    setAssociatedApplicationUrls(m_currentUrl);
}

Frame::~Frame()
{
}

void Frame::init()
{
    bool frameReceivedUrlArgs = false;
    if (!m_currentUrl.isEmpty()) {
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
    setAssociatedApplicationUrls(m_currentUrl);
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

    QAction *openAction = action("run associated application");
    openAction->setIcon(SmallIcon("image-x-generic"));
    openAction->setText(i18n("&Open Picture..."));
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

    if (!url.path().isEmpty()) {
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
        m_dirty = true;
    }

    if (constraints & Plasma::SizeConstraint) {
        //If on panel, keep geometry to 4:3 ratio
        if(formFactor() == Plasma::Vertical) {
            setMinimumSize(QSizeF(0, contentsRect().width()/1.33));
            setMaximumSize(QSizeF(-1, contentsRect().width()/1.33));
        } else if(formFactor() == Plasma::Horizontal) {
            setMinimumSize(QSizeF(contentsRect().height()*1.33,0));
            setMaximumSize(QSizeF(contentsRect().height()*1.33,-1));
        } else {
            int min = 22;
            if (m_shadow) {
                min += m_swOutline;
            }
            if (m_frame) {
                min += m_frameOutline;
            }
            setMinimumSize(QSizeF(min, min));
            setMaximumSize(QSizeF());
        }

        if (m_slideShow) {
            checkSlideFrame();

            int x = contentsRect().center().x() - (m_slideFrame->size().width() / 2);
            int y = contentsRect().bottom() - m_slideFrame->size().height() - 5;
            m_slideFrame->setPos(x, y);
        }

        m_dirty = true;
    }
}

QSizeF Frame::contentSizeHint() const
{
    if (!m_pictureSize.isEmpty() && (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter)){
        const qreal maxSize = contentsRect().width() > contentsRect().height() ? contentsRect().width() : contentsRect().height();
        QSize size = m_pictureSize;
        size.scale(maxSize, maxSize, Qt::KeepAspectRatio);
        return size;
    } else {
        return contentsRect().size();
    }
}

QSizeF Frame::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which != Qt::PreferredSize) {
        return Applet::sizeHint(which, constraint);
    } else {
        return m_pictureSize;
    }
}

void Frame::updatePicture()
{
    m_pictureSize = m_mySlideShow->image().size();
    QSizeF sizeHint = contentSizeHint();
    int frameLines = qMin(m_frameOutline, (int)(sizeHint.height()/10));
    const QSize contentsSize = sizeHint.toSize();

    if (contentsRect().size() != sizeHint) {
        resize(sizeHint + QSizeF(boundingRect().size() - contentsRect().size()));
        emit appletTransformedItself();
    }
    if (m_currentUrl.url().isEmpty() && m_mySlideShow->currentUrl().isEmpty()) {
        setAssociatedApplicationUrls(KUrl::List());
    } else {
        setAssociatedApplicationUrls(m_mySlideShow->currentUrl());
    }

    emit sizeHintChanged(Qt::PreferredSize);

    kDebug() << "Rendering picture";

    // create a QPixmap which can be drawn in paintInterface()
    QPixmap picture = m_mySlideShow->image();

    if (picture.isNull()) {
        return;
    }

    m_pixmap = QPixmap(contentsSize);
    m_pixmap.fill(Qt::transparent);
    QPainter *p = new QPainter();
    p->begin(&m_pixmap);

    int roundingFactor = qMin(qreal(sizeHint.height() / 10), qreal(12.0)) * m_roundCorners;
    int swRoundness = roundingFactor + frameLines / 2 * m_frame * m_roundCorners;

    QRectF frameRect(QPoint(0, 0), contentsSize);
    frameRect.adjust(m_swOutline, m_swOutline, -m_swOutline, -m_swOutline); //Pretty useless.

    QSizeF scaledSize = frameRect.size();
    scaledSize.scale(frameRect.size(), Qt::KeepAspectRatio);
    frameRect = QRectF(QPoint(frameRect.x() + (frameRect.width() - scaledSize.width()) / 2,
                      frameRect.y() + (frameRect.height() - scaledSize.height()) / 2), scaledSize);

    QRectF shadowRect;
    if (m_frame) {
        shadowRect = frameRect.adjusted(-frameLines, -frameLines,
                                        frameLines, frameLines);
    } else {
        shadowRect = frameRect;
    }

    // The frame path. It will be used to draw the frame and clip the image.
    QPainterPath framePath = Plasma::PaintUtils::roundedRectangle(frameRect, roundingFactor);

    p->setRenderHint(QPainter::SmoothPixmapTransform, true);
    p->setRenderHint(QPainter::Antialiasing, true);

    // Shadow
    if (m_shadow) {
        // The shadow is a couple of lines with decreasing opacity painted around the path
        p->setBrush(Qt::NoBrush);
        QPen pen = QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
        int shadowLines = qMin((int)(sizeHint.height() / 6), m_swOutline);

        // The shadow is drawn from inside to the outside
        shadowRect.adjust(+shadowLines, +shadowLines, -shadowLines, -shadowLines);

        // Make the path ot paint the frame in a bit smaller so it's inside the shadow
        frameRect.adjust(+shadowLines, +shadowLines, -shadowLines, -shadowLines);
        framePath = Plasma::PaintUtils::roundedRectangle(frameRect, roundingFactor);

        // Paint the shadow's lines around the picture
        for (int i = m_swOutline - shadowLines; i <= m_swOutline; i += 1) {
            // It's important to change the opacity using the QColor, as
            // QPainter->setOpacity() kills performance
            qreal opacity = 0.7 * exp(-(i / (double)(m_swOutline / 3)));
            pen.setColor(QColor(0, 0, 0, opacity * 254));
            p->setPen(pen);
            QPainterPath tr = Plasma::PaintUtils::roundedRectangle(shadowRect, swRoundness + i);
            p->drawPath(tr);
            shadowRect.adjust(-1, -1, + 1, + 1);
        }
    }

    p->setBrush(Qt::NoBrush);

    // Frame
    if (m_frame) {
        m_frameColor.setAlphaF(0.5);
        // The frame is painted twice as thick, the inner half lies behind the picture
        // This is important to not make the corners look "cut out" when rounded
        p->setPen(QPen(m_frameColor, frameLines * 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        p->drawPath(framePath);
    }

    // Picture

    // We save the painter here so the clipping only happens on the pixmap,
    // not on pixmap, frame, shadow, etc
    p->save();
    if (m_roundCorners) {
        p->setClipPath(framePath);
    }

    // Respect the smoothScaling setting
    p->setRenderHint(QPainter::SmoothPixmapTransform, m_smoothScaling);
    // draw our pixmap into the computed rectangle
    p->drawPixmap(frameRect.toRect(), m_mySlideShow->image());
    p->restore();

    p->setRenderHint(QPainter::SmoothPixmapTransform, true);
    // black frame
    if (m_frame) {
        p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->drawPath(framePath);
    } else if (m_roundCorners) {
        p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->drawPath(framePath);
    }

    // Paint status text on top of it all
    if (!m_mySlideShow->message().isEmpty()) {
        int dist = frameRect.width() / 10;
        QRect bgRect = frameRect.adjusted(dist-1, dist-1, -dist, -dist).toRect();

        // The text's background rounded rectangle
        QPainterPath bgPath = Plasma::PaintUtils::roundedRectangle(bgRect, bgRect.height()/15);

        QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
        c.setAlphaF(.3);
        QColor outline = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        outline.setAlphaF(.5);
        p->setBrush(c);
        p->setPen(outline);
        p->setPen(QPen(outline, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->drawPath(bgPath);
        QString message = m_mySlideShow->message();

        // Set the font and draw text
        p->setRenderHint(QPainter::Antialiasing);
        QFont textFont = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
        textFont.setPixelSize(qMax(KGlobalSettings::smallestReadableFont().pixelSize(), bgRect.height() / 6));
        p->setFont(textFont);

        QTextOption option;
        option.setAlignment(Qt::AlignCenter);
        option.setWrapMode(QTextOption::WordWrap);
        p->setPen(QPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->drawText(bgRect, message, option);
    }

    p->end();
    delete p;
    m_dirty = false;
    update();
}

void Frame::nextPicture()
{
    m_mySlideShow->setUpdateInterval(0);
    m_mySlideShow->nextPicture();
    m_mySlideShow->setUpdateInterval(m_slideshowTime * 1000);
}

void Frame::previousPicture()
{
    m_mySlideShow->setUpdateInterval(0);
    m_mySlideShow->previousPicture();
    m_mySlideShow->setUpdateInterval(m_slideshowTime * 1000);
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
    parent->addPage(m_configDialog->appearanceSettings, i18n("Appearance"), "preferences-desktop-theme");
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
    setAssociatedApplicationUrls(m_currentUrl);
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
        m_slideShow = true;
    } else {
        kDebug() << "Remote URL" << droppedUrl.url();
        m_currentUrl = droppedUrl;
        setAssociatedApplicationUrls(m_currentUrl);
        m_slideShow = false;
    }

    stopPotd();
    initSlideShow();

    KConfigGroup cg = config();
    cg.writeEntry("url", m_currentUrl);
    cg.writeEntry("slideshow", m_slideShow);
    cg.writeEntry("slideshow paths", m_slideShowPaths);
    emit configNeedsSaving();
}

void Frame::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_slideShow) {
        checkSlideFrame();
        m_slideFrame->show();
    }

    Applet::hoverEnterEvent(event);
}

void Frame::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_slideShow) {
        checkSlideFrame();
        m_slideFrame->hide();
    }

    Applet::hoverLeaveEvent( event );
}

void Frame::checkSlideFrame()
{
    if (m_slideFrame)
        return;

    m_slideFrame = new Plasma::Frame( this );
    m_slideFrame->setZValue( 10 );

    m_backButton = new Plasma::ToolButton(m_slideFrame);
    m_backButton->setImage("widgets/arrows", "left-arrow");
    m_backButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_backButton->setMaximumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    connect(m_backButton, SIGNAL(clicked()), this , SLOT(previousPicture()));

    m_nextButton = new Plasma::ToolButton(m_slideFrame);
    m_nextButton->setImage("widgets/arrows", "right-arrow");
    m_nextButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_nextButton->setMaximumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    connect(m_nextButton, SIGNAL(clicked()), this , SLOT(nextPicture()));

    QGraphicsLinearLayout *buttonsLayout = new QGraphicsLinearLayout();
    buttonsLayout->addItem(m_backButton);
    buttonsLayout->addItem(m_nextButton);
    m_slideFrame->setLayout(buttonsLayout);
    buttonsLayout->activate();

    m_slideFrame->setFrameShadow( Plasma::Frame::Raised );
    m_slideFrame->hide();

    constraintsEvent(Plasma::SizeConstraint);
}

void Frame::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{

    if (contentsRect().size() != contentSizeHint()) {
        resize(contentSizeHint());
        emit appletTransformedItself();
    }

    //kDebug() << "Paint!";
    Q_UNUSED(option)
    if (m_slideShow) {
        // temporarily suspend the slideshow to allow time for loading the image
        m_mySlideShow->setUpdateInterval(0);
    }
    if (m_dirty) {
        updatePicture();
    }
    p->drawPixmap(rect, m_pixmap, rect);

    if (m_slideShow) {
        // unsuspend the slideshow to allow time for loading the image
        m_mySlideShow->setUpdateInterval(m_slideshowTime * 1000);
    }
}

#include "frame.moc"
