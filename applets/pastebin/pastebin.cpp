/***************************************************************************
 *   Copyright (C) 2007 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
 *                         Artur Duque de Souza <morpheuz@gmail.com>       *
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

#include "pastebin.h"

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QFile>
#include <QBuffer>
#include <QPainter>
#include <QPaintEngine>
#include <QSignalMapper>

#include <KAction>
#include <KDebug>
#include <KLocale>
#include <KConfigDialog>
#include <KStandardAction>
#include <KToolInvocation>
#include <KNotification>

#include <kmimetype.h>
#include <ktemporaryfile.h>

#include <kio/global.h>
#include <kio/job.h>

#include <Plasma/Theme>

Pastebin::Pastebin(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_textServer(0),
    m_imageServer(0), m_textBackend(0), m_imageBackend(0),
    m_historySize(3), m_signalMapper(new QSignalMapper()), m_paste(0),
    m_topSeparator(0), m_bottomSeparator(0)
{
    setAcceptDrops(true);
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::ConstrainedSquare);
    setMinimumSize(16, 16);
    resize(150, 150);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showErrors()));

    connect(m_signalMapper, SIGNAL(mapped(const QString &)),
             this, SLOT(copyToClipboard(const QString &)));
}

Pastebin::~Pastebin()
{
    delete m_topSeparator;
    delete m_bottomSeparator;
    delete m_icon;
    QString history;
    for (int i = 0; i < m_actionHistory.size(); ++i) {
        history.prepend(m_actionHistory.at(i)->text());
        history.prepend("|");
        delete m_actionHistory.at(i);
    }

    KConfigGroup cg = config();
    cg.writeEntry("History", history);
}

void Pastebin::setImageServer(int backend)
{
    delete m_imageServer;

    switch(backend) {

    case Pastebin::IMAGEBINCA:
        m_imageServer = static_cast<ImagebinCAServer*>(new ImagebinCAServer(config()));
        break;

    case Pastebin::IMAGESHACK:
        m_imageServer = static_cast<ImageshackServer*>(new ImageshackServer(config()));
        break;
    }

    m_imageBackend = backend;
    connect(m_imageServer, SIGNAL(postFinished(QString)),
            this, SLOT(showResults(QString)));
    connect(m_imageServer, SIGNAL(postError()),
            this, SLOT(showErrors()));
}

void Pastebin::setTextServer(int backend)
{
    delete m_textServer;

    switch(backend) {

    case Pastebin::PASTEBINCA:
        m_textServer = static_cast<PastebinCAServer*>(new PastebinCAServer(config()));
        break;

    case Pastebin::PASTEBINCOM:
        m_textServer = static_cast<PastebinCOMServer*>(new PastebinCOMServer(config()));
        break;
    }

    m_textBackend = backend;
    connect(m_textServer, SIGNAL(postFinished(QString)),
            this, SLOT(showResults(QString)));
    connect(m_textServer, SIGNAL(postError()),
            this, SLOT(showErrors()));
}

void Pastebin::setHistorySize(int max)
{
    if (max <= 0)
        max = 0;

    while (max < m_actionHistory.count()) {
        delete m_actionHistory.takeFirst();
    }

    m_historySize = max;
}

void Pastebin::init()
{
    KConfigGroup cg = config();
    int textBackend = cg.readEntry("TextBackend", "0").toInt();
    int imageBackend = cg.readEntry("ImageBackend", "0").toInt();
    int historySize = cg.readEntry("HistorySize", "3").toInt();
    QStringList history = cg.readEntry("History", "").split("|", QString::SkipEmptyParts);

    for (int i = 0; i < history.size(); ++i) {
        addToHistory(history.at(i));
    }

    setTextServer(textBackend);
    setImageServer(imageBackend);
    setHistorySize(historySize);

    setActionState(Idle);
    setInteractionState(Waiting);
    m_icon = new KIcon("edit-paste"); // TODO: make member (for caching)

    updateTheme();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateTheme()));
    Plasma::ToolTipManager::self()->registerWidget(this);
    Plasma::ToolTipManager::self()->setContent(this, toolTipData);
}

void Pastebin::updateTheme()
{
    m_font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    m_font.setBold(true);
    m_fgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    m_bgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    m_linePen = QPen(m_fgColor);
    kDebug() << "Color" << m_bgColor << m_fgColor;
    update();
}

void Pastebin::setInteractionState(InteractionState state)
{
    switch (state ) {
        case Hovered:
            m_linePen.setStyle(Qt::DotLine);
            m_linePen.setWidth(2);
            showOverlay(true);
            break;
        case Waiting:
            showOverlay(false);
            break;
        case DraggedOver:
            m_linePen.setStyle(Qt::DashLine);
            m_linePen.setWidth(2);
            showOverlay(true);
            break;
        case Rejected:
            break;
        default:
            break;
    }
    m_interactionState = state;
}

void Pastebin::setActionState(ActionState state)
{
    toolTipData = Plasma::ToolTipContent();
    toolTipData.setAutohide(true);

    toolTipData.setMainText("Pastebin");

    //TODO: choose icons for each state

    switch (state ) {
        case Unset:
            toolTipData.setSubText(i18nc("The status of the applet has not been set - i.e. it is unset.", "Unset"));
            toolTipData.setImage(KIcon("edit-paste"));
            break;
        case Idle:
            setBusy(false);
            toolTipData.setSubText(i18n("Drop text or an image onto me to upload it to Pastebin."));
            toolTipData.setImage(KIcon("edit-paste"));
            break;
        case IdleError:
            setBusy(false);
            toolTipData.setSubText(i18n("Error during upload. Try again."));
            toolTipData.setImage(KIcon("dialog-cancel"));
            // Notification ...
            QTimer::singleShot(15000, this, SLOT(resetActionState()));
            timer->stop();
            break;
        case IdleSuccess:
            setBusy(false);
            toolTipData.setSubText(i18n("Successfully uploaded to %1.", m_url));
            toolTipData.setImage(KIcon("dialog-ok"));
            // Notification ...
            QTimer::singleShot(15000, this, SLOT(resetActionState()));
            timer->stop();
            break;
        case Sending:
            setBusy(true);
            toolTipData.setSubText(i18n("Sending...."));
            toolTipData.setImage(KIcon("view-history"));
            break;
        default:
            break;
    }

    Plasma::ToolTipManager::self()->setContent(this, toolTipData);
    m_actionState = state;
    update();
}

void Pastebin::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & (Plasma::FormFactorConstraint | Plasma::SizeConstraint)) {
        int minSize = KGlobalSettings::smallestReadableFont().pointSize();
        int dynSize = qMax((qreal)1.0, (qreal)(qMin(contentsRect().width(), contentsRect().height()) / 4));
        //kDebug() << "Min : Dyn" << minSize << dynSize << qMax(minSize, dynSize);
        m_font.setPointSize(qMax(minSize, dynSize));
    }
}

int Pastebin::iconSize()
{
    // return the biggest fitting icon size from KIconLoader
    int c = qMin(contentsRect().width(), contentsRect().height());
    int s;
    if (c >= KIconLoader::SizeEnormous) { // 128
        s = KIconLoader::SizeEnormous;
    } else if (c >= KIconLoader::SizeHuge) { // 64
        s = KIconLoader::SizeHuge;
    } else if (c >= KIconLoader::SizeLarge) { // 48
        s = KIconLoader::SizeLarge;
    } else if (c >= KIconLoader::SizeMedium) { // 32
        s = KIconLoader::SizeMedium;
    } else if (c >= KIconLoader::SizeSmallMedium) { // 32
        s = KIconLoader::SizeSmallMedium;
    } else { // 16
        s = KIconLoader::SizeSmall;
    }
    return s;
}

void Pastebin::paintPixmap(QPainter *painter, QPixmap &pixmap, const QRectF &rect, qreal opacity)
{
    int size = pixmap.size().width();
    QPointF iconOrigin = QPointF(rect.left() + (rect.width() - size) / 2,
                                 rect.top() + (rect.height() - size) / 2);

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::Antialiasing);

    if (!painter->paintEngine()->hasFeature(QPaintEngine::ConstantOpacity)) {
        QPixmap temp(QSize(size, size));
        temp.fill(Qt::transparent);

        QPainter p;
        p.begin(&temp);

        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(QPoint(0,0), pixmap);

        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(pixmap.rect(), QColor(0, 0, 0, opacity * 254));
        p.end();

        // draw the pixmap
        painter->drawPixmap(iconOrigin, temp);
    } else {
        // FIXME: Works, but makes hw acceleration impossible, use above code path
        qreal old = painter->opacity();
        painter->setOpacity(opacity);
        painter->drawPixmap(iconOrigin, pixmap);
        painter->setOpacity(old);
    }
}

void Pastebin::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *, const QRect &contentsRect)
{
    if (!contentsRect.isValid() || isBusy()) {
        return;
    }

    // BusyWidget is being shown
    if (m_actionState == Sending) {
        return;
    }

    // Fade in the icon on top of it
    int iconsize = iconSize();
    qreal pix_alpha = 1.0 - (0.5 * m_alpha); // Fading out to .5

    QPointF iconOrigin = QPointF(contentsRect.left() + (contentsRect.width() - iconsize) / 2,
                                 contentsRect.top() + (contentsRect.height() - iconsize) / 2);
    QRectF iconRect = QRectF(iconOrigin, QSize(iconsize, iconsize));

    if (m_actionState == IdleSuccess) {
        QPixmap iconok = KIcon("dialog-ok").pixmap(QSize(iconsize, iconsize));
        paintPixmap(p, iconok, iconRect, pix_alpha);
    } else if (m_actionState == IdleError) {
        QPixmap iconok = KIcon("dialog-cancel").pixmap(QSize(iconsize, iconsize));
        paintPixmap(p, iconok, iconRect, pix_alpha);
    } else {
        QPixmap iconpix = m_icon->pixmap(QSize(iconsize, iconsize));
        paintPixmap(p, iconpix, iconRect, pix_alpha);
    }


    // Draw background
    if (m_interactionState == DraggedOver) {
        m_fgColor.setAlphaF(m_alpha);
    } else if (m_interactionState == Hovered) {
        m_fgColor.setAlphaF(m_alpha * 0.15);
    } else {
        // Whatever, as long as it goes down to 0
        m_fgColor.setAlphaF(m_alpha * 0.15);
    }
    m_bgColor.setAlphaF(m_alpha * 0.3);

    p->setBrush(m_bgColor);
    m_linePen.setColor(m_fgColor);
    p->setPen(m_linePen);
    p->setFont(m_font);

    qreal proportion = contentsRect.width() / contentsRect.height();
    qreal round_radius = 35.0;
    p->drawRoundedRect(contentsRect, round_radius / proportion, round_radius, Qt::RelativeSize);

    QString hoverText = i18n("Drop!");
    if (m_interactionState == DraggedOver) {
        m_fgColor.setAlphaF(m_alpha * 0.8);
    } else if (m_interactionState == Hovered) {
        m_fgColor.setAlphaF(m_alpha * 0.6);
    }
    p->drawText(contentsRect, Qt::AlignCenter, hoverText);
    p->setPen(m_fgColor);

    if (m_actionState == IdleSuccess) {
        QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
        font.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
        p->setFont(font);
        p->drawText(contentsRect, Qt::AlignCenter, m_url);
    } else if (m_actionState == IdleError) {
        QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
        font.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
        font.setBold(true);
        p->setFont(font);
        p->drawText(contentsRect, Qt::AlignCenter, i18n("Error: Try Again"));
    } else {
        p->setFont(m_font);
        p->drawText(contentsRect, Qt::AlignCenter, hoverText);
    }
}

void Pastebin::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setInteractionState(Hovered);
    Applet::hoverEnterEvent(event);
}

void Pastebin::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setInteractionState(Waiting);
    Applet::hoverLeaveEvent(event);
}

void Pastebin::showOverlay(bool show)
{
    if (m_fadeIn == show) {
        return;
    }
    m_fadeIn = show;
    const int FadeInDuration = 400;

    if (m_animId != -1) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }
    m_animId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,
                                                      Plasma::Animator::EaseOutCurve, this,
                                                      "animationUpdate");
}

void Pastebin::animationUpdate(qreal progress)
{
    if (progress == 1) {
        m_animId = -1;
    }
    if (!m_fadeIn) {
        qreal new_alpha = m_fadeIn ? progress : 1 - progress;
        m_alpha = qMin(new_alpha, m_alpha);
    } else {
        m_alpha = m_fadeIn ? progress : 1 - progress;
    }
    m_alpha = qMax(qreal(0.0), m_alpha);
    update();
}


void Pastebin::createConfigurationInterface(KConfigDialog *parent)
{
    KConfigGroup cg = config();

    QWidget *general = new QWidget();
    uiConfig.setupUi(general);
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(general, i18n("General"), Applet::icon());

    uiConfig.textServer->setCurrentIndex(m_textBackend);
    uiConfig.imageServer->setCurrentIndex(m_imageBackend);
    uiConfig.historySize->setValue(m_historySize);

    QWidget *servers = new QWidget();
    uiServers.setupUi(servers);
    parent->addPage(servers, i18n("Servers"), Applet::icon());

    QString pastebincaURL = cg.readEntry("pastebinca", "http://pastebin.ca");
    uiServers.pastebinca->setText(pastebincaURL);

    QString pastebincomURL = cg.readEntry("pastebincom", "http://pastebin.com");
    uiServers.pastebincom->setText(pastebincomURL);

    QString imagebincaURL = cg.readEntry("imagebinca", "http://imagebin.ca");
    uiServers.imagebinca->setText(imagebincaURL);

    QString imageshackURL = cg.readEntry("imageshack", "http://imageshack.us");
    uiServers.imageshack->setText(imageshackURL);
}

void Pastebin::configAccepted()
{
    KConfigGroup cg = config();
    int textBackend = uiConfig.textServer->currentIndex();
    int imageBackend = uiConfig.imageServer->currentIndex();
    int historySize = uiConfig.historySize->value();

    QString pastebincaURL = uiServers.pastebinca->text();
    QString pastebincomURL = uiServers.pastebincom->text();
    QString imagebincaURL = uiServers.imagebinca->text();
    QString imageshackURL = uiServers.imageshack->text();

    cg.writeEntry("TextBackend", textBackend);
    cg.writeEntry("ImageBackend", imageBackend);
    cg.writeEntry("HistorySize", historySize);

    cg.writeEntry("pastebinca", pastebincaURL);
    cg.writeEntry("pastebincom", pastebincomURL);
    cg.writeEntry("imagebinca", imagebincaURL);
    cg.writeEntry("imageshack", imageshackURL);

    setTextServer(textBackend);
    setImageServer(imageBackend);
    setHistorySize(historySize);
    emit configNeedsSaving();
}

void Pastebin::showResults(const QString &url)
{
    timer->stop();
    m_url = url;
    setActionState(IdleSuccess);
    copyToClipboard(url);

    addToHistory(url);
}

void Pastebin::copyToClipboard(const QString &url)
{
    QApplication::clipboard()->setText(url);
    kDebug() << "Copying:" << url;
    QPixmap pix = KIcon("edit-paste").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium);

    KNotification *notify = new KNotification("urlcopied");
    notify->setComponentData(KComponentData("plasma_pastebin"));
    notify->setText(i18nc("Notification when the pastebin applet has copied the URL to the clipboard",
                          "%1 has been copied to your clipboard", url));
    notify->setPixmap(pix);
    notify->setActions(QStringList(i18n("Open browser")));
    connect(notify, SIGNAL(action1Activated()), this, SLOT(openLink()));
    notify->sendEvent();
}

void Pastebin::showErrors()
{
    setActionState(IdleError);
}

void Pastebin::openLink()
{
    KToolInvocation::invokeBrowser(m_url);
}

void Pastebin::resetActionState()
{
    setActionState(Idle);
    update();
}

void Pastebin::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_url.isEmpty() || event->button() != Qt::LeftButton) {
        Plasma::Applet::mousePressEvent(event);
    } else {
        openLink();
    }
    if (event->button() == Qt::MidButton) {
        if (m_actionState == Idle) {
            // paste clipboard content
#ifdef Q_WS_WIN
            // Same as for D'n'D, Windows doesn't pass any actual image data when pasting
            // image files. Though, it does provide us with those files' Urls. Since posting
            // multiple images isn't yet implemented - we'll use first Url from list
            QImage image;
            QString imageFileName;
            if (QApplication::clipboard()->mimeData()->hasUrls()) {
                imageFileName = QApplication::clipboard()->mimeData()->urls().at(0).toLocalFile();
                image.load(imageFileName);
                postContent(imageFileName, image);
            } else {
                postContent(QApplication::clipboard()->mimeData()->text(), image);
            };
#else
            postContent(QApplication::clipboard()->text(), QApplication::clipboard()->image());
#endif //Q_WS_WIN
        } else {
            // Now releasing the middlebutton click copies to clipboard
            event->accept();
        }
    }
}

void Pastebin::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if (!m_url.isEmpty() && event->button() == Qt::MidButton && m_actionState == IdleSuccess) {
        copyToClipboard(m_url);
    }
}

void Pastebin::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    InteractionState istate = Rejected;
    if (event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
    foreach (const QString &f, event->mimeData()->formats()) {
        if (f.indexOf("image/") != -1) {
            istate = DraggedOver;
        }
    }
    if (event->mimeData()->hasImage() || event->mimeData()->hasText()) {
        istate = DraggedOver;
    }
    setInteractionState(istate);
}

void Pastebin::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    setInteractionState(Waiting);
}

void Pastebin::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}

void Pastebin::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->objectName() != QString("Pastebin-applet")) {
#ifdef Q_WS_WIN
        // Apparently, Windows doesn't pass any actual image data when drag'n'dropping
        // image files. Though, it does provide us with those files' Urls. Since posting
        // multiple images isn't yet implemented - we'll use first Url from list
        QImage image;
        QString imageFileName;
        if (event->mimeData()->hasUrls()) {
            imageFileName = event->mimeData()->urls().at(0).toLocalFile();
            image.load(imageFileName);
            postContent(imageFileName, image);
        } else {
            postContent(event->mimeData()->text(), image);
        };
#else
        QImage image = qvariant_cast<QImage>(event->mimeData()->imageData());
        postContent(event->mimeData()->text(), image);
#endif //Q_WS_WIN
        event->acceptProposedAction();
    }
}

void Pastebin::addToHistory(const QString &url)
{
    if (m_historySize <= 0) {
        return;
    }

    if (m_actionHistory.size() >= m_historySize) {
        delete m_actionHistory.takeLast();
    }

    QAction *ac = new QAction(url, this);
    m_actionHistory.insert(0, ac);
    m_signalMapper->setMapping(ac, url);
    connect(ac, SIGNAL(triggered(bool)), m_signalMapper, SLOT(map()));
}

QList<QAction*> Pastebin::contextualActions()
{
    if (!m_paste) {
        m_paste = KStandardAction::paste(this);
        connect(m_paste, SIGNAL(triggered(bool)), this, SLOT(postClipboard()));
    }
    if (!m_topSeparator) {
        m_topSeparator = new QAction(this);
        m_topSeparator->setSeparator(true);
    }
    if (!m_bottomSeparator) {
        m_bottomSeparator = new QAction(this);
        m_bottomSeparator->setSeparator(true);
    }

    m_contextualActions.clear();

    m_contextualActions.append(m_paste);
    m_contextualActions.append(m_topSeparator);

    m_contextualActions.append(m_actionHistory);

    if (!m_actionHistory.isEmpty()) {
        m_contextualActions.append(m_bottomSeparator);
    }

    return m_contextualActions;
}

void Pastebin::postClipboard()
{
#ifdef Q_WS_WIN
// Same as for D'n'D, Windows doesn't pass any actual image data when pasting
// image files. Though, it does provide us with those files' Urls. Since posting
// multiple images isn't yet implemented - we'll use first Url from list
    QImage image;
    QString imageFileName;
    if (QApplication::clipboard()->mimeData()->hasUrls()) {
        imageFileName = QApplication::clipboard()->mimeData()->urls().at(0).toLocalFile();
        image.load(imageFileName);
        postContent(imageFileName, image);
    } else {
        postContent(QApplication::clipboard()->mimeData()->text(), image);
    };
#else
    postContent(QApplication::clipboard()->text(), QApplication::clipboard()->image());
#endif //Q_WS_WIN
}

void Pastebin::postContent(QString text, QImage imageData)
{
    bool image = false;
    bool validPath = false;

    setActionState(Sending);
    timer->start(20000);

    KUrl testPath(text);
    validPath = QFile::exists(testPath.toLocalFile());

    if (validPath) {
        KMimeType::Ptr type = KMimeType::findByPath(testPath.path());

        if (type->name().indexOf("image/") != -1) {
            image = true;
        }
    } else {
        if (!imageData.isNull()) {
            image = true;
        }
    }

    if (!image) {
        if (validPath) {
            QFile file(testPath.toLocalFile());
            file.open(QIODevice::ReadOnly);
            QTextStream in(&file);
            text = in.readAll();
        }

        // upload text
        m_textServer->post(text);
    } else {
        //upload image
        if (validPath) {
            m_imageServer->post(testPath.toLocalFile());
        } else {
            KTemporaryFile tempFile;
            if (tempFile.open()) {
                tempFile.setAutoRemove(false);

                QDataStream stream(&tempFile);

                QByteArray data;
                QBuffer buffer(&data);
                buffer.open(QIODevice::ReadWrite);

                QImage image = imageData;
                image.save(&buffer, "JPEG");
                stream.writeRawData(data, data.size());

                KUrl t(tempFile.fileName());
                tempFile.close();

                m_imageServer->post(t.path());

            } else {
                setActionState(IdleError);
                timer->stop();
            }
        }
    }
}


#include "pastebin.moc"
