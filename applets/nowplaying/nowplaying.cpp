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

#include <QGraphicsGridLayout>

NowPlaying::NowPlaying(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_state(NoPlayer),
      m_artistLabel(new Plasma::Label),
      m_titleLabel(new Plasma::Label),
      m_albumLabel(new Plasma::Label),
      m_timeLabel(new Plasma::Label),
      m_artistText(new Plasma::Label),
      m_titleText(new Plasma::Label),
      m_albumText(new Plasma::Label),
      m_timeText(new Plasma::Label),
      m_layout(0)
{
    resize(200, 60);
    setMinimumSize(100, 30);
}

NowPlaying::~NowPlaying()
{
}

void NowPlaying::init()
{
    m_layout = new QGraphicsGridLayout;

    // set up labels
    m_artistLabel->setText(i18nc("For a song or other music", "Artist:"));
    m_titleLabel->setText(i18nc("For a song or other music", "Title:"));
    m_albumLabel->setText(i18nc("For a song or other music", "Album:"));
    m_timeLabel->setText(i18nc("Position in a song", "Time:"));

    // create layout
    // TODO: make this configurable
    m_layout->addItem(m_artistLabel, 0, 0);
    m_layout->addItem(m_artistText, 0, 1);
    m_layout->addItem(m_titleLabel, 1, 0);
    m_layout->addItem(m_titleText, 1, 1);
    m_layout->addItem(m_albumLabel, 2, 0);
    m_layout->addItem(m_albumText, 2, 1);
    m_layout->addItem(m_timeLabel, 3, 0);
    m_layout->addItem(m_timeText, 3, 1);

    // calculate the left col width (why can't QGraphicsGridLayout just do this?)
    // DISABLED: for some reason the Labels aren't returning a sensible width
    //           if they did, maybe we could just use setColumnStretchFactor(0,0)?
    qreal width = 0;
    for (int i = 0; i < m_layout->rowCount(); i++) {
        QGraphicsLayoutItem* item = m_layout->itemAt(i, 0);
        if (item) {
            width = qMax(width, item->preferredWidth());
        }
    }
    kDebug() << "Preferred width is" << width;
    //m_layout->setColumnFixedWidth(0, width + 5);
    // instead:
    m_layout->setColumnStretchFactor(0, 1);
    m_layout->setColumnStretchFactor(1, 2);

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
    } else if (data["State"].toString() == "paused") {
        m_state = Paused;
    } else {
        m_state = Stopped;
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
        update();
    } else {
        m_watchingPlayer = players.first();
        kDebug() << "Installing" << m_watchingPlayer << "as watched player";
        dataEngine("nowplaying")->connectSource(m_watchingPlayer, this, 500);
    }
}

#include "nowplaying.moc"
