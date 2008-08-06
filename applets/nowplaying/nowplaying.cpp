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
#include "infopanel.h"

#include <Plasma/Slider>

#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>


K_EXPORT_PLASMA_APPLET(nowplaying, NowPlaying)


NowPlaying::NowPlaying(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_controller(0),
      m_state(NoPlayer),
      m_caps(NoCaps),
      m_volume(0),
      m_length(0),
      m_textPanel(new InfoPanel),
      m_buttonPanel(new Controls),
      m_volumeSlider(new Plasma::Slider(this)),
      m_positionSlider(new Plasma::Slider(this))
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    resize(300, 200);
    setMinimumSize(300, 200);

    connect(m_buttonPanel, SIGNAL(play()), this, SLOT(play()));
    connect(m_buttonPanel, SIGNAL(pause()), this, SLOT(pause()));
    connect(m_buttonPanel, SIGNAL(stop()), this, SLOT(stop()));
    connect(m_buttonPanel, SIGNAL(previous()), this, SLOT(prev()));
    connect(m_buttonPanel, SIGNAL(next()), this, SLOT(next()));
    connect(this, SIGNAL(stateChanged(State)),
            m_buttonPanel, SLOT(stateChanged(State)));
    connect(this, SIGNAL(capsChanged(Caps)),
            m_buttonPanel, SLOT(setCaps(Caps)));

    connect(this, SIGNAL(metadataChanged(QMap<QString,QString>)),
            m_textPanel, SLOT(updateMetadata(QMap<QString,QString>)));

    m_volumeSlider->setOrientation(Qt::Vertical);
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(100);
    m_volumeSlider->setValue(0);
    connect(this, SIGNAL(volumeChanged(int)),
            m_volumeSlider, SLOT(setValue(int)));
    connect(m_volumeSlider, SIGNAL(sliderMoved(int)),
            this, SLOT(setVolume(int)));
    m_volumeSlider->setEnabled(false);

    m_positionSlider->setOrientation(Qt::Horizontal);
    m_positionSlider->setMinimum(0);
    m_positionSlider->setMaximum(0);
    m_positionSlider->setValue(0);
    connect(this, SIGNAL(positionChanged(int)),
            m_positionSlider, SLOT(setValue(int)));
    connect(this, SIGNAL(lengthChanged(int)),
            m_positionSlider, SLOT(setMaximum(int)));
    connect(m_positionSlider, SIGNAL(sliderMoved(int)),
            this, SLOT(setPosition(int)));
    m_positionSlider->setEnabled(false);
}

NowPlaying::~NowPlaying()
{
}

void NowPlaying::init()
{
    m_layout = new QGraphicsGridLayout();
    m_layout->addItem(m_textPanel, 0, 0);
    m_layout->addItem(m_buttonPanel, 1, 0);
    m_layout->addItem(m_positionSlider, 2, 0);
    m_layout->addItem(m_volumeSlider, 0, 1, 3, 1); // rowspan, colspan

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

    QString timeText;
    int length = data["Length"].toInt();
    if (length != m_length) {
        m_length = length;
        if (length == 0) {
            emit positionChanged(0);
        }
        emit lengthChanged(m_length);
    }
    if (length != 0) {
        int pos = data["Position"].toInt();
        timeText = QString::number(pos / 60) + ':' +
                   QString::number(pos % 60).rightJustified(2, '0') + " / " +
                   QString::number(length / 60) + ':' +
                   QString::number(length % 60).rightJustified(2, '0');
        // we assume it's changed
        emit positionChanged(pos);
    }

    QMap<QString,QString> metadata;
    metadata["Artist"] = data["Artist"].toString();
    metadata["Album"] = data["Album"].toString();
    metadata["Title"] = data["Title"].toString();
    metadata["Time"] = timeText;
    metadata["Track number"] = QString::number(data["Track number"].toInt());
    metadata["Comment"] = data["Comment"].toString();
    metadata["Genre"] = data["Genre"].toString();

    // the time should usually have changed
    emit metadataChanged(metadata);

    // TODO: we should set a tooltip with the timeText on the position slider

    if (data["Volume"].toDouble() != m_volume) {
        m_volume = data["Volume"].toDouble();
        emit volumeChanged(m_volume * 100);
    }

    // used for seeing when the track has changed
    QString track = metadata["Artist"] + " - " + metadata["Title"];

    // assume the artwork didn't change unless the track did
    if (track != m_track) {
        m_track = track;

        m_artwork = data["Artwork"].value<QPixmap>();
        emit coverChanged(m_artwork);
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
    if (data["Can seek"].toBool()) {
        newcaps |= CanSeek;
    }
    if (data["Can set volume"].toBool()) {
        newcaps |= CanSetVolume;
    }
    if (newcaps != m_caps) {
        emit capsChanged(newcaps);
        m_caps = newcaps;
        m_positionSlider->setEnabled(m_caps & CanSeek);
        m_volumeSlider->setEnabled(m_caps & CanSetVolume);
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
        m_positionSlider->setEnabled(false);
        m_volumeSlider->setEnabled(false);
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

void NowPlaying::setVolume(int volumePercent)
{
    qreal volume = ((qreal)qBound(0, volumePercent, 100)) / 100;
    if (m_controller) {
        KConfigGroup op = m_controller->operationDescription("volume");
        op.writeEntry("level", volume);
        m_controller->startOperationCall(op);
    }
}

void NowPlaying::setPosition(int position)
{
    if (m_controller) {
        KConfigGroup op = m_controller->operationDescription("seek");
        op.writeEntry("seconds", position);
        m_controller->startOperationCall(op);
    }
}

#include "nowplaying.moc"
