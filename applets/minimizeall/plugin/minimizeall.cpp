/*
 *    Copyright 2015 Sebastian Kügler <sebas@kde.org>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "minimizeall.h"

#include <KWindowSystem>

#include <QDebug>
#include <QTimer>

MinimizeAll::MinimizeAll()
{
    m_active = false;
}

MinimizeAll::~MinimizeAll()
{
}

bool MinimizeAll::active() const
{
    return m_active;
}

void MinimizeAll::minimizeAllWindows()
{
    m_minimizedWindows.clear();

    for (auto wid: KWindowSystem::self()->stackingOrder()) {
        if (match(wid, false)) { // skip hidden windows
            m_minimizedWindows << wid;
            KWindowSystem::self()->minimizeWindow(wid);
        }
    }
    if (!m_active) {
        m_active = true;
        Q_EMIT activeChanged();
    }

    // delay connecting to the window and desktop changes so we skip the
    // active window change that happens after minimizing
    QTimer::singleShot(100, this, [this] () {
        connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged,
                this, &MinimizeAll::reset);
        connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged,
                this, &MinimizeAll::reset);
    });
    qDebug() << "Minimized " << m_minimizedWindows.count() << " windows";
}

void MinimizeAll::unminimizeAllWindows()
{
    qDebug() << "UnMini " << m_minimizedWindows.count() << " windows";
    for (auto wid: m_minimizedWindows) {
        if (match(wid)) {
            KWindowSystem::self()->unminimizeWindow(wid);
        }
    }
    reset();
}

bool MinimizeAll::match(const WId& wid, bool includehidden) const
{
    bool _match = true;
    KWindowInfo info(wid, NET::WMState | NET::WMWindowType | NET::WMDesktop);
    if (!includehidden && info.hasState(NET::Hidden)) {
        _match = false;
    } else if (info.windowType(NET::DockMask ) == NET::Dock) {
        _match = false;
    } else if (!info.isOnCurrentDesktop()) {
        _match = false;
    }

    return _match;
}

void MinimizeAll::reset()
{
    if (m_active) {
        disconnect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged,
                   this, &MinimizeAll::reset);
        disconnect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged,
                   this, &MinimizeAll::reset);

        m_minimizedWindows.clear();
        m_active = false;
        Q_EMIT activeChanged();
    }
}

