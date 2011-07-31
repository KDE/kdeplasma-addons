/***************************************************************************
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>                    *
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

#include "playerdbushandler.h"
#include "playeradaptor.h"

#include <QDBusConnection>

#include <phonon/audiooutput.h>

// Marshall the DBusStatus data into a D-BUS argument
QDBusArgument &operator<<(QDBusArgument &argument, const DBusStatus &status)
{
    argument.beginStructure();
    argument << status.Play;
    argument << status.Random;
    argument << status.Repeat;
    argument << status.RepeatPlaylist;
    argument.endStructure();
    return argument;
}

// Retrieve the DBusStatus data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusStatus &status)
{
    argument.beginStructure();
    argument >> status.Play;
    argument >> status.Random;
    argument >> status.Repeat;
    argument >> status.RepeatPlaylist;
    argument.endStructure();
    return argument;
}

PlayerDBusHandler::PlayerDBusHandler(QObject *parent, Phonon::MediaObject *media, Phonon::AudioOutput *audioOutput)
    : QObject(parent),
      m_mediaObject(media),
      m_audioOutput(audioOutput)
{
    qDBusRegisterMetaType<DBusStatus>();

    new PlayerAdaptor(this);
    setObjectName("PlayerDBusHandler");
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/Player", this);

    connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));

    connect(m_mediaObject, SIGNAL(seekableChanged(bool)), this, SLOT(seekableChanged(bool)));

    connect(m_mediaObject, SIGNAL(metaDataChanged()), this, SLOT(trackChanged()));
}


PlayerDBusHandler::~PlayerDBusHandler()
{
}



void PlayerDBusHandler::PlayPause()
{
    if (m_mediaObject->state() == Phonon::PlayingState) {
        m_mediaObject->pause();
    } else {
        m_mediaObject->play();
    }
}

void PlayerDBusHandler::Play()
{
    if (m_mediaObject->state() != Phonon::PlayingState) {
        m_mediaObject->play();
    }
}

void PlayerDBusHandler::Pause()
{
    if (m_mediaObject->state() == Phonon::PlayingState) {
        m_mediaObject->pause();
    }
}

void PlayerDBusHandler::Stop()
{
    m_mediaObject->stop();
}

DBusStatus PlayerDBusHandler::GetStatus()
{
    DBusStatus status;

    //0 = Playing, 1 = Paused, 2 = Stopped.
    switch (m_mediaObject->state()) {
    case Phonon::PlayingState:
        status.Play = 0;
        break;
    case Phonon::PausedState:
        status.Play = 1;
        break;
    case Phonon::StoppedState:
    default:
        status.Play = 2;
    }

    status.Random = 0;
    status.Repeat = 0;
    status.RepeatPlaylist = 0;

    return status;
}

int PlayerDBusHandler::GetCaps()
{
    int caps = NONE;
    if (m_mediaObject->state() == Phonon::PlayingState) {
        caps |= CAN_PAUSE;
    }
    if (m_mediaObject->state() == Phonon::PausedState) {
        caps |= CAN_PLAY;
    }

    if (m_mediaObject->isSeekable()) {
        caps |= CAN_SEEK;
    }

    caps |= CAN_HAS_TRACKLIST;
    caps |= CAN_PROVIDE_METADATA; //though it might be empty...
    return caps;
}

//see http://wiki.xmms2.xmms.se/index.php/MPRIS_Metadata
//derived from DragonPlayer
QVariantMap PlayerDBusHandler::GetMetadata()
{
    QVariantMap ret;
    QMultiMap<QString, QString> stringMap = m_mediaObject->metaData();
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

    ret["time"] = m_mediaObject->totalTime()/1000;

    ret["location"] = m_mediaObject->currentSource().url().toString();
    return ret;
}

void PlayerDBusHandler::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState)
    Q_UNUSED(newState)

    emit StatusChange(GetStatus());
    emit CapsChange(GetCaps());
}

void PlayerDBusHandler::seekableChanged(bool isSeekable)
{
    Q_UNUSED(isSeekable)

    emit CapsChange(GetCaps());
}

void PlayerDBusHandler::trackChanged()
{
    emit TrackChange(GetMetadata());
}

void PlayerDBusHandler::PositionSet(int progress)
{
    m_mediaObject->seek(progress);
}

qint64 PlayerDBusHandler::PositionGet() const
{
    return m_mediaObject->currentTime();
}

void PlayerDBusHandler::VolumeSet(int value)
{
     m_audioOutput->setVolume(qreal(value)/100.0);
}

int PlayerDBusHandler::VolumeGet() const
{
     return m_audioOutput->volume()*100;
}

#include "playerdbushandler.moc"
