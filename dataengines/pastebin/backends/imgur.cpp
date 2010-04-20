/***************************************************************************
 *   Copyright (C) 2010 by Nikhil Marathe <nsm.nikhil@gmail.com>           *
 *                         Vardhman Jain <vardhman@gmail.com>              *
 *                         Gilles Caulier <caulier.gilles@gmail.com>       *
 *                         Michał Ziąbkowski <mziab@o2.pl>             *
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

#include "imgur.h"

#include <QFile>
#include <QString>

#include <kmimetype.h>
#include <kurl.h>
#include <krandom.h>
#include <kapplication.h>
#include <KDebug>

ImgurServer::ImgurServer(const QString &server)
{
    if (server.isEmpty()) {
        m_server = IMGUR_SERVER;
    } else {
        m_server = server;
    }

    m_boundary  = "----------";
    m_boundary += KRandom::randomString(42 + 13).toAscii();
}

ImgurServer::~ImgurServer()
{
}

void ImgurServer::readKIOData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);

    if (data.length() == 0) {
        return;
    }

    m_data.append(data);
}

void ImgurServer::finished(KJob *job)
{
    Q_UNUSED(job);

    if (m_data.length() == 0) {
        emit postError();
        return;
    }

    QString reply(m_data);
    QRegExp re(".*<imgur_page>([^<]+)</imgur_page>.*");

    if( !re.exactMatch(reply) ) {
        emit postError();
        return;
    }

    QString pasteUrl = re.cap(1).replace("&amp;", "&");
    emit postFinished(pasteUrl);
}

// taken from flickr KIPI Plugin
void ImgurServer::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";

    m_buffer.append(str);
}

// taken from flickr KIPI Plugin
bool ImgurServer::addPair(const QString& name, const QString& value)
{
     QByteArray str;

     str += "--";
     str += m_boundary;
     str += "\r\n";
     str += "Content-Disposition: form-data; name=\"";
     str += name.toAscii();
     str += "\"";
     str += "\r\n\r\n";
     str += value.toUtf8();
     str += "\r\n";

     m_buffer.append(str);
     return true;
}

// taken from flickr KIPI Plugin
bool ImgurServer::addFile(const QString& name,const QString& path)
{
    KMimeType::Ptr ptr = KMimeType::findByUrl(path);
    QString mime = ptr->name();
    if (mime.isEmpty()) {
        // if we ourselves can't determine the mime of the local file,
        // very unlikely the remote site will be able to identify it
        return false;
    }

    QFile imageFile(path);
    if (!imageFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray imageData = imageFile.readAll();
    imageFile.close();

    QByteArray str;

    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"";
    str += name.toAscii();
    str += "\"; ";
    str += "filename=\"";
    str += QFile::encodeName(KUrl(path).fileName()).replace(".tmp", ".jpg");
    str += "\"";
    str += "\r\n";
    str += "Content-Type: ";
    str +=  mime.toAscii();
    str += "\r\n\r\n";

    m_buffer.append(str);

    int oldSize = m_buffer.size();
    m_buffer.resize(oldSize + imageData.size() + 2);
    memcpy(m_buffer.data() + oldSize, imageData.data(), imageData.size());
    m_buffer[m_buffer.size()-2] = '\r';
    m_buffer[m_buffer.size()-1] = '\n';

    return true;
}

void ImgurServer::post(const QString& content)
{
    m_data.clear();
    KUrl url(QString("%1").arg(m_server));
    addFile( "image", content );

    // key associated with plasma-devel@kde.org
    // thanks to Alan Schaaf of Imgur ( alan@imgur.com )
    addPair( "key", "d0757bc2e94a0d4652f28079a0be9379" );

    KIO::TransferJob *tf = KIO::http_post(url, m_buffer, KIO::HideProgressInfo);

    tf->addMetaData("content-type","Content-Type: multipart/form-data; boundary=" + m_boundary);

    connect(tf, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(readKIOData(KIO::Job*, const QByteArray&)));
    connect(tf, SIGNAL(result(KJob *)), this, SLOT(finished(KJob *)));
}
