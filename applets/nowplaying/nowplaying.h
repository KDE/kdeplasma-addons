/*
 *   Copyright 2007,2008 by Alex Merry <alex.merry@kdemail.net>
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

class QGraphicsGridLayout;
class QLabel;

class NowPlaying : public Plasma::Applet
{
    Q_OBJECT

public:
    NowPlaying(QObject *parent, const QVariantList &args);
    ~NowPlaying();
    void init();
    void constraintsUpdated(Plasma::Constraints);

public slots:
    void dataUpdated(const QString &name,
                     const Plasma::DataEngine::Data &data);
    void playerAdded(const QString &name);
    void playerRemoved(const QString &name);

private:
    void findPlayer();
    QGraphicsLayoutItem* createLabel(const QString& text);
    QGraphicsLayoutItem* createLabel(QLabel* label);

    enum State {
        Playing,
        Paused,
        Stopped,
        NoPlayer
    };
    QString m_watchingPlayer;
    State m_state;

    QLabel* m_artistLabel;
    QLabel* m_titleLabel;
    QLabel* m_albumLabel;
    QLabel* m_timeLabel;

    QGraphicsGridLayout* m_layout;
};

K_EXPORT_PLASMA_APPLET(nowplaying, NowPlaying)

#endif // NOWPLAYING_H
