#include "controls.h"

#include <Plasma/Icon>
#include <QGraphicsLinearLayout>
#include <KDebug>

Controls::Controls()
    : m_playpause(new Plasma::Icon),
      m_stop(new Plasma::Icon),
      m_prev(new Plasma::Icon),
      m_next(new Plasma::Icon),
      m_layout(0),
      m_state(NoPlayer),
      m_caps(NoCaps)
{
    m_playpause->setIcon("media-playback-start");
    connect(m_playpause, SIGNAL(clicked()), this, SLOT(playPauseClicked()));
    m_stop->setIcon("media-playback-stop");
    connect(m_stop, SIGNAL(clicked()), this, SIGNAL(stop()));
    m_prev->setIcon("media-skip-backward");
    connect(m_prev, SIGNAL(clicked()), this, SIGNAL(previous()));
    m_next->setIcon("media-skip-forward");
    connect(m_next, SIGNAL(clicked()), this, SIGNAL(next()));

    setCaps(NoCaps);

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Horizontal);
    // adding stretches -> segfault
    //layout->addStretch(1);
    layout->addItem(m_prev);
    layout->addItem(m_playpause);
    layout->addItem(m_stop);
    layout->addItem(m_next);
    //layout->addStretch(1);

    m_layout = layout;
}

Controls::~Controls()
{
    // FIXME: mem leak if the widget isn't on a canvas, vs. crash if it is...
    // delete m_layout;
    if (!m_playpause->scene()) {
        delete m_playpause;
    }
    if (!m_stop->scene()) {
        delete m_stop;
    }
    if (!m_prev->scene()) {
        delete m_prev;
    }
    if (!m_next->scene()) {
        delete m_next;
    }
}

QGraphicsLayoutItem* Controls::widget() const
{
    return m_layout;
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
            m_playpause->setEnabled(m_caps & CanPause);
        } else {
            m_playpause->setIcon("media-playback-start");
            m_playpause->setEnabled(m_caps & CanPlay);
        }
        if (state == NoPlayer) {
            setCaps(NoCaps);
        }
        m_state = state;
    }
}

void Controls::setCaps(Caps caps)
{
    m_prev->setEnabled(caps & CanGoPrevious);
    m_next->setEnabled(caps & CanGoNext);
    m_stop->setEnabled(caps & CanStop);
    if (m_state == Playing) {
        m_playpause->setEnabled(caps & CanPause);
    } else {
        m_playpause->setEnabled(caps & CanPlay);
    }
    m_caps = caps;
}

// vim: sw=4 sts=4 et tw=100
