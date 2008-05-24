/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "minimize-all.h"
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>
#include <KIcon>
#include <kwindowsystem.h>
#include <netwm.h>
#include <KIconLoader>
#include <QX11Info>
#include <QTimer>
#include <QGraphicsLinearLayout>

MinimizeAll::MinimizeAll(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_wm2ShowingDesktop(false)
#ifndef MINIMIZE_ONLY
      , m_down(false), m_goingDown(false)
#endif
{
    setAspectRatioMode(Plasma::Square);
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize * 2, iconSize * 2);
}

MinimizeAll::~MinimizeAll()
{
}

void MinimizeAll::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    Plasma::Icon *icon = new Plasma::Icon(KIcon("plasma-minimize-all"), QString(), this);
    layout->addItem(icon);
    connect(icon, SIGNAL(clicked()), this, SLOT(pressed()));

    NETRootInfo info(QX11Info::display(), NET::Supported);
    m_wm2ShowingDesktop = info.isSupported(NET::WM2ShowingDesktop);

#ifndef MINIMIZE_ONLY
    if (m_wm2ShowingDesktop) {
        connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(reset()));
    }
#endif
}

QSizeF MinimizeAll::contentSizeHint() const
{
    QSizeF sizeHint = geometry().size();

    if (containment() && containment()->containmentType() ==
                         Plasma::Containment::PanelContainment) {
        switch (formFactor()) {
            case Plasma::Vertical:
                sizeHint.setHeight(sizeHint.width());
                break;
            case Plasma::Horizontal:
                sizeHint.setWidth(sizeHint.height());
                break;
            default:
                break;
        }
    }
    return sizeHint;
}

void MinimizeAll::pressed()
{
    if (m_wm2ShowingDesktop) {
        NETRootInfo info(QX11Info::display(), 0);
#ifndef MINIMIZE_ONLY
        m_down = !m_down;
        m_goingDown = m_down;
        info.setShowingDesktop(m_down);
        // NETRootInfo::showingDesktop() returns always false
        QTimer::singleShot(500, this, SLOT(delay()));
#else
        info.setShowingDesktop(true);
#endif
    }
}

#ifndef MINIMIZE_ONLY

void MinimizeAll::delay()
{
    m_goingDown = false;
}

void MinimizeAll::reset()
{
    if (!m_goingDown) {
        m_down = false;
    }
}

#endif

#include "minimize-all.moc"
