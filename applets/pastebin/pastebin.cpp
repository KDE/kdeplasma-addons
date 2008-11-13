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
#include <QUrl>
#include <QFile>

#include <KDebug>
#include <KLocale>
#include <KConfigDialog>
#include <KToolInvocation>
#include <kmimetype.h>

#include <kio/global.h>
#include <kio/job.h>

#include <plasma/widgets/flashinglabel.h>
#include <plasma/widgets/label.h>

using namespace Plasma;

Pastebin::Pastebin(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_text_server(0), m_image_server(0),
      m_text_backend(0), m_image_backend(0),
      m_text(i18n("Drag text/image here to post to server"))
{
    setAcceptDrops(true);
    setHasConfigurationInterface(true);
    m_displayEdit = new Plasma::Label(this);
    m_displayEdit->setText(m_text);
    m_displayEdit->setAcceptDrops(false);
    connect(m_displayEdit, SIGNAL(linkActivated(QString)),
            this, SLOT(openLink(QString)));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    layout->addItem(m_displayEdit);
    resize(200,200);
}

Pastebin::~Pastebin()
{
    delete m_displayEdit;
    delete m_text_server;
    delete m_image_server;
}

void Pastebin::setImageServer(int backend)
{
    if (m_image_server)
        delete m_image_server;

    switch(backend) {

    case Pastebin::IMAGEBINCA:
        m_image_server = static_cast<ImagebinCAServer*>(new ImagebinCAServer());
        break;
    }

    m_image_backend = backend;
    connect(m_image_server, SIGNAL(postFinished(QString)),
            this, SLOT(showResults(QString)));
}

void Pastebin::setTextServer(int backend)
{
    if (m_text_server)
        delete m_text_server;

    switch(backend) {

    case Pastebin::PASTEBINCA:
        m_text_server = static_cast<PastebinCAServer*>(new PastebinCAServer());
        break;

    case Pastebin::PASTEBINCOM:
        m_text_server = static_cast<PastebinCOMServer*>(new PastebinCOMServer());
        break;
    }

    m_text_backend = backend;
    connect(m_text_server, SIGNAL(postFinished(QString)),
            this, SLOT(showResults(QString)));
}

void Pastebin::init()
{
    KConfigGroup cg = config();
    int textBackend = cg.readEntry("TextBackend", "0").toInt();
    int imageBackend = cg.readEntry("ImageBackend", "0").toInt();
    setTextServer(textBackend);
    setImageServer(imageBackend);
}

void Pastebin::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget;
    ui.setupUi(widget);
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, parent->windowTitle(), Applet::icon());
    ui.textServer->setCurrentIndex(m_text_backend);
    ui.imageServer->setCurrentIndex(m_image_backend);
}

void Pastebin::configAccepted()
{
    KConfigGroup cg = config();
    int textBackend = ui.textServer->currentIndex();
    int imageBackend = ui.imageServer->currentIndex();
    cg.writeEntry("TextBackend", textBackend);
    cg.writeEntry("ImageBackend", imageBackend);
    setTextServer(textBackend);
    setImageServer(imageBackend);
    emit configNeedsSaving();
}

void Pastebin::showResults(const QString &url)
{
    setBusy(false);
    m_text = i18n("Successfully posted to: <a href=\"%1\">%2</a><p>"
                  "Drag text/image here to post to server", url, url);
    m_displayEdit->setText(m_text);
    QApplication::clipboard()->setText(url);
}

void Pastebin::openLink(const QString &link)
{
    KToolInvocation::invokeBrowser(link);
}

void Pastebin::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
}

void Pastebin::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}

void Pastebin::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->objectName() != QString("Pastebin-applet")) {
        bool image = false;

        m_text = event->mimeData()->text();
        m_displayEdit->setText("");
        setBusy(true);

        QUrl test_path(event->mimeData()->text());
        if (QFile::exists(test_path.path())) {
            KMimeType::Ptr type = KMimeType::findByPath(test_path.path());

            if (type->name().indexOf("image/") != -1) {
                image = true;
            }
        }

        if (!image) {
            m_text_server->post(m_text);
        } else {
            m_image_server->post(test_path.path());
        }

        event->acceptProposedAction();
    }
}

void Pastebin::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QMimeData *data = new QMimeData;
    data->setText(m_text);
    data->setObjectName("Pastebin-applet");

    QDrag *drag = new QDrag(event->widget());
    drag->setMimeData(data);
    drag->start();
}

#include "pastebin.moc"

