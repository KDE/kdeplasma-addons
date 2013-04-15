/***************************************************************************
 *   Copyright (C) 2007 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
 *                         Artur Duque de Souza <asouza@kde.org>           *
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

#include <KConfigDialog>
#include <KStandardAction>
#include <KToolInvocation>
#include <KNotification>
#include <KAction>

#include <kmimetype.h>
#include <KTemporaryFile>
#include <KDE/KIO/MimetypeJob>
#include <KDebug>

#include <kio/global.h>
#include <kio/job.h>

#include <Plasma/Theme>
#include <Plasma/Service>
#include <Plasma/ServiceJob>
#include <KNS3/DownloadDialog>

#include <QApplication>
#include <QClipboard>
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QFile>
#include <QBuffer>
#include <QPainter>
#include <QPaintEngine>
#include <QSignalMapper>
#include <QPropertyAnimation>


Pastebin::Pastebin(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_signalMapper(new QSignalMapper()), m_paste(0),
      m_topSeparator(0), m_bottomSeparator(0),
      m_historySize(3), m_newStuffDialog(0)
{
    setAcceptDrops(true);
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::ConstrainedSquare);
    setMinimumSize(16, 16);
    resize(150, 150);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(showErrors()));

    connect(m_signalMapper, SIGNAL(mapped(QString)),
             this, SLOT(copyToClipboard(QString)));
    connect(this, SIGNAL(activate()), this, SLOT(postClipboard()));

    // connect to all sources of our 'share' dataengine
    m_engine = dataEngine("org.kde.plasma.dataengine.share");
    m_engine->connectAllSources(this);

    // to detect when the mimetypes were added again after a refresh
    connect(m_engine, SIGNAL(sourceAdded(QString)),
            this, SLOT(sourceAdded(QString)));
    connect(m_engine, SIGNAL(sourceRemoved(QString)),
            this, SLOT(sourceRemoved(QString)));
}

// save history of URLs
void Pastebin::saveHistory()
{
    QString history;
    const int numberOfActionHistory = m_actionHistory.size();
    for (int i = 0; i < numberOfActionHistory; ++i) {
        history.prepend(m_actionHistory.at(i)->toolTip());
        history.prepend('|');
    }

    KConfigGroup cg = config();
    cg.writeEntry("History", history);
}

Pastebin::~Pastebin()
{
    delete m_topSeparator;
    delete m_bottomSeparator;
    delete m_newStuffDialog;

    saveHistory();
    const int numberOfActionHistory = m_actionHistory.size();
    for (int i = 0; i < numberOfActionHistory; ++i) {
        delete m_actionHistory.at(i);
    }
}

void Pastebin::init()
{
    configChanged();
    setActionState(Idle);
    setInteractionState(Waiting);

    updateTheme();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateTheme()));
    Plasma::ToolTipManager::self()->registerWidget(this);
    Plasma::ToolTipManager::self()->setContent(this, toolTipData);
}

void Pastebin::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    // update the options
    if (source != "Mimetypes") {
        const QString mimetype = data.value("Mimetypes").toString();

        if (mimetype.startsWith("text/")) {
            m_txtServers.insert(data.value("Name").toString(), source);
        } else if (mimetype.startsWith("image/")) {
            m_imgServers.insert(data.value("Name").toString(), source);
        } else {
            kDebug() << "Mimetype not supported by this applet";
        }
    }
}

void Pastebin::sourceAdded(const QString &source)
{
    // update the options
    if (source != "Mimetypes") {
        const Plasma::DataEngine::Data data = m_engine->query(source);
        const QString mimetype = data.value("Mimetypes").toString();

        if (mimetype.startsWith("text/")) {
            m_txtServers.insert(data.value("Name").toString(), source);
        } else if (mimetype.startsWith("image/")) {
            m_imgServers.insert(data.value("Name").toString(), source);
        } else {
            kDebug() << "Mimetype not supported by this applet";
        }
    }
}

void Pastebin::sourceRemoved(const QString &source)
{
    // update the options
    if (source != "Mimetypes") {
        QString key = m_txtServers.key(source);
        m_txtServers.remove(key);

        key = m_imgServers.key(source);
        m_imgServers.remove(key);
    }
}

void Pastebin::setHistorySize(int max)
{
    if (max <= 0) {
        max = 0;
    }
    while (max < m_actionHistory.count()) {
        delete m_actionHistory.takeFirst();
    }

    m_historySize = max;
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
            m_timer->stop();
            break;
        case IdleSuccess:
            setBusy(false);
            toolTipData.setSubText(i18n("Successfully uploaded to %1.", m_url));
            toolTipData.setImage(KIcon("dialog-ok"));
            // Notification ...
            QTimer::singleShot(15000, this, SLOT(resetActionState()));
            m_timer->stop();
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
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical ) {
            setPreferredSize(-1, -1);
        } else {
            setPreferredSize(150, 150);
        }
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
        QPixmap iconpix = KIcon("edit-paste").pixmap(QSize(iconsize, iconsize));
        if (!iconpix.isNull()) {
            paintPixmap(p, iconpix, iconRect, pix_alpha);
        }
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

    QPropertyAnimation *animation = m_animation.data();
    if (!animation) {
        animation = new QPropertyAnimation(this, "animationUpdate");
        animation->setDuration(400);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::Linear);
        m_animation = animation;
    } else if (animation->state() == QAbstractAnimation::Running) {
        animation->pause();
    }

    if (m_fadeIn) {
        animation->setDirection(QAbstractAnimation::Forward);
        animation->start(QAbstractAnimation::KeepWhenStopped);
    } else {
        animation->setDirection(QAbstractAnimation::Backward);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

qreal Pastebin::animationValue() const
{
    return m_alpha;
}

void Pastebin::animationUpdate(qreal progress)
{
    m_alpha = progress;
    update();
}

void Pastebin::getNewStuff()
{
    if (!m_newStuffDialog) {
        QString ghns("pastebin.knsrc");
        m_newStuffDialog = new KNS3::DownloadDialog( ghns );
        connect(m_newStuffDialog, SIGNAL(accepted()),
                this, SLOT(newStuffFinished()));
    }
    m_newStuffDialog->show();
}

void Pastebin::newStuffFinished()
{
    if ( m_newStuffDialog->changedEntries().count() ) {
        // refresh the options of config dialog
        refreshConfigDialog();

        // setup the config dialog to last options
        KConfigGroup cg = config();
        uiConfig.textServer->setCurrentItem(cg.readEntry("TextProvider", ""));
        uiConfig.imageServer->setCurrentItem(cg.readEntry("ImageProvider", ""));
    }
}

void Pastebin::refreshConfigDialog()
{
    // setup text
    uiConfig.textServer->clear();
    uiConfig.textServer->addItems(m_txtServers.keys());

    // setup image
    uiConfig.imageServer->clear();
    uiConfig.imageServer->addItems(m_imgServers.keys());
}

void Pastebin::createConfigurationInterface(KConfigDialog *parent)
{
    KConfigGroup cg = config();

    QWidget *general = new QWidget();
    uiConfig.setupUi(general);

    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    parent->addPage(general, i18n("General"), Applet::icon());

    uiConfig.ghnsButton->setIcon(KIcon("get-hot-new-stuff"));
    connect(uiConfig.ghnsButton, SIGNAL(clicked()), this, SLOT(getNewStuff()));

    refreshConfigDialog();
    uiConfig.textServer->setCurrentItem(cg.readEntry("TextProvider", m_txtServers.keys().at(0)));
    uiConfig.imageServer->setCurrentItem(cg.readEntry("ImageProvider", m_imgServers.keys().at(0)));
    uiConfig.historySize->setValue(m_historySize);

    connect(uiConfig.textServer , SIGNAL(currentIndexChanged(int)) , parent, SLOT(settingsModified()));
    connect(uiConfig.imageServer , SIGNAL(currentIndexChanged(int)) , parent, SLOT(settingsModified()));
    connect(uiConfig.historySize , SIGNAL(valueChanged(int)) , parent, SLOT(settingsModified()));
}

void Pastebin::configAccepted()
{
    KConfigGroup cg = config();
    int historySize = uiConfig.historySize->value();
    setHistorySize(historySize);

    cg.writeEntry("TextProvider", uiConfig.textServer->currentText());
    cg.writeEntry("ImageProvider", uiConfig.imageServer->currentText());
    cg.writeEntry("HistorySize", historySize);
    saveHistory();

    emit configNeedsSaving();
}

void Pastebin::configChanged()
{
    KConfigGroup cg = config();
    int historySize = cg.readEntry("HistorySize", "3").toInt();
    QStringList history = cg.readEntry("History", "").split('|', QString::SkipEmptyParts);

    m_actionHistory.clear();
    setHistorySize(historySize);
    const int numberOfItems = history.size();
    for (int i = 0; i < numberOfItems; ++i) {
        addToHistory(history.at(i));
    }
}

void Pastebin::showResults(const QString &url)
{
    m_timer->stop();
    m_url = url;
    setActionState(IdleSuccess);
    copyToClipboard(url);
    addToHistory(url);
}

void Pastebin::copyToClipboard(const QString &url)
{
    QApplication::clipboard()->setText(url, lastMode);
    kDebug() << "Copying:" << url;
    m_oldUrl = url;
    QPixmap pix = KIcon("edit-paste").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium);

    KNotification *notify = new KNotification("urlcopied");
    notify->setComponentData(KComponentData("plasma_pastebin"));
    notify->setText(i18nc("Notification when the pastebin applet has copied the URL to the clipboard",
                          "The URL for your paste has been copied to the clipboard"));
    notify->setPixmap(pix);
    notify->setActions(QStringList(i18n("Open browser")));
    connect(notify, SIGNAL(action1Activated()), this, SLOT(openLink()));
    notify->sendEvent();
}

void Pastebin::showErrors()
{
    setActionState(IdleError);
}

void Pastebin::openLink(bool old)
{
    if(old) {
      KToolInvocation::invokeBrowser(m_oldUrl);
    } else {
      KToolInvocation::invokeBrowser(m_url);
    }
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
        openLink(false);
    }
    if (event->button() == Qt::MidButton) {
        if (m_actionState == Idle) {
            // paste clipboard content
            postClipboard(true);
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
        lastMode = QClipboard::Clipboard;
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
    postClipboard(false);
}

void Pastebin::postClipboard(bool preferSelection)
{
    lastMode = QClipboard::Clipboard;
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
    if (preferSelection) {
        lastMode = QApplication::clipboard()->supportsSelection() ? QClipboard::Selection : QClipboard::Clipboard;
        postContent(QApplication::clipboard()->text(lastMode), QApplication::clipboard()->image(lastMode));
    } else {
        postContent(QApplication::clipboard()->text(), QApplication::clipboard()->image());
    }
#endif //Q_WS_WIN
}

void Pastebin::postContent(QString text, const QImage& imageData)
{
    QString sourceName;
    KUrl testPath(text);
    bool validPath = true;

    // use KIO to check if the file exists using mimetype job
    KIO::MimetypeJob *mjob = KIO::mimetype(testPath);
    if (!mjob->exec()) {
        // it's not a file - usually this happens when we are
        // just sharing plain text or image
        validPath = false;
    }

    KConfigGroup cg = config();
    // This is needed to provide smooth transition between old config and new one
    const QString txtProvider = cg.readEntry("TextProvider", m_txtServers.keys().at(0));
    const QString imgProvider = cg.readEntry("ImageProvider", m_imgServers.keys().at(0));

    bool isTemporary = false;
    if (validPath) {
        KMimeType::Ptr type = KMimeType::findByPath(testPath.path());

        if (type->name().indexOf("image/") != -1) {
            // its image
            sourceName = m_imgServers.value(imgProvider);
        } else {
            // its text
            sourceName = m_txtServers.value(txtProvider);
        }
    } else if (imageData.isNull()) {
        sourceName = m_txtServers.value(txtProvider);
    } else {
        sourceName = m_imgServers.value(imgProvider);

        KTemporaryFile tempFile;
        tempFile.setSuffix(".png");
        if (tempFile.open()) {
            tempFile.setAutoRemove(false);

            imageData.save(&tempFile, "PNG");
            tempFile.close();

            text = tempFile.fileName();

            isTemporary = true;
        } else {
            setActionState(IdleError);
            return;
        }
    }

    kDebug() << "Is valid path: " << validPath;
    kDebug() << "Provider used: " << sourceName;

    if (sourceName.isEmpty()) {
        // no provider was configured
        showErrors();
        return;
    }

    m_postingService = m_engine->serviceForSource(sourceName);
    KConfigGroup ops = m_postingService->operationDescription("share");
    ops.writeEntry("content", text);

    Plasma::ServiceJob *job = m_postingService->startOperationCall(ops);
    if (isTemporary) // Store tempfile-job mapping for cleanup when finished.
        m_pendingTempFileJobs[job] = text;
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(postingFinished(KJob*)));

    setActionState(Sending);
    m_timer->start(20000);
}

void Pastebin::postingFinished(KJob *job)
{
    Plasma::ServiceJob *sjob = static_cast<Plasma::ServiceJob*>(job);
    if (sjob->error()) {
        showErrors();
    } else {
        showResults(sjob->result().toString());
    }

    // Cleanup of temp file...
    QString tempUrl = m_pendingTempFileJobs.take(job);
    if (!tempUrl.isEmpty())
        KIO::file_delete(KUrl(tempUrl), KIO::HideProgressInfo);
}



#include "pastebin.moc"
