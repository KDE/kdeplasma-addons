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
#include <QtDBus/QDBusMessage>

#include <KDE/KMessageBox>
#include <KDE/KDebug>
#include <KDE/KIcon>
#include <KDE/KRun>
#include <KDE/KUrl>

#include "audioplayercontrolrunner_config.h"
#include "audioplayercontrolconfigkeys.h"

Q_DECLARE_METATYPE(QList<QVariantMap>)

/** The variable PLAY contains the action label for play */
static const QString PLAY("play");
/** The variable APPEND contains the action label for append */
static const QString APPEND("append");
/** The variable QUEUE contains the action label for queue */
static const QString QUEUE("queue");
/** The variable NONE says that no action is needed */
static const QString NONE("none");


AudioPlayerControlRunner::AudioPlayerControlRunner(QObject *parent, const QVariantList& args)
        : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);

    setObjectName("Audio Player Control Runner");
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
    m_running = playerRunning();
    m_songsInPlaylist = songsInPlaylist();
    m_nextSongAvailable = nextSongAvailable();
    m_prevSongAvailable = prevSongAvailable();
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
         * "/Player" "org.freedesktop.MediaPlayer" "Play" "actions" "start" args...
         * <path>    <interface>                 <method> <actions> <start player>
         * <actions> is NONE if no action is needed
         */

        QVariantList playcontrol;
        playcontrol  << "/Player" << "org.freedesktop.MediaPlayer";

        /* The commands */

        //Play
        if (context.isValid() && m_comPlay.startsWith(term, Qt::CaseInsensitive) &&
	    (!m_running || m_songsInPlaylist)) {
            QVariantList data = playcontrol;
            data << ((currentSong() == -1) ? "Next" : "Play") << NONE << "start";
            matches << createMatch(this, i18n("Start playing"), i18n("Audio player control"), "play",
                                   KIcon("media-playback-start"), data, 1.0);
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
                data << "Next" << NONE << "nostart";
                matches << createMatch(this, i18n("Play next song"), i18n("Audio player control"),
                                       "next", KIcon("media-skip-forward"), data, 1.0);
            }

            //Previous song
            if (context.isValid() && m_comPrev.startsWith(term,Qt::CaseInsensitive)
                    && m_prevSongAvailable) {
                QVariantList data = playcontrol;
                data << "Prev" << NONE << "nostart";
                matches << createMatch(this, i18n("Play previous song"), i18n("Audio player control") ,
                                       "previous", KIcon("media-skip-backward"), data, 1.0);
            }
        }//--- if (m_songsInPlaylist)

        //Pause
        if (context.isValid() && m_comPause.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << "Pause" << NONE << "nostart";
            matches << createMatch(this, i18n("Pause playing"), i18n("Audio player control"),
                                   "pause", KIcon("media-playback-pause"), data, 1.0);
        }

        //Stop
        if (context.isValid() && m_comStop.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << "Stop" << NONE << "nostart";
            matches << createMatch(this, i18n("Stop playing"), i18n("Audio player control"),
                                   "stop", KIcon("media-playback-stop"), data, 1.0);
        }

        //Increase
        if (context.isValid() && m_comIncrease.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << "VolumeUp" << NONE << "nostart" << m_increaseBy;
            matches << createMatch(this, i18n("Increase volume by %1" , m_increaseBy),
                                   "volumeup", i18n("Audio player control"), KIcon("audio-volume-high"), data, 1.0);
        } else if (context.isValid() && equals(term, QRegExp(m_comIncrease + " \\d{1,2}0{0,1}"))) {
            int volumeChange = getNumber(term, ' ');
            QVariantList data = playcontrol;
            data << "VolumeUp" << NONE << "nostart" << volumeChange;
            matches << createMatch(this, i18n("Increase volume by %1" , volumeChange),
                                   "volumeup", i18n("Audio player control"), KIcon("audio-volume-high"), data, 1.0);
        }

        //Decrease
        if (context.isValid() && m_comDecrease.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << "VolumeDown" << NONE << "nostart" << m_decreaseBy;
            matches << createMatch(this, i18n("Reduce volume by %1", m_decreaseBy),
                                   "volumedown", i18n("Audio player control"), KIcon("audio-volume-low"), data, 1.0);
        } else if (context.isValid() && equals(term, QRegExp(m_comDecrease + " \\d{1,2}0{0,1}"))) {
            int volumeChange = getNumber(term, ' ');
            QVariantList data = playcontrol;
            data << "VolumeDown" << NONE << "nostart" << volumeChange;
            matches << createMatch(this, i18n("Reduce volume by %1", volumeChange),
                                   "volumedown", i18n("Audio player control"), KIcon("audio-volume-low"), data, 1.0);
        }

        //Set volume to
        if (context.isValid() && equals(term, QRegExp(m_comVolume + " \\d{1,2}0{0,1}"))) {
            QVariantList data = playcontrol;
            int newVolume = getNumber(term , ' ');
            data << "VolumeSet" << NONE << "nostart" << newVolume;
            matches << createMatch(this, i18n("Set volume to %1%" , newVolume),
                                   "volume", i18n("Audio player control"), KIcon("audio-volume-medium"), data, 1.0);
        }

        //Mute
        if (context.isValid() && m_comMute.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << "Mute" << NONE << "nostart";
            matches << createMatch(this, i18n("Mute"), i18n("Audio player control"),
                                   "mute", KIcon("audio-volume-muted"), data, 1.0);
        }

        //Quit player
        if (context.isValid() && m_comQuit.startsWith(term,Qt::CaseInsensitive)) {
            QVariantList data;
            data  << "/" << "org.freedesktop.MediaPlayer" << "Quit" << NONE
            << "nostart";
            matches << createMatch(this, i18n("Quit %1", m_player),"",
                                   "quit", KIcon("application-exit"), data, 1.0);
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
            actionNames = QString("%1,%2,%3").arg(PLAY).arg(APPEND).arg(QUEUE);
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
    QDBusInterface tracklist(QString("org.mpris.%1").arg(m_player),
                             "/TrackList", "org.freedesktop.MediaPlayer");

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
            KRun::run("amarok --queue %u", list, 0);
        } else if (a == action(APPEND)) {
            if (!(pos > -1)) {
                tracklist.call(QDBus::NoBlock, "AddTrack", url , false);
            }
        } else {
            //Action play was selected
            if (pos > -1) {
                tracklist.call(QDBus::NoBlock, "PlayTrack", pos);
            } else {
                tracklist.call(QDBus::NoBlock, "AddTrack", url, true);
            }
        }
    }/* Only Amarok part over */ else {
        if ((data[4].toString().compare("start") == 0)) {
            //The players's interface isn't available but it should be started
            if (!startPlayer()) {
                return;
            }
        }

        QDBusMessage msg = QDBusMessage::createMethodCall(QString("org.mpris.%1").arg(m_player),data[0].toString(),
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
            addAction(PLAY, KIcon("media-playback-start"), i18n("Play"));
            addAction(QUEUE, KIcon("media-track-queue-amarok"), i18n("Queue"));
            addAction(APPEND, KIcon("media-track-add-amarok"), i18n("Append to playlist"));
        }

        QStringList actions = data[3].toString().split(',');

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

    if (m_player == "amarok") {
        syntaxes << Plasma::RunnerSyntax(m_comPlay + " :q:",
                                         i18n("Plays the selected song. If :q: is not empty it lists songs matching :q: to play them"));
        syntaxes << Plasma::RunnerSyntax(m_comAppend + " :q:",
                                         i18n("Displays songs matching :q: for appending the selected to the playlist"));
        syntaxes << Plasma::RunnerSyntax(m_comQueue + " :q:",
                                         i18n("Displays songs matching :q: for queueing them"));
    } else {
        syntaxes << Plasma::RunnerSyntax(m_comPlay,
                                         i18n("Plays a song from playlist"));
    }
    syntaxes << Plasma::RunnerSyntax(m_comPause,i18n("Pauses the playing"));
    syntaxes << Plasma::RunnerSyntax(m_comNext, i18n("Plays the next song in the playlist if one is available"));
    syntaxes << Plasma::RunnerSyntax(m_comMute, i18n("Mutes/unmutes the player"));
    syntaxes << Plasma::RunnerSyntax(m_comIncrease + " :q:",
                                     i18n("Increases the volume by :q:. If :q: is empty it increases by the configured value"));
    syntaxes << Plasma::RunnerSyntax(m_comDecrease + " :q:",
                                     i18n("Decreases the volume by :q:. If :q: is empty it decreases by the configured value"));
    syntaxes << Plasma::RunnerSyntax(m_comPrev, i18n("Plays the previous song if one is available"));
    syntaxes << Plasma::RunnerSyntax(m_comStop, i18n("Stops the playing"));
    syntaxes << Plasma::RunnerSyntax(m_comVolume + " :q:", i18n("Sets the volume to :q:"));
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

bool AudioPlayerControlRunner::playerRunning()
{
    QDBusInterface intf(QString("org.mpris.%1").arg(m_player), "/");
    return intf.isValid();
}

bool AudioPlayerControlRunner::startPlayer()
{
    if (playerRunning()) {
        return true;
    }

    if (!KRun::run(m_player, KUrl::List(), 0)) {
        //We couldn't start the player
        KMessageBox::error(0, i18n("%1 not found", m_player),
                           i18n("%1 was not found so the runner is unable to work.",m_player));
        return false;
    }

    /*while (!playerRunning()) {
        //Waiting for the player's interface to appear
        ;
    }*/
    return true;
}

int AudioPlayerControlRunner::posInPlaylist(KUrl url)
{
    QDBusInterface player(QString("org.mpris.%1").arg(m_player), "/TrackList", "org.freedesktop.MediaPlayer");
    for (int i = 0; i < songsInPlaylist(); i++)
    {
        QDBusPendingReply<QVariantMap> data = player.asyncCall("GetMetadata", i);
        KUrl curl = KUrl(KUrl::fromPercentEncoding(data.value().value("location").toByteArray()));
        kDebug() << curl << ":" << url;
        if (curl == url)
        {
            return i;
        }
    }
    return -1;
}

int AudioPlayerControlRunner::songsInPlaylist()
{
    QDBusInterface player(QString("org.mpris.%1").arg(m_player), "/TrackList", "org.freedesktop.MediaPlayer");
    QDBusPendingReply<int> length = player.asyncCall("GetLength");
    length.waitForFinished();
    return length.value();
}

bool AudioPlayerControlRunner::nextSongAvailable()
{
    QDBusInterface player(QString("org.mpris.%1").arg(m_player), "/TrackList", "org.freedesktop.MediaPlayer");
    QDBusPendingReply<int> length = player.asyncCall("GetLength");
    QDBusPendingReply<int> current = player.asyncCall("GetCurrentTrack");
    length.waitForFinished();
    current.waitForFinished();
    return !((length.value() - 1) == current.value());
}

bool AudioPlayerControlRunner::prevSongAvailable()
{
    QDBusInterface player(QString("org.mpris.%1").arg(m_player), "/TrackList", "org.freedesktop.MediaPlayer");
    QDBusPendingReply<int> current = player.asyncCall("GetCurrentTrack");
    current.waitForFinished();
    return current.value() > 0;
}

bool AudioPlayerControlRunner::equals(const QString &text, QRegExp reg)
{
    reg.setCaseSensitivity(Qt::CaseInsensitive);
    return reg.exactMatch(text);
}

int AudioPlayerControlRunner::getNumber(const QString& term, const char character)
{
    return term.section(character, 1, 1).toInt();
}

QList<Plasma::QueryMatch> AudioPlayerControlRunner::searchCollectionFor(const QString &term, const QString &actionNames)
{
    QDBusInterface amarok(QString("org.mpris.%1").arg(m_player),
                          "/Collection", "org.kde.amarok.Collection");


    QString query("<query version=\"1.0\"><limit value=\"5\" /><filters>");
    QStringList queryItems = term.split(' ', QString::SkipEmptyParts);
    foreach(const QString &queryItem, queryItems) {
        query.append(QString("<or><include field=\"title\" value=\"%1\" />").arg(queryItem));
        query.append(QString("<or><include field=\"artist\" value=\"%1\" />").arg(queryItem));
        query.append(QString("<or><include field=\"album\" value=\"%1\" /></or>").arg(queryItem));
        query.append(QString("</or></or>"));
    }

    query.append("</filters><includeCollection id=\"localCollection\" /></query>");

    QDBusPendingReply<QList<QVariantMap> > reply = amarok.asyncCall("MprisQuery", query);
    reply.waitForFinished();

    if (!reply.isValid()) {
        return QList<Plasma::QueryMatch>();
    }

    QVariantList data;
    data  << "/TrackList" << "org.freedesktop.MediaPlayer";
    QList<Plasma::QueryMatch> matches;
    foreach (const QVariantMap &map, reply.value()) {
        QString artist = map["artist"].toString();
        QString title = map["title"].toString();
        QString url = map["location"].toString();
        double relevance = map["rating"].toInt()*0.2;
        //QString album = map["xesam:album"].toString();

        data << url << actionNames;
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::PossibleMatch);
        match.setIcon(KIcon("audio-x-generic"));
        match.setText(QString("%1 - %2").arg(artist).arg(title));
        match.setData(data);
        match.setRelevance(relevance);
        match.setId("track_" + url);
        matches.append(match);
        data.removeLast();
        data.removeLast();
    }//--- foreach

    return matches;
}

int AudioPlayerControlRunner::currentSong()
{
    QDBusPendingReply<int> current = QDBusInterface(QString("org.mpris.%1").arg(m_player), "/TrackList", "org.freedesktop.MediaPlayer").asyncCall("GetCurrentTrack");
    current.waitForFinished();
    return current;
}
#include "audioplayercontrolrunner.moc"
