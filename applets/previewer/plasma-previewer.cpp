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
#include <QPainter>
#include <QGraphicsSceneDragDropEvent>
#include <QLabel>
#include <QPalette>
#include <QDesktopWidget>
#include <QApplication>

// KDE
#include <KIcon>
#include <KLocale>
#include <KParts/ReadOnlyPart>
#include <KMimeTypeTrader>
#include <KFileDialog>
#include <KMenu>
#include <KAction>
#include <KUrl>
#include <KService>
#include <KGlobalSettings>
#include <KFileItem>
#include <KIO/DeleteJob>
#include <KRun>
#include <KMessageBox>

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

    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    resize(PreviewWidget::suggestedWidth(), 150);
    if (args.count()) {
        kDebug() << "Opening file from arg passed into applet ..." << args.value(0).toString();
        m_currentFile = args.value(0).toString();
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

QGraphicsWidget* Previewer::graphicsWidget()
{
    if (!m_previewWidget) {
        m_previewWidget = new PreviewWidget(this);
        m_previewWidget->setMinimumSize(200, m_previewWidget->iconSize().height() + m_previewWidget->s_topBorder +
                                        m_previewWidget->bottomBorderHeight());
        m_previewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(m_previewWidget, SIGNAL(urlsDropped(KUrl::List)), this, SLOT(openUrls(KUrl::List)));
        connect(m_previewWidget, SIGNAL(fileOpenRequested(KUrl)), this, SLOT(openFile(KUrl)));
    }

    return m_previewWidget;
}

void Previewer::init()
{
    setPopupIcon("previewer");
    setupActions();
    if (!m_currentFile.isEmpty()) {
        openFile(m_currentFile);
    }
}

void Previewer::constraintsEvent(Plasma::Constraints constraints)
{
    Q_UNUSED(constraints)
    setBackgroundHints(NoBackground);
}

void Previewer::setupActions()
{
    KAction *open = new KAction(KIcon("document-preview"), i18n("&Open"), this);
    connect(open, SIGNAL(triggered()), this, SLOT(openFile()));

    m_actions << open;

    KAction *onTop = new KAction(i18n("Preview Dialog Always on Top"), this);
    onTop->setCheckable(true);
    onTop->setChecked(true);
    connect(onTop, SIGNAL(toggled(bool)), this, SLOT(stayOnTop(bool)));
    m_actions << onTop;

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_actions << sep;

    m_recents = new KMenu();
    m_recents->setIcon(KIcon("document-open-recent"));
    QAction *r_action = m_recents->menuAction();
    r_action->setText(i18n("Recently Opened"));
    m_actions<<r_action;
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
    kDebug()<<"opening file";
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
        m_dialog->setTitle(u.fileName());
        m_dialog->resize(600,500);
        QDesktopWidget *dw = QApplication::desktop();
        m_dialog->move(dw->screenGeometry(dw->screenNumber(QCursor::pos())).center() - QPoint(m_dialog->width()/2, m_dialog->height()/2));
        m_dialog->show();
        m_base->setFocus(Qt::TabFocusReason);

        if (!m_previewHistory.contains(u)) {
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
    int index = m_previewHistory.indexOf(cur);
    kDebug() << index;

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
        removeRecent(index);
        KIO::del(cur);
        m_previewWidget->setItemsList(m_previewHistory);
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
    kDebug()<<"addPreview() reached";
    if (m_previewHistory.contains(url)) {
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

    if (!m_previewWidget) {
        // make sure it's initialized!
        graphicsWidget();
    }

    if (m_previewHistory.isEmpty()) {
        // animating this is WICKED smooth =)
        m_previewWidget->expand();
    }

    KAction *a = new KAction(KIcon(mimeType->iconName()), KUrl(url).fileName(), this);
    a->setData(url);
    connect(a, SIGNAL(triggered()), this, SLOT(reopenPreview()));

    addRecent(a, url);
    m_previewWidget->setItemsList(m_previewHistory);
}

void Previewer::reopenPreview()
{
    KAction *s = qobject_cast<KAction*>(sender());
    if (s) {
        openFile(s->data().toUrl());
    }
}

QList<QAction*> Previewer::contextualActions()
{
    return m_actions;
}

void Previewer::removeRecent(int index)
{
    m_previewHistory.takeAt(index);
    delete m_recents->actions().takeAt(index);
}

void Previewer::addRecent(QAction *action, const QUrl &url)
{
    m_previewHistory.append(url);
    m_recents->addAction(action);
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
        kDebug()<<service->exec();
        KRun::run(service->exec(), url, m_dialog);
    }

}

K_EXPORT_PLASMA_APPLET(previewer, Previewer)

#include "plasma-previewer.moc"

