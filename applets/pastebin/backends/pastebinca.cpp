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

#include "pastebinca.h"

#include <KDebug>
#include <QUrl>

PastebinCAServer::PastebinCAServer(const KConfigGroup& config)
    : PastebinServer(),
      m_apikey("AnGWmLlrReJCCJOUMOMpHViBTFkFZxre")
{
    m_server = config.readEntry("pastebinca", "http://pastebin.ca");
}

PastebinCAServer::~PastebinCAServer()
{
}

void PastebinCAServer::finished(KJob *job)
{
    Q_UNUSED(job);

    if (_data.length() == 0 || _data.contains("FAIL")) {
        emit postError();
        return;
    }

    QString url(_data);
    url.remove("SUCCESS:");
    url.prepend(QString("%1/").arg(m_server));
    emit postFinished(url);
}

void PastebinCAServer::readKIOData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);

    if (data.length() == 0) {
        return;
    }

    _data.append(data);
}

void PastebinCAServer::post(const QString& content)
{
    QByteArray bytearray = "content=";
    bytearray.append(QUrl::toPercentEncoding(content,"/"));
    bytearray.append("&api=");
    bytearray.append(m_apikey);
    bytearray.append("&description=&type=1&expiry=1%20day&name=");

    QString url("/quiet-paste.php");
    url.prepend(m_server);

    _data.clear();

    KIO::TransferJob *tf = KIO::http_post(KUrl(url),bytearray,KIO::HideProgressInfo);
    tf->addMetaData("content-type","Content-Type: application/x-www-form-urlencoded");
    connect(tf, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(readKIOData(KIO::Job*, const QByteArray&)));

    connect(tf, SIGNAL(result(KJob *)), this, SLOT(finished(KJob *)));
}
