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

#include <QtCore/QFile>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>


PastebinService::PastebinService(PastebinEngine *engine)
    : Plasma::Service(engine)
{
    setName("pastebin");
}

Plasma::ServiceJob *PastebinService::createJob(const QString &operation,
                                               QMap<QString, QVariant> &parameters)
{
    return new PastebinJob(destination(), operation, parameters, this);
}

PastebinJob::PastebinJob(const QString &destination, const QString &operation,
                         QMap<QString, QVariant> &parameters, QObject *parent)
    : Plasma::ServiceJob(destination, operation, parameters, parent),
      m_server(0)
{
    const QString server = parameters["server"].toString();
    const int backend = parameters["backend"].toInt();

    if (operation == "image") {
        switch (backend) {
            case PastebinService::IMAGEBINCA:
                m_server = new ImagebinCAServer(server);
                break;

            case PastebinService::IMAGESHACK:
                m_server = new ImageshackServer(server);
                break;

            case PastebinService::SIMPLESTIMAGEHOSTING:
                m_server = new SimplestImageHostingServer(server);
                break;

            case PastebinService::IMGUR:
                m_server = new ImgurServer(server);
                break;

            default:
                break;
        }
    } else {
        switch (backend) {
            case PastebinService::PASTEBINCA:
                m_server = new PastebinCAServer(server);
                break;

            case PastebinService::PASTEBINCOM:
                m_server = new PastebinCOMServer(server);
                break;

            default:
                break;
        }
    }

    if (m_server) {
        connect(m_server, SIGNAL(postFinished(QString)), this, SLOT(showResults(QString)));
        connect(m_server, SIGNAL(postError()), this, SLOT(showErrors()));
        connect(m_server, SIGNAL(postError(QString)), this, SLOT(showErrors(QString)));
    }
}

void PastebinJob::start()
{
    if (!m_server) {
        showErrors(i18n("Invalid service requested."));
    } else if (operationName() == "image") {
        postImage();
    } else {
        postText();
    }
}

void PastebinJob::postText()
{
    const bool fromFile = operationName() == "text";
    const QString text = (fromFile ? parameters()["fileName"] : parameters()["text"]).toString();
    kDebug() << "posting:" << fromFile << text;

    if (text.isEmpty()) {
        showErrors(i18n("Content posted was empty."));
    } else if (fromFile) {
        KUrl testPath(text);
        bool validPath = QFile::exists(testPath.toLocalFile());

        // if it's a valid path, then take the file's content
        // otherwise just post the content's of fileName
        if (validPath) {
            QFile file(testPath.toLocalFile());
            file.open(QIODevice::ReadOnly);
            QTextStream in(&file);
            QString text = in.readAll();
            m_server->post(text);
        } else if (testPath.scheme().toLower() == QString("http")) {
            // lets use tiny url ;)
            QString tinyUrl = QString("http://tinyurl.com/api-create.php?url=%1").arg(testPath.prettyUrl());
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)),
                    this, SLOT(processTinyUrl(QNetworkReply*)));
            manager->get(QNetworkRequest(tinyUrl));
        } else {
            showErrors(i18n("Invalid URL"));
        }
    } else {
        m_server->post(text);
    }
}

void PastebinJob::postImage()
{
    // it's an image
    QString url = parameters()["server"].toString();
    QString fileName = parameters()["fileName"].toString();
    //    bool privacy = parameters["privacy"].toBool();

    KUrl testPath(fileName);
    bool validPath = QFile::exists(testPath.toLocalFile());

    if (validPath) {
        m_server->post(testPath.toLocalFile());
    } else {
        showErrors(i18n("Could not post image."));
    }
}

void PastebinJob::processTinyUrl(QNetworkReply *reply)
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

void PastebinJob::showResults(const QString &url)
{
    setResult(url);
}

void PastebinJob::showErrors(const QString &message)
{
    QString errorMsg = message;
    if (errorMsg.isEmpty()) {
        errorMsg = i18n("Unknown Error");
    }

    setError(1);
    setErrorText(message);
    emitResult();
}

#include "pastebinservice.moc"


