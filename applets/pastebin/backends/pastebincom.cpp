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

PastebinCOMServer::PastebinCOMServer(const KConfigGroup& config)
    : PastebinServer()
{
    m_server = config.readEntry("pastebincom", "http://pastebin.com");
}

PastebinCOMServer::~PastebinCOMServer()
{
}

void PastebinCOMServer::result(KIO::Job *job, const KUrl &url)
{
    Q_UNUSED(job);

    if (url.url().contains("pastebin.php")) {
        // we had an error
        emit postError();
        return;
    }

    emit postFinished(url.url());
}

void PastebinCOMServer::post(const QString& content)
{
    QByteArray bytearray = "code2=";
    bytearray.append(QUrl::toPercentEncoding(content,"/"));
    bytearray.append("&parent_pid=&format=text&expiry=d&poster=&paste=Send");

    QString url("/pastebin.php");
    url.prepend(m_server);

    KIO::TransferJob *tf = KIO::http_post(KUrl(url),
                                          bytearray,KIO::HideProgressInfo);

    tf->addMetaData("content-type","Content-Type: application/x-www-form-urlencoded");
    connect(tf, SIGNAL(redirection(KIO::Job*, const KUrl&)),
            this, SLOT(result(KIO::Job*, const KUrl&)));

}
