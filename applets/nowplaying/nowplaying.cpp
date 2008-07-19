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


#include <Plasma/Theme>
#include <Plasma/Label>
#include <Plasma/Icon>

#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QLabel>

NowPlaying::NowPlaying(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_controller(0),
      m_state(NoPlayer),
      m_artistLabel(new Plasma::Label),
      m_titleLabel(new Plasma::Label),
      m_albumLabel(new Plasma::Label),
      m_timeLabel(new Plasma::Label),
      m_artistText(new Plasma::Label),
      m_titleText(new Plasma::Label),
      m_albumText(new Plasma::Label),
      m_timeText(new Plasma::Label),
      m_playpause(new Plasma::Icon),
      m_stop(new Plasma::Icon),
      m_prev(new Plasma::Icon),
      m_next(new Plasma::Icon),
      m_textPanel(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    resize(300, 165);
    setMinimumSize(100, 30);
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


    // set up buttons
    m_playpause->setIcon("media-playback-start");
    connect(m_playpause, SIGNAL(clicked()), this, SLOT(playpause()));
    m_stop->setIcon("media-playback-stop");
    connect(m_stop, SIGNAL(clicked()), this, SLOT(stop()));
    m_prev->setIcon("media-skip-backward");
    connect(m_prev, SIGNAL(clicked()), this, SLOT(prev()));
    m_next->setIcon("media-skip-forward");
    connect(m_next, SIGNAL(clicked()), this, SLOT(next()));

    m_buttonPanel = new QGraphicsLinearLayout(Qt::Horizontal);
    // adding stretches -> segfault
    //m_buttonPanel->addStretch(1);
    m_buttonPanel->addItem(m_prev);
    m_buttonPanel->addItem(m_playpause);
    m_buttonPanel->addItem(m_stop);
    m_buttonPanel->addItem(m_next);
    //m_buttonPanel->addStretch(1);


    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->addItem(m_textPanel);
    m_layout->addItem(m_buttonPanel);

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
    if (data["State"].toString() == "playing") {
        m_state = Playing;
        m_playpause->setIcon("media-playback-pause");
    } else if (data["State"].toString() == "paused") {
        m_state = Paused;
        m_playpause->setIcon("media-playback-start");
    } else {
        m_state = Stopped;
        m_playpause->setIcon("media-playback-start");
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

    m_playpause->setVisible(data["Can play"].toBool() || data["Can pause"].toBool());
    m_stop->setVisible(data["Can stop"].toBool());
    m_prev->setVisible(data["Can skip backward"].toBool());
    m_next->setVisible(data["Can skip forward"].toBool());

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
        m_watchingPlayer.clear();
        m_controller = 0;
        update();
    } else {
        m_watchingPlayer = players.first();
        m_controller = dataEngine("nowplaying")->serviceForSource(m_watchingPlayer);
        kDebug() << "Installing" << m_watchingPlayer << "as watched player";
        dataEngine("nowplaying")->connectSource(m_watchingPlayer, this, 500);
    }
}

void NowPlaying::playpause()
{
    if (m_state == Playing) {
        m_playpause->setIcon("media-playback-start");
        if (m_controller) {
            m_controller->startOperationCall(m_controller->operationDescription("pause"));
        }
    } else {
        m_playpause->setIcon("media-playback-pause");
        if (m_controller) {
            m_controller->startOperationCall(m_controller->operationDescription("play"));
        }
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
