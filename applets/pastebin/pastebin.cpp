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
#include <QBuffer>

#include <KDebug>
#include <KLocale>
#include <KConfigDialog>
#include <KToolInvocation>
#include <kmimetype.h>
#include <ktemporaryfile.h>

#include <kio/global.h>
#include <kio/job.h>

#include <plasma/widgets/flashinglabel.h>
#include <plasma/widgets/label.h>

using namespace Plasma;

Pastebin::Pastebin(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_textServer(0), m_imageServer(0),
      m_textBackend(0), m_imageBackend(0),
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
    setBackgroundHints(TranslucentBackground);
    resize(200,200);
}

Pastebin::~Pastebin()
{
    delete m_displayEdit;
    delete m_textServer;
    delete m_imageServer;
}

void Pastebin::setImageServer(int backend)
{
    if (m_imageServer)
        delete m_imageServer;

    switch(backend) {

    case Pastebin::IMAGEBINCA:
        m_imageServer = static_cast<ImagebinCAServer*>(new ImagebinCAServer());
        break;

    case Pastebin::IMAGESHACK:
        m_imageServer = static_cast<ImageshackServer*>(new ImageshackServer());
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
    if (m_textServer)
        delete m_textServer;

    switch(backend) {

    case Pastebin::PASTEBINCA:
        m_textServer = static_cast<PastebinCAServer*>(new PastebinCAServer());
        break;

    case Pastebin::PASTEBINCOM:
        m_textServer = static_cast<PastebinCOMServer*>(new PastebinCOMServer());
        break;
    }

    m_textBackend = backend;
    connect(m_textServer, SIGNAL(postFinished(QString)),
            this, SLOT(showResults(QString)));
    connect(m_textServer, SIGNAL(postError()),
            this, SLOT(showErrors()));
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
    parent->addPage(widget, i18n("General"), Applet::icon());
    ui.textServer->setCurrentIndex(m_textBackend);
    ui.imageServer->setCurrentIndex(m_imageBackend);
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

void Pastebin::showErrors()
{
    setBusy(false);
    m_text = i18n("Error during post. Try again."
                  "Drag text/image here to post to server");
    m_displayEdit->setText(m_text);
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
        bool validPath = false;

        m_text = event->mimeData()->text();
        m_displayEdit->setText("");
        setBusy(true);

        QUrl testPath(m_text);
        validPath = QFile::exists(testPath.path());

        if (validPath) {
            KMimeType::Ptr type = KMimeType::findByPath(testPath.path());

            if (type->name().indexOf("image/") != -1) {
                image = true;
            }
        }
        else {
            if (event->mimeData()->hasImage()) {
                image = true;
            }
        }

        if (!image) {
            // upload text
            m_textServer->post(m_text);
        } else {
            //upload image
            if (validPath) {
                m_imageServer->post(testPath.path());
            }
            else {
                KTemporaryFile tempFile;
                if (tempFile.open()) {
                    tempFile.setAutoRemove(false);

                    QDataStream stream(&tempFile);

                    QByteArray data;
                    QBuffer buffer(&data);
                    buffer.open(QIODevice::ReadWrite);

                    QImage image = qvariant_cast<QImage>(event->mimeData()->imageData());
                    image.save(&buffer, "JPEG");
                    stream.writeRawData(data, data.size());

                    QUrl t(tempFile.fileName());
                    tempFile.close();

                    m_imageServer->post(t.path());

                }
                else {
                    setBusy(false);
                }

            }
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

