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

#ifndef PLAYERDBUSHANDLER_HEADER
#define PLAYERDBUSHANDLER_HEADER

#include <QObject>
#include <QVariantMap>
#include <QDBusArgument>

#include <phonon/mediaobject.h>

namespace Phonon
{
    class MediaObject;
    class AudioOutput;
}

struct DBusStatus
{
    int Play; //Playing = 0, Paused = 1, Stopped = 2
    int Random; //Linearly = 0, Randomly = 1
    int Repeat; //Go_To_Next = 0, Repeat_Current = 1
    int RepeatPlaylist; //Stop_When_Finished = 0, Never_Give_Up_Playing = 1
};

Q_DECLARE_METATYPE(DBusStatus)

// Marshall the DBusStatus data into a D-BUS argument
QDBusArgument &operator << ( QDBusArgument &argument, const DBusStatus &status );
// Retrieve the DBusStatus data from the D-BUS argument
const QDBusArgument &operator >> ( const QDBusArgument &argument, DBusStatus &status );

enum DbusCaps {
    NONE                  = 0,
    //CAN_GO_NEXT           = 1 << 0, not for now
    //CAN_GO_PREV           = 1 << 1,
    CAN_PAUSE             = 1 << 2,
    CAN_PLAY              = 1 << 3,
    CAN_SEEK              = 1 << 4,
    CAN_PROVIDE_METADATA  = 1 << 5,
    CAN_HAS_TRACKLIST     = 1 << 6
};

class PlayerDBusHandler : public QObject
{
    Q_OBJECT

public:

    PlayerDBusHandler(QObject *parent,
                      Phonon::MediaObject *mediaObject,
                      Phonon::AudioOutput *audioOutput);
    ~PlayerDBusHandler();


public Q_SLOTS:
    void PlayPause();
    void Play();
    void Pause();
    void Stop();
    DBusStatus GetStatus();
    int GetCaps();
    QVariantMap GetMetadata();
    void PositionSet(int progress);
    qint64 PositionGet() const;
    void VolumeSet(int value);
    int VolumeGet() const;


protected Q_SLOTS:
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void trackChanged();
    void seekableChanged(bool isSeekable);

Q_SIGNALS:
    void StatusChange(DBusStatus status);
    void CapsChange(int caps);
    void TrackChange(QVariantMap metadata);

private:

    Phonon::MediaObject *m_mediaObject;
    Phonon::AudioOutput *m_audioOutput;
};

#endif
