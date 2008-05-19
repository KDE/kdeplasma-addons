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

#include <QPainter>

#include <KGlobalSettings>
#include <Plasma/Theme>

#include <QGraphicsGridLayout>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QFont>

NowPlaying::NowPlaying(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_state(NoPlayer),
      m_artistLabel(new QLabel),
      m_titleLabel(new QLabel),
      m_albumLabel(new QLabel),
      m_timeLabel(new QLabel),
      m_layout(0)
{
    resize(200, 60);
    setMinimumSize(100, 30);
}

NowPlaying::~NowPlaying()
{
}

QGraphicsLayoutItem* NowPlaying::createLabel(const QString& text)
{
    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);

    QLabel* label = new QLabel(text);
    label->setFont(font);
    label->setAutoFillBackground(false);

    QGraphicsProxyWidget* proxy = new QGraphicsProxyWidget;
    proxy->setWidget(label);

    return proxy;
}

QGraphicsLayoutItem* NowPlaying::createLabel(QLabel* label)
{
    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);

    label->setFont(font);
    label->setAutoFillBackground(false);

    QGraphicsProxyWidget* proxy = new QGraphicsProxyWidget;
    proxy->setWidget(label);

    return proxy;
}

void NowPlaying::init()
{
    m_layout = new QGraphicsGridLayout;

    m_layout->addItem(createLabel(i18nc("For a song or other music", "Artist:")), 0, 0);
    m_layout->addItem(createLabel(m_artistLabel), 0, 1);

    m_layout->addItem(createLabel(i18nc("For a song or other music", "Title:")), 1, 0);
    m_layout->addItem(createLabel(m_titleLabel), 1, 1);

    m_layout->addItem(createLabel(i18nc("For a song or other music", "Album:")), 2, 0);
    m_layout->addItem(createLabel(m_albumLabel), 2, 1);

    m_layout->addItem(createLabel(i18nc("Position in a song", "Time:")), 3, 0);
    m_layout->addItem(createLabel(m_timeLabel), 3, 1);

    qreal width = 0;
    for (int i = 0; i < m_layout->rowCount(); i++) {
        QGraphicsLayoutItem* item = m_layout->itemAt(i, 0);
        if (item) {
            width = qMax(width, item->preferredWidth());
        }
    }
    m_layout->setColumnFixedWidth(0, width + 5);

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

    m_artistLabel->setText(data["Artist"].toString());
    m_albumLabel->setText(data["Album"].toString());
    m_titleLabel->setText(data["Title"].toString());

    int length = data["Length"].toInt();
    if (length != 0) {
        int pos = data["Position"].toInt();
        m_timeLabel->setText(QString::number(pos / 60) + ':' +
                             QString::number(pos % 60).rightJustified(2, '0') + " / " +
                             QString::number(length / 60) + ':' +
                             QString::number(length % 60).rightJustified(2, '0'));
    } else {
        m_timeLabel->setText(QString());
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
