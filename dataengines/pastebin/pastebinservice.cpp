/*
 *   Copyright 2009 Artur Duque de Souza <asouza@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "pastebinservice.h"



PastebinService::PastebinService(PastebinEngine *engine)
    : Plasma::Service(), m_textServer(0), m_imageServer(0)
{
    m_engine = engine;
    setName("pastebin");
}

Plasma::ServiceJob *PastebinService::createJob(const QString &operation,
                                               QMap<QString, QVariant> &parameters)
{
    QString server = parameters["server"].toString();

    // it's a text
    if (operation == "text") {
        delete m_textServer;

        int backend = parameters["backend"].toInt();
        switch(backend) {
            case PastebinService::PASTEBINCA:
                m_textServer = new PastebinCAServer(server);
                break;

            case PastebinService::PASTEBINCOM:
                m_textServer = new PastebinCOMServer(server);
                break;
        }

        connect(m_textServer, SIGNAL(postFinished(QString)),
                this, SLOT(showResults(QString)));
        connect(m_textServer, SIGNAL(postError()),
                this, SLOT(showErrors()));

        postText(parameters);
    } else {
        delete m_imageServer;

        int backend = parameters["backend"].toInt();
        switch(backend) {
            case PastebinService::IMAGEBINCA:
                m_imageServer = new ImagebinCAServer(server);
                break;

            case PastebinService::IMAGESHACK:
                m_imageServer = new ImageshackServer(server);
                break;

            case PastebinService::SIMPLESTIMAGEHOSTING:
                m_imageServer = new SimplestImageHostingServer(server);
                break;
        }

        connect(m_imageServer, SIGNAL(postFinished(QString)),
                this, SLOT(showResults(QString)));
        connect(m_imageServer, SIGNAL(postError()),
                this, SLOT(showErrors()));

        postImage(parameters);
    }

    return 0;
}

void PastebinService::postText(QMap<QString, QVariant> &parameters)
{
    QString text = parameters["fileName"].toString();

    KUrl testPath(text);
    bool validPath = QFile::exists(testPath.toLocalFile());

    // if it's a valid path, then take the file's content
    // otherwise just post the content's of fileName
    if (validPath) {
        QFile file(testPath.toLocalFile());
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        text = in.readAll();
    } else if (testPath.scheme().toLower() == QString("http")) {
        // lets use tiny url ;)
        QString tinyUrl = QString("http://tinyurl.com/api-create.php?url=%1").arg(testPath.prettyUrl());
        manager = new QNetworkAccessManager(this);
        connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(processTinyUrl(QNetworkReply*)));
        manager->get(QNetworkRequest(tinyUrl));
        return;
    }

    // the post
    m_textServer->post(text);
}

void PastebinService::postImage(QMap<QString, QVariant> &parameters)
{
    // it's an image
    QString url = parameters["server"].toString();
    QString fileName = parameters["fileName"].toString();
    //    bool privacy = parameters["privacy"].toBool();

    KUrl testPath(fileName);
    bool validPath = QFile::exists(testPath.toLocalFile());

    if (validPath) {
        m_imageServer->post(testPath.toLocalFile());
    } else {
        showErrors(i18n("Could not post image."));
    }
}

void PastebinService::processTinyUrl(QNetworkReply *reply)
{
    QByteArray dataRaw(reply->readAll());
    QString data(dataRaw);

    if (data.isEmpty() || !data.contains("tinyurl.com")) {
        showErrors(i18n("Problems while posting URL."));
        return;
    }

    // its ok!
    showResults(data);
}

void PastebinService::showResults(const QString &url)
{
    m_engine->setData("result", url);
}

void PastebinService::showErrors(const QString &message)
{
    QString errorMsg = message;

    if (errorMsg.isEmpty()) {
        errorMsg = i18n("Unknown Error");
    }

    m_engine->setData("error", errorMsg);
}
