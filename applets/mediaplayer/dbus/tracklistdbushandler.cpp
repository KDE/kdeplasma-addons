/******************************************************************************
 * Copyright (C) 2009 Marco Martin <notmart@gmail.com>                        *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#include "tracklistdbushandler.h"
#include "tracklistadaptor.h"

#include <QList>

#include <kdebug.h>

#include <Phonon/MediaObject>
#include <Phonon/MediaSource>

TrackListDBusHandler::TrackListDBusHandler(QObject *parent, Phonon::MediaObject *media)
    : QObject(parent),
      m_currentTrack(0),
      m_random(false),
      m_loop(false)
{
    m_media = media;
    setObjectName("TrackListDBusHandler");
    new TrackListAdaptor( this );
    QDBusConnection::sessionBus().registerObject("/TrackList", this);
    connect (m_media, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));
}

int TrackListDBusHandler::AddTrack( const QString& url, bool playImmediately )
{
    Phonon::MediaSource source(url);
    if (source.type() != Phonon::MediaSource::Invalid) {
        m_tracks.append(source);
        if (playImmediately) {
            m_media->setCurrentSource(source);
            m_media->play();
        }
        emit TrackListChange(m_tracks.size());
        return 0;
    } else {
        return -1;
    }
}

void TrackListDBusHandler::DelTrack(int index)
{
    if (index < m_tracks.size() ) {
        m_tracks.removeAt(index);
        emit TrackListChange(m_tracks.size());
    }
}

int TrackListDBusHandler::GetCurrentTrack()
{
    return m_currentTrack;
}

int TrackListDBusHandler::GetLength()
{
    return m_tracks.size();
}

QVariantMap TrackListDBusHandler::GetMetadata(int position)
{
    QVariantMap ret;
    if (position < 0 || position > m_tracks.size()-1) {
        return ret;
    }

    //FIXME: ugly and slow
    Phonon::MediaObject mediaObject;
    mediaObject.setCurrentSource(m_tracks[position]);

    QMultiMap<QString, QString> stringMap = mediaObject.metaData();
    QMultiMap<QString, QString>::const_iterator i = stringMap.constBegin();

    while (i != stringMap.constEnd()) {
        bool number = false;
        int value = i.value().toInt(&number);

        //tracknumber always string, according to MPRIS spec
        if (number && (i.key().toLower() != "tracknumber")) {
            ret[i.key().toLower()] = value;
        } else {
            ret[i.key().toLower()] = QVariant(i.value());
        }
        ++i;
    }

    ret["time"] = mediaObject.totalTime()/1000;

    ret["location"] = mediaObject.currentSource().url().toString();
    return ret;
}

void TrackListDBusHandler::SetLoop(bool enable)
{
    m_loop = enable;
}

void TrackListDBusHandler::SetRandom( bool enable )
{
    m_random = enable;

    //sucky randomness, but we don't care the playlist is really cryptographically secure..
    if (m_random) {
        qsrand(QDateTime::currentDateTime().toTime_t());
    }
}


Phonon::MediaSource TrackListDBusHandler::nextTrack()
{
    if (m_tracks.size() < 2) {
        return Phonon::MediaSource();
    }

    if (m_random) {
        m_currentTrack = qrand() % (m_tracks.size() - 1);
    } else {
        m_currentTrack = (m_currentTrack + 1) % (m_tracks.size() - 1);
    }

    Phonon::MediaSource source = m_tracks[m_currentTrack];

    if (!m_loop) {
        m_tracks.removeAt(m_currentTrack);
    } else {
        emit TrackListChange(m_tracks.size());
    }

    return source;
}

void TrackListDBusHandler::aboutToFinish()
{
    m_media->enqueue(nextTrack());
}

#include "tracklistdbushandler.moc"
