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
#include "controls.h"

#include <Plasma/IconWidget>
#include <QGraphicsLinearLayout>
#include <KDebug>

Controls::Controls(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_playpause(new Plasma::IconWidget(this)),
      m_stop(new Plasma::IconWidget(this)),
      m_prev(new Plasma::IconWidget(this)),
      m_next(new Plasma::IconWidget(this)),
      m_state(NoPlayer)
{
    m_playpause->setIcon("media-playback-start");
    connect(m_playpause, SIGNAL(clicked()), this, SLOT(playPauseClicked()));
    m_playpause->setMinimumSize(m_playpause->sizeFromIconSize(16));
    m_stop->setIcon("media-playback-stop");
    connect(m_stop, SIGNAL(clicked()), this, SIGNAL(stop()));
    m_stop->setMinimumSize(m_stop->sizeFromIconSize(16));
    m_prev->setIcon("media-skip-backward");
    connect(m_prev, SIGNAL(clicked()), this, SIGNAL(previous()));
    m_prev->setMinimumSize(m_prev->sizeFromIconSize(16));
    m_next->setIcon("media-skip-forward");
    connect(m_next, SIGNAL(clicked()), this, SIGNAL(next()));
    m_next->setMinimumSize(m_next->sizeFromIconSize(16));

    setDisplayedButtons(AllButtons);
}

Controls::~Controls()
{
}

void Controls::playPauseClicked()
{
    if (m_state == Playing) {
        m_state = Paused;
        m_playpause->setIcon("media-playback-start");
        m_controller->associateWidget(m_playpause, "play");
        emit pause();
    } else {
        m_state = Playing;
        m_playpause->setIcon("media-playback-pause");
        m_controller->associateWidget(m_playpause, "pause");
        emit play();
    }
}

void Controls::stateChanged(State state)
{
    if (m_state != state) {
        if (state == Playing) {
            m_playpause->setIcon("media-playback-pause");
            m_controller->associateWidget(m_playpause, "pause");
        } else {
            m_playpause->setIcon("media-playback-start");
            m_controller->associateWidget(m_playpause, "play");
        }
        /* Probably not needed
        if (state == NoPlayer) {
            setController(0);
        }
        */
        m_state = state;
    }
}

void Controls::setController(Plasma::Service* controller)
{
    m_controller = controller;

    if (!controller) {
        m_prev->setEnabled(false);
        m_next->setEnabled(false);
        m_stop->setEnabled(false);
        m_playpause->setEnabled(false);
    } else {
        controller->associateWidget(m_prev, "previous");
        controller->associateWidget(m_next, "next");
        controller->associateWidget(m_stop, "stop");
        if (m_state == Playing) {
            controller->associateWidget(m_playpause, "pause");
        } else {
            controller->associateWidget(m_playpause, "play");
        }
    }
}

Controls::Buttons Controls::displayedButtons() const
{
    Buttons result;
    if (m_prev->isVisible())
    {
        result |= PreviousButton;
    }
    if (m_next->isVisible())
    {
        result |= NextButton;
    }
    if (m_playpause->isVisible())
    {
        result |= PlayPauseButton;
    }
    if (m_stop->isVisible())
    {
        result |= StopButton;
    }
    return result;
}

static void showHideButton(QGraphicsLinearLayout* layout,
                           QGraphicsWidget* button,
                           bool show)
{
    if (show)
    {
        button->show();
        layout->addItem(button);
        kDebug() << "Button minimum size:" << button->minimumSize();
        kDebug() << "Button preferred size:" << button->preferredSize();
    }
    else
    {
        button->hide();
    }
}

void Controls::setDisplayedButtons(Buttons buttons)
{
    kDebug() << "Minimum size before changing buttons:" << minimumSize();

    setLayout(0);
    kDebug() << "Layout:" << (QObject*)layout();
    delete layout();

    QGraphicsLinearLayout* newLayout = new QGraphicsLinearLayout(Qt::Horizontal);

    newLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    showHideButton(newLayout, m_prev, (buttons & PreviousButton));
    showHideButton(newLayout, m_playpause, (buttons & PlayPauseButton));
    showHideButton(newLayout, m_stop, (buttons & StopButton));
    showHideButton(newLayout, m_next, (buttons & NextButton));
    //newLayout->addStretch();
    setLayout(newLayout);

    kDebug() << "Minimum size after changing buttons:" << minimumSize();
}

// vim: sw=4 sts=4 et tw=100
