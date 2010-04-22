/***************************************************************************
 *   Copyright (C) 2008 by Artur Duque de Souza <morpheuz@gmail.com>       *
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

#include "pastebincom.h"

#include <KDebug>
#include <QUrl>

PastebinCOMServer::PastebinCOMServer(const QString &server)
{
    if (server.isEmpty()) {
        m_server = PASTEBINCOM_SERVER;
    } else {
        m_server = server;
    }
}

PastebinCOMServer::~PastebinCOMServer()
{
}

void PastebinCOMServer::data(KIO::Job* job, const QByteArray &data)
{
    if (data.isEmpty()) {
        if (job->error()) {
            emit postError(job->errorString());
        } else if (m_resultingUrl.isEmpty() || m_resultingUrl.startsWith("ERROR")) {
            emit postError(m_resultingUrl);
        } else {
            emit postFinished(m_resultingUrl);
        }
    } else {
        m_resultingUrl += data;
    }
}

void PastebinCOMServer::post(const QString& content)
{
    m_resultingUrl.clear();
    QByteArray bytearray = "paste_code=";
    bytearray.append(QUrl::toPercentEncoding(content,"/"));
    bytearray.append("&paste_format=text&paste_expiry_date=1D&paste_email=");

    QString url(m_server + "/api_public.php");

    KIO::TransferJob *tf = KIO::http_post(KUrl(url), bytearray, KIO::HideProgressInfo);

    tf->addMetaData("content-type","Content-Type: application/x-www-form-urlencoded");
    connect(tf, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(data(KIO::Job*, const QByteArray&)));
}

#include "pastebincom.moc"

