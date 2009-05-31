/*
 *   Copyright 2007,2008 by Alex Merry <alex.merry@kdemail.net>
 *   Copyright 2008 by Tony Murray <murraytony@gmail.com>
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

#ifndef NOWPLAYING_H
#define NOWPLAYING_H

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/service.h>
#include <Plasma/ToolTipManager>

#include <QPixmap>

class QGraphicsLayout;
namespace Plasma {
    class Slider;
}
class Controls;
class InfoPanel;

enum State {
    Playing,
    Paused,
    Stopped,
    NoPlayer
};

enum CapsFlags {
    NoCaps = 0,
    CanPlay = 1,
    CanPause = 2,
    CanStop = 4,
    CanGoPrevious = 8,
    CanGoNext = 16,
    CanSeek = 32,
    CanSetVolume = 64
};
Q_DECLARE_FLAGS(Caps, CapsFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Caps)

class NowPlaying : public Plasma::Applet
{
    Q_OBJECT

public:
    NowPlaying(QObject *parent, const QVariantList &args);
    ~NowPlaying();
    void init();
    void constraintsEvent(Plasma::Constraints);

signals:
    void stateChanged(State state);
    void controllerChanged(Plasma::Service* controller);
    void metadataChanged(const QMap<QString,QString>& metadata);
    void coverChanged(const QPixmap& picture);
    void lengthChanged(int length);

public slots:
    void dataUpdated(const QString &name,
                     const Plasma::DataEngine::Data &data);
    void playerAdded(const QString &name);
    void playerRemoved(const QString &name);
    void toolTipAboutToShow();

private slots:
    void play();
    void pause();
    void stop();
    void prev();
    void next();
    void setVolume(int volumePercent);
    void setPosition(int position);

private:
    void findPlayer();
    void layoutPlanar();
    void layoutHorizontal();

    QString m_watchingPlayer;
    Plasma::Service* m_controller;
    State m_state;
    enum CurrentLayoutType
    {
        NoLayout,
        PlanarLayout,
        HorizontalLayout
    };
    CurrentLayoutType m_currentLayout;

    qreal m_volume;
    int m_length;
    QString m_artist;
    QString m_title;
    QPixmap m_artwork;

    InfoPanel* m_textPanel;
    Controls* m_buttonPanel;
    Plasma::Slider* m_volumeSlider;
    Plasma::Slider* m_positionSlider;
};

#endif // NOWPLAYING_H
