/*
 *   Copyright (C) 2008 Bruno Virlet <bvirlet@kdemail.net>
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

#ifndef AUDIOPLAYERCONTROLRUNNER_H
#define AUDIOPLAYERCONTROLRUNNER_H

#include <plasma/abstractrunner.h>

#include <KIcon>

class QDBusPendingCallWatcher;

class AudioPlayerControlRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    AudioPlayerControlRunner(QObject *parent, const QVariantList& args);
    ~AudioPlayerControlRunner();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);
    QList<QAction*> actionsForMatch(const Plasma::QueryMatch &match);

    void reloadConfiguration();

private:
    /** A own method to create a match easily
      * @param context the terms context
      * @param runner the instance of AudioPlayerControlRunner
      * @param term the search term
      * @param term the main text of the match
      * @param subtext the subtext of the match
      * @param icon the icon of the match
      * @param data the data of the match
      * @param relevance the relevance of the match
      * @return the created match
      */
    Plasma::QueryMatch createMatch(Plasma::AbstractRunner* runner,
                                   const QString &title, const QString &subtext,
                                   const QString &id, const KIcon &icon,
                                   const QVariantList &data, const float &relevance);

    /** Tests if the player is running
      * @return @c true if the DBus interface of the player is availalbe, @c false in any other case
      */
    bool playerRunning() const;

    /** Starts the player detached from the current progress if it isn't started already.
      * @return @c true if it was successful, @c false in any other case
      */
    bool startPlayer() const;

    /** Returns the position of the song @c song in the playlist
      * @return the position of the song, -1 if the song is not in it
      */
    int posInPlaylist(const KUrl& url);

    /** Tests, if text and reg match
      * @param text the string
      * @param reg the regular expression
      * @return @c true if they match, @c false in any other case
      */
    bool equals(const QString &text, QRegExp reg);

    /** Looks for the number in the command term (it isn't case sensitive)
      * @param text the term
      * @param character the separator
      * @return the number as int
      */
    int getNumber(const QString& term, const char character);

    /** Searches the collection for term (it searches in artist,
      * album and title)
      * @param term the term to search for, it will be split at whitespaces
      * and it will be searched after every part
      * @param actionNames contains the actions (play, append, queue) separated with a QLatin1Char( ',' )
      * @return a list of Plasma::QueryMatch, where every element is for one match
      */
    QList<Plasma::QueryMatch> searchCollectionFor(const QString& term, const QString& actionNames);

    /** @return the index of the selected song in the player's playlist */
    int currentSong();

private slots:
    void prep();
    void songsInPlaylist(QDBusPendingCallWatcher *call);
    void prevNextSongAvailable(QDBusPendingCallWatcher *call);

private:
    /** The player this runner controls */
    QString m_player;

    //The commands
    /** Command for play a song */
    QString m_comPlay;
    /** Command for append a song */
    QString m_comAppend;
    /** Command for queue a song */
    QString m_comQueue;
    /** Command for pause playing */
    QString m_comPause;
    /** Command for stop playing */
    QString m_comStop;
    /** Command for playing the next song */
    QString m_comNext;
    /** Command for playing the previous song */
    QString m_comPrev;
    /** Command for mute */
    QString m_comMute;
    /** Commnd for increase the volume */
    QString m_comIncrease;
    /** Command for decrease the volume */
    QString m_comDecrease;
    /** Command for changing the volume */
    QString m_comVolume;
    /** Command for quit the player */
    QString m_comQuit;

    /** How much to increase */
    int m_increaseBy;

    /** How much to decrease */
    int m_decreaseBy;

    /** The number of songs in the playlist */
    int m_songsInPlaylist;

    /** The current track, as set on prep */
    int m_currentTrack;

    /** Search the collection */
    bool m_searchCollection : 1;

    /** Use the commands */
    bool m_useCommands : 1;

    /** The running state of the player */
    bool m_running : 1;

    /** @c true if a next song is available */
    bool m_nextSongAvailable : 1;

    /** @c true if a previous song is available */
    bool m_prevSongAvailable : 1;
};

K_EXPORT_PLASMA_RUNNER(audioplayercontrol, AudioPlayerControlRunner)

#endif
