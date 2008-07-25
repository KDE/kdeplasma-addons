/*
 *   Copyright 2007,2008 by Alex Merry <alex.merry@kdemail.net>
 *
 *   Some code (text size calculation) taken from clock applet:
 *   Copyright 2007 by Sebastian Kuegler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#include "nowplaying.h"
#include "controls.h"


#include <Plasma/Theme>
#include <Plasma/Label>

#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QLabel>


K_EXPORT_PLASMA_APPLET(nowplaying, NowPlaying)


NowPlaying::NowPlaying(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_controller(0),
      m_state(NoPlayer),
      m_caps(NoCaps),
      m_artistLabel(new Plasma::Label),
      m_titleLabel(new Plasma::Label),
      m_albumLabel(new Plasma::Label),
      m_timeLabel(new Plasma::Label),
      m_artistText(new Plasma::Label),
      m_titleText(new Plasma::Label),
      m_albumText(new Plasma::Label),
      m_timeText(new Plasma::Label),
      m_textPanel(0),
      m_buttonPanel(new Controls)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    resize(300, 165);
    setMinimumSize(100, 30);

    connect(m_buttonPanel, SIGNAL(play()), this, SLOT(play()));
    connect(m_buttonPanel, SIGNAL(pause()), this, SLOT(pause()));
    connect(m_buttonPanel, SIGNAL(stop()), this, SLOT(stop()));
    connect(m_buttonPanel, SIGNAL(previous()), this, SLOT(prev()));
    connect(m_buttonPanel, SIGNAL(next()), this, SLOT(next()));
    connect(this, SIGNAL(stateChanged(State)),
            m_buttonPanel, SLOT(stateChanged(State)));
    connect(this, SIGNAL(capsChanged(Caps)),
            m_buttonPanel, SLOT(setCaps(Caps)));
}

NowPlaying::~NowPlaying()
{
}

void NowPlaying::init()
{
    // set up labels
    m_textPanel = new QGraphicsGridLayout;
    m_textPanel->setColumnStretchFactor(0, 0);
    m_textPanel->setColumnSpacing(0, 10);
    m_textPanel->setColumnAlignment(0, Qt::AlignRight);

    m_artistLabel->setText(i18nc("For a song or other music", "Artist:"));
    m_artistLabel->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_titleLabel->setText(i18nc("For a song or other music", "Title:"));
    m_titleLabel->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_albumLabel->setText(i18nc("For a song or other music", "Album:"));
    m_albumLabel->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_timeLabel->setText(i18nc("Position in a song", "Time:"));
    m_timeLabel->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // TODO: make this configurable
    m_textPanel->addItem(m_artistLabel, 0, 0);
    m_textPanel->addItem(m_artistText, 0, 1);
    m_textPanel->addItem(m_titleLabel, 1, 0);
    m_textPanel->addItem(m_titleText, 1, 1);
    m_textPanel->addItem(m_albumLabel, 2, 0);
    m_textPanel->addItem(m_albumText, 2, 1);
    m_textPanel->addItem(m_timeLabel, 3, 0);
    m_textPanel->addItem(m_timeText, 3, 1);

    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->addItem(m_textPanel);
    m_layout->addItem(m_buttonPanel->widget());

    setLayout(m_layout);

    connect(dataEngine("nowplaying"), SIGNAL(sourceAdded(QString)),
            SLOT(playerAdded(QString)));
    connect(dataEngine("nowplaying"), SIGNAL(sourceRemoved(QString)),
            SLOT(playerRemoved(QString)));

    findPlayer();
}

void NowPlaying::constraintsUpdated(Plasma::Constraints constraints)
{
    // FIXME: we should probably change to everything in one
    // or two lines if we're horizonal
}

void NowPlaying::dataUpdated(const QString &name,
                             const Plasma::DataEngine::Data &data)
{
                    //i18n("No media player found")
                    //i18nc("The state of a music player", "Stopped")
    if (name != m_watchingPlayer) {
        kDebug() << "Wasn't expecting an update from" << name;
        return;
    }
    if (data.isEmpty()) {
        kDebug() << "Got no data";
        findPlayer();
        return;
    }

    State newstate;
    if (data["State"].toString() == "playing") {
        newstate = Playing;
    } else if (data["State"].toString() == "paused") {
        newstate = Paused;
    } else {
        newstate = Stopped;
    }
    if (newstate != m_state) {
        emit stateChanged(newstate);
        m_state = newstate;
    }

    m_artistText->setText(data["Artist"].toString());
    m_albumText->setText(data["Album"].toString());
    m_titleText->setText(data["Title"].toString());

    int length = data["Length"].toInt();
    if (length != 0) {
        int pos = data["Position"].toInt();
        m_timeText->setText(QString::number(pos / 60) + ':' +
                             QString::number(pos % 60).rightJustified(2, '0') + " / " +
                             QString::number(length / 60) + ':' +
                             QString::number(length % 60).rightJustified(2, '0'));
    } else {
        m_timeText->setText(QString());
    }

    Caps newcaps = NoCaps;
    if (data["Can play"].toBool()) {
        newcaps |= CanPlay;
    }
    if (data["Can pause"].toBool()) {
        newcaps |= CanPause;
    }
    if (data["Can stop"].toBool()) {
        newcaps |= CanStop;
    }
    if (data["Can skip backward"].toBool()) {
        newcaps |= CanGoPrevious;
    }
    if (data["Can skip forward"].toBool()) {
        newcaps |= CanGoNext;
    }
    if (newcaps != m_caps) {
        emit capsChanged(newcaps);
        m_caps = newcaps;
    }

    update();
}

void NowPlaying::playerAdded(const QString &name)
{
    kDebug() << "Player" << name << "added";
    if (m_watchingPlayer.isEmpty()) {
        kDebug() << "Installing" << name << "as watched player";
        m_watchingPlayer = name;
        dataEngine("nowplaying")->connectSource(m_watchingPlayer, this, 500);
    }
}

void NowPlaying::playerRemoved(const QString &name)
{
    kDebug() << "Player" << name << "removed";
    if (m_watchingPlayer == name) {
        findPlayer();
    }
}

void NowPlaying::findPlayer()
{
    QStringList players = dataEngine("nowplaying")->sources();
    kDebug() << "Looking for players.  Possibilities:" << players;
    if (players.isEmpty()) {
        m_state = NoPlayer;
        m_caps = NoCaps;
        m_watchingPlayer.clear();
        m_controller = 0;

        emit stateChanged(m_state);
        emit capsChanged(m_caps);
        update();
    } else {
        m_watchingPlayer = players.first();
        m_controller = dataEngine("nowplaying")->serviceForSource(m_watchingPlayer);
        kDebug() << "Installing" << m_watchingPlayer << "as watched player";
        dataEngine("nowplaying")->connectSource(m_watchingPlayer, this, 999);
    }
}

void NowPlaying::play()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("play"));
    }
}

void NowPlaying::pause()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("pause"));
    }
}

void NowPlaying::stop()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("stop"));
    }
}

void NowPlaying::prev()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("previous"));
    }
}

void NowPlaying::next()
{
    if (m_controller) {
        m_controller->startOperationCall(m_controller->operationDescription("next"));
    }
}

#include "nowplaying.moc"
