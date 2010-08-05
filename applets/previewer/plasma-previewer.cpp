/*
 *  Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>

 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "plasma-previewer.h"
#include "previeweradaptor.h"
#include "previewitemmodel.h"
#include "previewdialog.h"
#include "previewwidget.h"

// Qt
#include <QGraphicsSceneDragDropEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QLabel>

// KDE
#include <KIcon>
#include <KLocale>
#include <KParts/ReadOnlyPart>
#include <KMimeTypeTrader>
#include <KFileDialog>
#include <KUrl>
#include <KService>
#include <KIO/DeleteJob>
#include <KRun>
#include <KMessageBox>
#include <KDebug>

// Plasma
#include <plasma/theme.h>



Previewer::Previewer(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_base(0),
      m_dialog(0),
      m_part(0),
      m_currentFile(QString()),
      m_previewWidget(0)
{
    new PreviewerAdaptor(this);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/Previewer", this);

    setAcceptHoverEvents(true);
    setAcceptDrops(true);
    setBackgroundHints(NoBackground);

    resize(PreviewWidget::suggestedWidth(), 150);
    if (args.count()) {
        kDebug() << "Opening file from arg passed into applet ..." << args.value(0).toString();
        m_currentFile = args.value(0).toString();
        setAssociatedApplicationUrls(KUrl(m_currentFile));
    }
}

Previewer::~Previewer()
{
    if (!hasFailedToLaunch()) {
        if (m_part) {
            m_part->closeUrl();
            delete m_part;
            m_part = 0;
        }
    }
}

void Previewer::setStartSize()
{
    resize(PreviewWidget::suggestedWidth(), 150);
}

QGraphicsWidget* Previewer::graphicsWidget()
{
    if (!m_previewWidget) {
        m_previewWidget = new PreviewWidget(this);
        m_previewWidget->setMinimumSize(200, m_previewWidget->iconSize().height() + m_previewWidget->s_topBorder +
                                        m_previewWidget->bottomBorderHeight());
        m_previewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_previewWidget->setPreferredSize(m_previewWidget->minimumSize());
        connect(m_previewWidget, SIGNAL(urlsDropped(KUrl::List)), this, SLOT(openUrls(KUrl::List)));
        connect(m_previewWidget, SIGNAL(fileOpenRequested(KUrl)), this, SLOT(openFile(KUrl)));
    }

    return m_previewWidget;
}

void Previewer::init()
{
    setPopupIcon("previewer");
    graphicsWidget();
    if (!m_currentFile.isEmpty()) {
        openFile(m_currentFile);
    }
}

void Previewer::constraintsEvent(Plasma::Constraints constraints)
{
    Q_UNUSED(constraints)
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

void Previewer::setupPreviewDialog()
{
    if (m_dialog) {
        return;
    }

    m_dialog = new PreviewDialog();
    m_dialog->setWindowFlags(Qt::X11BypassWindowManagerHint);
    m_base = m_dialog->baseWidget();

    connect(m_dialog, SIGNAL(closeClicked()), this, SLOT(closeFile()));
    connect(m_dialog, SIGNAL(removeClicked()), this, SLOT(removeCurrentFromHistory()));
    connect(m_dialog, SIGNAL(runClicked()), this, SLOT(slotRunClicked()));
}

void Previewer::stayOnTop(bool top)
{
    setupPreviewDialog();
    bool visibility = m_dialog->isVisible();
    if (top) {
        m_dialog->setWindowFlags(Qt::X11BypassWindowManagerHint);
    } else {
        m_dialog->setWindowFlags(Qt::FramelessWindowHint);
    }

    m_dialog->setVisible(visibility);
}

void Previewer::openFile(KUrl u)
{
    kDebug() << "opening file";
    if (!u.isValid()) {
        u = KFileDialog::getOpenUrl();

        if (!u.isValid()) {
            return;
        }
    }

    //previewing = true;
    setupPreviewDialog();

    //kDebug()<<u;
    KMimeType::Ptr mimeType = KMimeType::findByUrl(u, 0, true);

    m_dialog->setMimeIcon(KIconLoader::global()->loadIcon(mimeType->iconName(), KIconLoader::Toolbar));

    delete m_part;
    m_part = 0;

    m_part = KMimeTypeTrader::createPartInstanceFromQuery<KParts::ReadOnlyPart>(mimeType->name(), m_base);

    if (!m_part) {
        return;
    }

    // this informs globally about the service used to make the part
    KService::Ptr service = KMimeTypeTrader::self()->preferredService(mimeType->name());
    m_currentService =  service->desktopEntryName();

    if (m_part->openUrl(u)) {
        kDebug() << "part successfully created";
        m_currentFile = u.pathOrUrl();
        setAssociatedApplicationUrls(KUrl(m_currentFile));
        m_dialog->setTitle(u.fileName());
        m_dialog->resize(600,500);
        QDesktopWidget *dw = QApplication::desktop();
        m_dialog->move(dw->screenGeometry(dw->screenNumber(QCursor::pos())).center() - QPoint(m_dialog->width()/2, m_dialog->height()/2));
        m_dialog->show();
        m_base->setFocus(Qt::TabFocusReason);

        if (!m_previewWidget->previews().contains(u)) {
            addPreview(u);
        }
        //   browser->setCurrentUrl(u);
    }
}

void Previewer::goToPage(uint page)
{
    if (m_currentService.isEmpty()) {
        return;
    }

    if (m_currentService.contains("okular")) {
        QMetaObject::invokeMethod(m_part, "goToPage", Qt::QueuedConnection, Q_ARG(uint, page));
    }
}

uint Previewer::currentPage()
{
    if (m_currentService.isEmpty()) {
        return 0;
    }

    if (m_currentService.contains("okular")) {
        uint res;
        QMetaObject::invokeMethod(m_part, "currentPage", Q_RETURN_ARG(uint, res));
        return res;
    }

    return 0;
}

void Previewer::openFile(QString p)
{
    openFile(KUrl(p));
}

void Previewer::closeFile(bool hide)
{
    //mime_icon->clear();
    setupPreviewDialog();
    m_dialog->setMimeIcon(QPixmap());
    m_dialog->titleLabel()->clear();
    m_dialog->setVisible(!hide);
}

void Previewer::removeCurrentFromHistory()
{
    KUrl cur(currentFile());
    int index = m_previewWidget->previews().indexOf(cur);

    m_dialog->setWindowFlags(Qt::FramelessWindowHint);
    m_dialog->show();

    int buttonCode = KMessageBox::questionYesNo(m_dialog, i18n("Are you sure you want to remove:\n%1", cur.pathOrUrl()),
                                                  i18n("Deleting File"));

    m_dialog->setWindowFlags(Qt::X11BypassWindowManagerHint);

    if (index != -1 && buttonCode == KMessageBox::Yes) {
        closeFile(false);
        delete m_part;
        m_part = 0;
        m_dialog->hide();
        KIO::del(cur);
        m_previewWidget->removeItem(index);
        return;
    }

    m_dialog->show();
}

void Previewer::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!KUrl::List::canDecode(event->mimeData())) {
        return;
    }

    const KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
    openUrls(urls);
}

void Previewer::addPreview(const QUrl& url, KMimeType::Ptr mimeType)
{
    kDebug() << "addPreview() reached";
    if (m_previewWidget->previews().contains(url)) {
        return;
    }

    if (!mimeType) {
        mimeType = KMimeType::findByUrl(KUrl(url), 0, true);

        if (!mimeType) {
            return;
        }
    }

    KService::List lst = KMimeTypeTrader::self()->query(mimeType->name(), "KParts/ReadOnlyPart");
    if (lst.isEmpty()) {
        // no KPart? we can't do them.
        return;
    }

    if (m_previewWidget->previews().isEmpty()) {
        // animating this is WICKED smooth =)
        m_previewWidget->animateHeight(true);
    }

    m_previewWidget->addItem(url);
}

QString Previewer::currentFile()
{
    return m_currentFile;
}

void Previewer::openUrls(KUrl::List list)
{
    foreach (const KUrl &url, list) {
        KMimeType::Ptr mimeType = KMimeType::findByUrl(url, 0, true);
        if (mimeType->is("inode/directory")) {
            QDir dir(url.pathOrUrl());
            dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

            QStringList entries = dir.entryList();
            foreach (const QString &entry, entries) {
                KUrl u = url;
                u.addPath(entry);
                addPreview(u);
            }
        } else {
            // this avoids items that cannot have a part
            addPreview(url, mimeType);
        }
    }
}

void Previewer::slotRunClicked()
{
    KUrl url(m_currentFile);
    KMimeType::Ptr mimeType = KMimeType::findByUrl(url, 0, true);
    KService::Ptr service = KMimeTypeTrader::self()->preferredService(mimeType->name());
    if (service) {
        //kDebug()<<service->exec();
        KRun::run(service->exec(), url, m_dialog);
    }

}

K_EXPORT_PLASMA_APPLET(previewer, Previewer)

#include "plasma-previewer.moc"

