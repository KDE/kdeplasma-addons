/*
 *   Copyright (C) 2008 Bruno Virlet <bvirlet@kdemail.net>
 *   Copyright (C) 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright (C) 2009 Jan G. Marker <jangerrit@weiler-marker.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "audioplayercontrolrunner.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusMessage>

#include <KDE/KMessageBox>
#include <KDE/KDebug>
#include <KDE/KIcon>
#include <KDE/KRun>
#include <KDE/KUrl>

#include "audioplayercontrolconfigkeys.h"

Q_DECLARE_METATYPE(QList<QVariantMap>)

/** The variable PLAY contains the action label for play */
static const QString PLAY(QLatin1String("play"));
/** The variable APPEND contains the action label for append */
static const QString APPEND(QLatin1String("append"));
/** The variable QUEUE contains the action label for queue */
static const QString QUEUE(QLatin1String("queue"));
/** The variable NONE says that no action is needed */
static const QString NONE(QLatin1String("none"));


AudioPlayerControlRunner::AudioPlayerControlRunner(QObject *parent, const QVariantList& args)
        : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);

    setObjectName(QLatin1String( "Audio Player Control Runner" ));
    setSpeed(AbstractRunner::SlowSpeed);

    qDBusRegisterMetaType<QList<QVariantMap> >();

    connect(this, SIGNAL(prepare()), this, SLOT(prep()));

    reloadConfiguration();
}

AudioPlayerControlRunner::~AudioPlayerControlRunner()
{
}


void AudioPlayerControlRunner::prep()
{
    m_running = false;
    m_songsInPlaylist = 0;
    m_currentTrack = -1;
    m_nextSongAvailable = false;
    m_prevSongAvailable = false;

    QDBusInterface player(QString::fromLatin1( "org.mpris.%1").arg(m_player), QLatin1String( "/TrackList" ), QLatin1String( "org.freedesktop.MediaPlayer" ));
    QDBusPendingCall call = player.asyncCall(QLatin1String( "GetLength" ));
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(songsInPlaylist(QDBusPendingCallWatcher*)));

    call = player.asyncCall(QLatin1String( "GetCurrentTrack" ));
    watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(prevNextSongAvailable(QDBusPendingCallWatcher*)));
}

void AudioPlayerControlRunner::match(Plasma::RunnerContext &context)
{
    if (context.query().length() < 3) {
        return;
    }

    const QString term = context.query();

    QList<Plasma::QueryMatch> matches;

    if (m_useCommands) {
        /* DBus paths that are used in the command executes */
        /* The data variable looks like this:
         * "/PlayerQLatin1String( " " )org.freedesktop.MediaPlayerQLatin1String( " " )PlayQLatin1String( " " )actionsQLatin1String( " " )start" args...
         * <path>    <interface>                 <method> <actions> <start player>
         * <actions> is NONE if no action is needed
         */

        QVariantList playcontrol;
        playcontrol  << QLatin1String( "/Player" ) << QLatin1String( "org.freedesktop.MediaPlayer" );

        /* The commands */

        //Play
        if (context.isValid() && m_comPlay.startsWith(term, Qt::CaseInsensitive) &&
	    (!m_running || m_songsInPlaylist)) {
            QVariantList data = playcontrol;
            data << ((currentSong() == -1) ? QLatin1String( "Next" ) : QLatin1String( "Play" )) << NONE << QLatin1String( "start" );
            matches << createMatch(this, i18n("Start playing"), i18n("Audio player control"), QLatin1String( "play" ),
                                   KIcon( QLatin1String( "media-playback-start" )), data, 1.0);
        }

        if (!context.isValid() || !m_running) {
            //The interface of the player is not availalbe, so the rest of the commands
            //is not needed
            context.addMatches(term,matches);
            return;
        }

        if (context.isValid() && m_songsInPlaylist) {
            //The playlist isn't empty
            //Next song
            if (m_comNext.startsWith(term,Qt::CaseInsensitive)
                    && m_nextSongAvailable) {
                QVariantList data = playcontrol;
                data << QLatin1String( "Next" ) << NONE << QLatin1String( "nostart" );
                matches << createMatch(this, i18n("Play next song"), i18n("Audio player control"),
                                       QLatin1String( "next" ), KIcon( QLatin1String( "media-skip-forward" )), data, 1.0);
            }

            //Previous song
            if (context.isValid() && m_comPrev.startsWith(term,Qt::CaseInsensitive)
                    && m_prevSongAvailable) {
                QVariantList data = playcontrol;
                data << QLatin1String( "Prev" ) << NONE << QLatin1String( "nostart" );
                matches << createMatch(this, i18n("Play previous song"), i18n("Audio player control") ,
                                       QLatin1String( "previous" ), KIcon( QLatin1String( "media-skip-backward" )), data, 1.0);
            }
        }//--- if (m_songsInPlaylist)

        //Pause
        if (context.isValid() && m_comPause.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << QLatin1String( "Pause" ) << NONE << QLatin1String( "nostart" );
            matches << createMatch(this, i18n("Pause playing"), i18n("Audio player control"),
                                   QLatin1String( "pause" ), KIcon( QLatin1String( "media-playback-pause" )), data, 1.0);
        }

        //Stop
        if (context.isValid() && m_comStop.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << QLatin1String( "Stop" ) << NONE << QLatin1String( "nostart" );
            matches << createMatch(this, i18n("Stop playing"), i18n("Audio player control"),
                                   QLatin1String( "stop" ), KIcon( QLatin1String( "media-playback-stop" )), data, 1.0);
        }

        //Increase
        if (context.isValid() && m_comIncrease.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << QLatin1String( "VolumeUp" ) << NONE << QLatin1String( "nostart" ) << m_increaseBy;
            matches << createMatch(this, i18n("Increase volume by %1" , m_increaseBy),
                                   QLatin1String( "volumeup" ), i18n("Audio player control"), KIcon(QLatin1String( "audio-volume-high" )), data, 1.0);
        } else if (context.isValid() && equals(term, QRegExp( m_comIncrease + QLatin1String( " \\d{1,2}0{0,1}" ) ) ) ) {
            int volumeChange = getNumber(term, ' ' );
            QVariantList data = playcontrol;
            data << QLatin1String( "VolumeUp" ) << NONE << QLatin1String( "nostart" ) << volumeChange;
            matches << createMatch(this, i18n("Increase volume by %1" , volumeChange),
                                   QLatin1String( "volumeup" ), i18n("Audio player control"), KIcon(QLatin1String( "audio-volume-high" )), data, 1.0);
        }

        //Decrease
        if (context.isValid() && m_comDecrease.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << QLatin1String( "VolumeDown" ) << NONE << QLatin1String( "nostart" ) << m_decreaseBy;
            matches << createMatch(this, i18n("Reduce volume by %1", m_decreaseBy),
                                   QLatin1String( "volumedown" ), i18n("Audio player control"), KIcon(QLatin1String( "audio-volume-low" )), data, 1.0);
        } else if (context.isValid() && equals(term, QRegExp( m_comDecrease + QLatin1String( " \\d{1,2}0{0,1}" ) ) ) ) {
            int volumeChange = getNumber(term, ' ');
            QVariantList data = playcontrol;
            data << QLatin1String( "VolumeDown" ) << NONE << QLatin1String( "nostart" ) << volumeChange;
            matches << createMatch(this, i18n("Reduce volume by %1", volumeChange),
                                   QLatin1String( "volumedown" ), i18n("Audio player control"), KIcon(QLatin1String( "audio-volume-low" )), data, 1.0);
        }

        //Set volume to
        if (context.isValid() && equals(term, QRegExp( m_comVolume + QLatin1String( " \\d{1,2}0{0,1}" ) ) ) ) {
            QVariantList data = playcontrol;
            int newVolume = getNumber(term , ' ');
            data << QLatin1String( "VolumeSet" ) << NONE << QLatin1String( "nostart" ) << newVolume;
            matches << createMatch(this, i18n("Set volume to %1%" , newVolume),
                                   QLatin1String( "volume" ), i18n("Audio player control"), KIcon(QLatin1String( "audio-volume-medium" )), data, 1.0);
        }

        //Mute
        if (context.isValid() && m_comMute.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << QLatin1String( "Mute" ) << NONE << QLatin1String( "nostart" );
            matches << createMatch(this, i18n("Mute"), i18n("Audio player control"),
                                   QLatin1String( "mute" ), KIcon( QLatin1String( "audio-volume-muted" )), data, 1.0);
        }

        //Quit player
        if (context.isValid() && m_comQuit.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data;
            data  << QLatin1String( "/" ) << QLatin1String( "org.freedesktop.MediaPlayer" ) << QLatin1String( "Quit" ) << NONE
            << QLatin1String( "nostart" );
            matches << createMatch(this, i18n("Quit %1", m_player),QLatin1String( "" ),
                                   QLatin1String( "quit" ), KIcon( QLatin1String( "application-exit" )), data, 1.0);
        }
    }//--- if (m_useCommands)

    if (context.isValid() && m_searchCollection) {
        QString actionNames;
        QString searchTerm = term;
        QString command;

        if (term.startsWith(m_comPlay,Qt::CaseInsensitive)
                && term.length() > m_comPlay.length()) {
            command = m_comPlay;
            actionNames = PLAY;
        } else if (term.startsWith(m_comAppend, Qt::CaseInsensitive)
                   && term.length() > m_comAppend.length()) {
            command = m_comAppend;
            actionNames = APPEND;
        } else if (term.startsWith(m_comQueue, Qt::CaseInsensitive)
                   && term.length() > m_comQueue.length()) {
            command = m_comQueue;
            actionNames = QUEUE;
        } else {
            actionNames = QString::fromLatin1( "%1,%2,%3").arg(PLAY).arg(APPEND).arg(QUEUE);
        }

	if (!context.isValid())
	{
	    return;
	}
        searchTerm = searchTerm.right(searchTerm.length() - (command.length() + 1));
        matches << searchCollectionFor(searchTerm, actionNames);
        //Adds matches for all song matches for term
    }

    context.addMatches(term, matches);
}

void AudioPlayerControlRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    QDBusInterface tracklist(QString::fromLatin1( "org.mpris.%1").arg(m_player),
                             QLatin1String( "/TrackList" ), QLatin1String( "org.freedesktop.MediaPlayer" ));

    QVariantList data = match.data().value<QVariantList>();

    /* Only Amarok part*/
    QString url = data[2].toString();
    int pos = posInPlaylist(url);
    kDebug() << "pos" << pos;
    QAction *a = match.selectedAction();
    if (data[3].toString().compare(NONE)) {
        if (!a)
        {
            a = action(data[3].toString());
        }
        if (a == action(QUEUE)) {
            KUrl::List list;
            list << KUrl(url);
            KRun::run(QLatin1String( "amarok --queue %u" ), list, 0);
        } else if (a == action(APPEND)) {
            if (!(pos > -1)) {
                tracklist.call(QDBus::NoBlock, QLatin1String( "AddTrack" ), url , false);
            }
        } else {
            //Action play was selected
            if (pos > -1) {
                tracklist.call(QDBus::NoBlock, QLatin1String( "PlayTrack" ), pos);
            } else {
                tracklist.call(QDBus::NoBlock, QLatin1String( "AddTrack" ), url, true);
            }
        }
    }/* Only Amarok part over */ else {
        if ((data[4].toString().compare(QLatin1String( "start" )) == 0)) {
            //The players's interface isn't available but it should be started
            if (!startPlayer()) {
                return;
            }
        }

        QDBusMessage msg = QDBusMessage::createMethodCall(QString::fromLatin1( "org.mpris.%1").arg(m_player),data[0].toString(),
                           data[1].toString(), data[2].toString());
        kDebug() << msg;
        QVariantList args;
        for (int i = 5;data.length() > i;++i) {
            args << data[i];
        }
        msg.setArguments(args);
        QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
    }
}

QList<QAction*> AudioPlayerControlRunner::actionsForMatch(const Plasma::QueryMatch &match)
{
    QList<QAction*> ret;
    QVariantList data = match.data().value<QVariantList>();

    if (data.length() > 3 && data[3].toString().compare(NONE)) {
        if (!action(PLAY)) {
            addAction(PLAY, KIcon(QLatin1String( "media-playback-start" )), i18n("Play"));
            addAction(QUEUE, KIcon(QLatin1String( "media-track-queue-amarok" )), i18n("Queue"));
            addAction(APPEND, KIcon(QLatin1String( "media-track-add-amarok" )), i18n("Append to playlist"));
        }

        const QStringList actions = data[3].toString().split(QLatin1Char( ',' ));

        for (int i = 0; i < actions.length(); ++i) {
            ret << action(actions[i]);
        }
    }

    return ret;
}

void AudioPlayerControlRunner::reloadConfiguration()
{
    KConfigGroup grp = config();
    m_player = grp.readEntry(CONFIG_PLAYER, "amarok");
    m_useCommands = grp.readEntry(CONFIG_COMMANDS, true);
    m_searchCollection = grp.readEntry(CONFIG_COLLECTION, true);
    m_comPlay = grp.readEntry(CONFIG_PLAY, i18n("play"));
    m_comAppend = grp.readEntry(CONFIG_APPEND, i18n("append"));
    m_comQueue = grp.readEntry(CONFIG_QUEUE, i18n("queue"));
    m_comPause = grp.readEntry(CONFIG_PAUSE, i18n("pause"));
    m_comNext = grp.readEntry(CONFIG_NEXT, i18n("next"));
    m_comMute = grp.readEntry(CONFIG_MUTE, i18n("mute"));
    m_comIncrease = grp.readEntry(CONFIG_INCREASE, i18n("increase"));
    m_increaseBy = qBound(0, grp.readEntry(CONFIG_INCREASE_BY, 15), 100);
    m_comDecrease = grp.readEntry(CONFIG_DECREASE, i18n("decrease"));
    m_decreaseBy = qBound(0, grp.readEntry(CONFIG_DECREASE_BY, 15), 100);
    m_comPrev = grp.readEntry(CONFIG_PREV, i18n("prev"));
    m_comStop = grp.readEntry(CONFIG_STOP, i18n("stop"));
    m_comVolume = grp.readEntry(CONFIG_VOLUME, i18n("volume"));
    m_comQuit = grp.readEntry(CONFIG_QUIT, i18n("quit"));

    /* Adding the syntaxes for helping the user */
    QList<Plasma::RunnerSyntax> syntaxes;

    if (m_player == QLatin1String( "amarok" )) {
        syntaxes << Plasma::RunnerSyntax(m_comPlay + QLatin1String( " :q:" ),
                                         i18n("Plays the selected song. If :q: is not empty it lists songs matching :q: to play them"));
        syntaxes << Plasma::RunnerSyntax(m_comAppend + QLatin1String( " :q:" ),
                                         i18n("Displays songs matching :q: for appending the selected to the playlist"));
        syntaxes << Plasma::RunnerSyntax(m_comQueue + QLatin1String( " :q:" ),
                                         i18n("Displays songs matching :q: for queueing them"));
    } else {
        syntaxes << Plasma::RunnerSyntax(m_comPlay,
                                         i18n("Plays a song from playlist"));
    }
    syntaxes << Plasma::RunnerSyntax(m_comPause,i18n("Pauses the playing"));
    syntaxes << Plasma::RunnerSyntax(m_comNext, i18n("Plays the next song in the playlist if one is available"));
    syntaxes << Plasma::RunnerSyntax(m_comMute, i18n("Mutes/unmutes the player"));
    syntaxes << Plasma::RunnerSyntax(m_comIncrease + QLatin1String( " :q:" ),
                                     i18n("Increases the volume by :q:. If :q: is empty it increases by the configured value"));
    syntaxes << Plasma::RunnerSyntax(m_comDecrease + QLatin1String( " :q:" ),
                                     i18n("Decreases the volume by :q:. If :q: is empty it decreases by the configured value"));
    syntaxes << Plasma::RunnerSyntax(m_comPrev, i18n("Plays the previous song if one is available"));
    syntaxes << Plasma::RunnerSyntax(m_comStop, i18n("Stops the playing"));
    syntaxes << Plasma::RunnerSyntax(m_comVolume + QLatin1String( " :q:" ), i18n("Sets the volume to :q:"));
    syntaxes << Plasma::RunnerSyntax(m_comQuit, i18n("Quits the player"));

    setSyntaxes(syntaxes);
}

Plasma::QueryMatch AudioPlayerControlRunner::createMatch(Plasma::AbstractRunner* runner,
        const QString &title, const QString &subtext, const QString &id,
        const KIcon &icon, const QVariantList &data, const float &relevance)
{
    Plasma::QueryMatch match(runner);
    match.setText(title);
    match.setSubtext(subtext);
    match.setId(id);
    match.setIcon(icon);
    match.setData(data);
    match.setRelevance(relevance);
    return match;
}

bool AudioPlayerControlRunner::playerRunning() const
{
    return m_running;
}

bool AudioPlayerControlRunner::startPlayer() const
{
    if (playerRunning()) {
        return true;
    }

    if (!KRun::run(m_player, KUrl::List(), 0)) {
        //We couldn't start the player
        KMessageBox::error(0, i18n("%1 not found", m_player),
                           i18n("%1 was not found so the runner is unable to work.", m_player));
        return false;
    }

    /*while (!playerRunning()) {
        //Waiting for the player's interface to appear
        ;
    }*/
    return true;
}

int AudioPlayerControlRunner::posInPlaylist(const KUrl& url)
{
    QDBusInterface player(QString::fromLatin1( "org.mpris.%1").arg(m_player), QLatin1String( "/TrackList" ), QLatin1String( "org.freedesktop.MediaPlayer" ));
    for (int i = 0; i < m_songsInPlaylist; i++) {
        QDBusPendingReply<QVariantMap> data = player.asyncCall(QLatin1String( "GetMetadata" ), i);
        KUrl curl = KUrl(KUrl::fromPercentEncoding(data.value().value(QLatin1String( "location" )).toByteArray()));
        kDebug() << curl << ":" << url;
        if (curl == url) {
            return i;
        }
    }
    return -1;
}

void AudioPlayerControlRunner::songsInPlaylist(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<int> reply = *call;
    m_running = !reply.isError();

    if (m_running) {
        m_songsInPlaylist = reply.value();
        if (m_currentTrack > -1) {
            // calculate if the next song is available given the new count
            m_nextSongAvailable = m_songsInPlaylist > m_currentTrack;
        }
    } else {
        m_songsInPlaylist = 0;
    }

    call->deleteLater();
}

void AudioPlayerControlRunner::prevNextSongAvailable(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<int> reply = *call;
    m_running = !reply.isError();

    if (m_running) {
        m_currentTrack = reply.value();
        if (m_songsInPlaylist > 0) {
            m_nextSongAvailable = m_songsInPlaylist > m_currentTrack;
            m_prevSongAvailable = m_currentTrack > 0;
        }
    } else {
        m_currentTrack = 0;
    }

    call->deleteLater();
}

bool AudioPlayerControlRunner::equals(const QString &text, QRegExp reg)
{
    reg.setCaseSensitivity(Qt::CaseInsensitive);
    return reg.exactMatch(text);
}

int AudioPlayerControlRunner::getNumber(const QString& term, const char character)
{
    return term.section(QLatin1Char( character ), 1, 1).toInt();
}

QList<Plasma::QueryMatch> AudioPlayerControlRunner::searchCollectionFor(const QString &term, const QString &actionNames)
{
    QDBusInterface amarok(QString::fromLatin1( "org.mpris.%1").arg(m_player),
                          QLatin1String( "/Collection" ), QLatin1String( "org.kde.amarok.Collection" ));


    QString query(QLatin1String( "<query version=\"1.0\"><limit value=\"5\" /><filters>" ));
    QStringList queryItems = term.split(QLatin1Char( ' ' ), QString::SkipEmptyParts);
    foreach(const QString &queryItem, queryItems) {
        query.append(QString::fromLatin1( "<or><include field=\"title\" value=\"%1\" />").arg(queryItem));
        query.append(QString::fromLatin1( "<or><include field=\"artist\" value=\"%1\" />").arg(queryItem));
        query.append(QString::fromLatin1( "<or><include field=\"album\" value=\"%1\" /></or>").arg(queryItem));
        query.append(QLatin1String( "</or></or>"));
    }

    query.append(QLatin1String( "</filters><includeCollection id=\"localCollection\" /></query>" ));

    QDBusPendingReply<QList<QVariantMap> > reply = amarok.asyncCall(QLatin1String( "MprisQuery" ), query);
    reply.waitForFinished();

    if (!reply.isValid()) {
        return QList<Plasma::QueryMatch>();
    }

    QVariantList data;
    data  << QLatin1String( "/TrackList" ) << QLatin1String( "org.freedesktop.MediaPlayer" );
    QList<Plasma::QueryMatch> matches;
    foreach (const QVariantMap &map, reply.value()) {
        QString artist = map[QLatin1String( "artist" )].toString();
        QString title = map[QLatin1String( "title" )].toString();
        QString url = map[QLatin1String( "location" )].toString();
        double relevance = map[QLatin1String( "rating" )].toInt()*0.2;
        //QString album = map["xesam:album"].toString();

        data << url << actionNames;
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::PossibleMatch);
        match.setIcon(KIcon( QLatin1String( "audio-x-generic" )));
        match.setText(QString::fromLatin1( "%1 - %2").arg(artist).arg(title));
        match.setData(data);
        match.setRelevance(relevance);
        match.setId(QLatin1String( "track_" ) + url);
        matches.append(match);
        data.removeLast();
        data.removeLast();
    }//--- foreach

    return matches;
}

int AudioPlayerControlRunner::currentSong()
{
    QDBusPendingReply<int> current = QDBusInterface(QString::fromLatin1( "org.mpris.%1").arg(m_player), QLatin1String( "/TrackList" ), QLatin1String( "org.freedesktop.MediaPlayer" )).asyncCall(QLatin1String( "GetCurrentTrack" ));
    current.waitForFinished();
    return current;
}
#include "audioplayercontrolrunner.moc"
