/*
 * Copyright 2008  Alex Merry <alex.merry@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */
#ifndef CONTROLS_H
#define CONTROLS_H

#include <QGraphicsWidget>
#include <QFlags>

#include "nowplaying.h"

namespace Plasma {
    class IconWidget;
}
class QGraphicsLinearLayout;

class Controls : public QGraphicsWidget
{
    Q_OBJECT

public:
    enum Button
    {
        NoButtons = 0,
        PlayPauseButton = 1,
        StopButton = 2,
        PreviousButton = 4,
        NextButton = 8,
        AllButtons = 15
    };
    Q_DECLARE_FLAGS(Buttons, Button)

    Controls(QGraphicsWidget *parent = 0);
    ~Controls();

    Buttons displayedButtons() const;
    void setDisplayedButtons(Buttons buttons);

public slots:
    void stateChanged(State state);
    void setController(Plasma::Service* controller);

signals:
    void play();
    void pause();
    void stop();
    void previous();
    void next();

private slots:
    void playPauseClicked();

private:
    Plasma::IconWidget* m_playpause;
    Plasma::IconWidget* m_stop;
    Plasma::IconWidget* m_prev;
    Plasma::IconWidget* m_next;

    QGraphicsLinearLayout* m_layout;

    State m_state;
    Plasma::Service* m_controller;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Controls::Buttons)

#endif // CONTROLS_H
