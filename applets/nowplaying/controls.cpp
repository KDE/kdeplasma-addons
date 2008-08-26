#include "controls.h"

#include <Plasma/Icon>
#include <QGraphicsLinearLayout>
#include <KDebug>

Controls::Controls(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_playpause(new Plasma::Icon(this)),
      m_stop(new Plasma::Icon(this)),
      m_prev(new Plasma::Icon(this)),
      m_next(new Plasma::Icon(this)),
      m_layout(0),
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

    QGraphicsLinearLayout* m_layout = new QGraphicsLinearLayout(Qt::Horizontal);
    // adding stretches -> segfault
    //m_layout->addStretch(1);
    m_layout->addItem(m_prev);
    m_layout->addItem(m_playpause);
    m_layout->addItem(m_stop);
    m_layout->addItem(m_next);
    //m_layout->addStretch(1);

    setLayout(m_layout);
}

Controls::~Controls()
{
}

void Controls::playPauseClicked()
{
    if (m_state == Playing) {
        m_state = Paused;
        m_playpause->setIcon("media-playback-start");
        emit pause();
    } else {
        m_state = Playing;
        m_playpause->setIcon("media-playback-pause");
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

// vim: sw=4 sts=4 et tw=100
