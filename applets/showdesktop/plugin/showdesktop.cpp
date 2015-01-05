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

#include "showdesktop.h"

#include <KGlobalSettings>
#include <KWindowSystem>

#ifdef HAVE_X11
#include <NETWM>
#include <QtX11Extras/QX11Info>
#endif

ShowDesktop::ShowDesktop()
    : m_wm2ShowingDesktop(false)
#ifndef MINIMIZE_ONLY
     ,m_down(false)
#endif
{
#ifdef HAVE_X11
    if (QX11Info::isPlatformX11()) {
        NETRootInfo info(QX11Info::connection(), NET::Supported);
        m_wm2ShowingDesktop = info.isSupported(NET::WM2ShowingDesktop);
    }
#endif
}

ShowDesktop::~ShowDesktop()
{
}

void ShowDesktop::showDesktop()
{
#ifdef HAVE_X11
    if (QX11Info::isPlatformX11()) {
        if (m_wm2ShowingDesktop) {
            NETRootInfo info(QX11Info::connection(), 0);
#ifndef MINIMIZE_ONLY
            m_down = !m_down;
            info.setShowingDesktop(m_down);
            //TODO: Use setShowingDesktop() of KWindowSystem
#else
            info.setShowingDesktop(true);
#endif
        }
    }
#endif
}

#include "showdesktop.moc"
