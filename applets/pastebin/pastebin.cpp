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

Pastebin::Pastebin(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args), m_textServer(0),
      m_imageServer(0), m_textBackend(0), m_imageBackend(0),
      m_graphicsWidget(0)
{
    setAcceptDrops(true);
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPopupIcon("edit-paste");

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showErrors()));
}

Pastebin::~Pastebin()
{
    delete m_displayEdit;
    delete m_textServer;
    delete m_imageServer;
    delete m_graphicsWidget;
    delete timer;
}

void Pastebin::setImageServer(int backend)
{
    if (m_imageServer)
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

void Pastebin::init()
{
    KConfigGroup cg = config();
    int textBackend = cg.readEntry("TextBackend", "0").toInt();
    int imageBackend = cg.readEntry("ImageBackend", "0").toInt();
    setTextServer(textBackend);
    setImageServer(imageBackend);
}

QGraphicsWidget *Pastebin::graphicsWidget()
{
    if (m_graphicsWidget) {
        return m_graphicsWidget;
    }

    m_resultsLabel = new DraggableLabel(this);
    m_resultsLabel->setVisible(false);
    m_displayEdit = new Plasma::Label(this);
    m_displayEdit->setText(i18n("Drop text or images on me to upload them to Pastebin."));
    m_displayEdit->setAcceptDrops(false);
    m_displayEdit->nativeWidget()->setTextInteractionFlags(Qt::NoTextInteraction);
    registerAsDragHandle(m_displayEdit);
    connect(m_resultsLabel, SIGNAL(linkActivated(QString)), this, SLOT(openLink(QString)));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    layout->addItem(m_displayEdit);
    layout->addItem(m_resultsLabel);

    m_graphicsWidget = new QGraphicsWidget(this);
    m_graphicsWidget->setLayout(layout);
    m_graphicsWidget->setPreferredSize(200, 200);
    return m_graphicsWidget;
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

    QString pastebincaURL = uiServers.pastebinca->text();
    QString pastebincomURL = uiServers.pastebincom->text();
    QString imagebincaURL = uiServers.imagebinca->text();
    QString imageshackURL = uiServers.imageshack->text();

    cg.writeEntry("TextBackend", textBackend);
    cg.writeEntry("ImageBackend", imageBackend);

    cg.writeEntry("pastebinca", pastebincaURL);
    cg.writeEntry("pastebincom", pastebincomURL);
    cg.writeEntry("imagebinca", imagebincaURL);
    cg.writeEntry("imageshack", imageshackURL);

    setTextServer(textBackend);
    setImageServer(imageBackend);
    emit configNeedsSaving();
}

void Pastebin::showResults(const QString &url)
{
    setBusy(false);
    timer->stop();

    m_displayEdit->setVisible(true);
    m_resultsLabel->setVisible(true);
    m_resultsLabel->m_url = url;
    m_resultsLabel->setText(i18n("Successfully uploaded to: <a href=\"%1\">%2</a><p>", url, url));
    QApplication::clipboard()->setText(url);
}

void Pastebin::showErrors()
{
    setBusy(false);
    m_displayEdit->setVisible(true);
    m_resultsLabel->setVisible(true);
    m_resultsLabel->setText(i18n("Error during uploading! Please try again."));
    m_resultsLabel->m_url = KUrl();
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

        QString text = event->mimeData()->text();
        setBusy(true);
        timer->start(20000);

        m_displayEdit->setVisible(false);
        m_resultsLabel->setVisible(false);
        QUrl testPath(text);
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
            if (validPath) {
                QFile file(testPath.path());
                file.open(QIODevice::ReadOnly);
                QTextStream in(&file);
                text = in.readAll();
            }

            // upload text
            m_textServer->post(text);
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
                    timer->stop();
                }

            }
        }

        event->acceptProposedAction();
    }
}

DraggableLabel::DraggableLabel(QGraphicsWidget *parent)
    : Plasma::Label(parent)
{
}

void DraggableLabel::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_url.isEmpty() || event->button() != Qt::LeftButton) {
        Plasma::Label::mousePressEvent(event);
    } else {
        event->accept();
    }
}

void DraggableLabel::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    int distance = (event->buttonDownPos(Qt::LeftButton) - event->pos().toPoint()).toPoint().manhattanLength();
    if (distance > KGlobalSettings::dndEventDelay()) {
        QMimeData *data = new QMimeData;
        data->setText(m_url.prettyUrl());
        data->setObjectName("Pastebin-applet");

        QDrag *drag = new QDrag(event->widget());
        drag->setMimeData(data);
        drag->start();
    }
}


#include "pastebin.moc"

