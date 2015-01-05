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

#ifndef SHOWDESKTOP_HEADER
#define SHOWDESKTOP_HEADER

// #define MINIMIZE_ONLY

#include <config-kdeplasma-addons.h>
#include <QObject>

class ShowDesktop : public QObject
{

Q_OBJECT

public:
    ShowDesktop();
    ~ShowDesktop();

    Q_INVOKABLE void showDesktop();

private:
    bool m_wm2ShowingDesktop;

#ifndef MINIMIZE_ONLY
    bool m_down;
#endif
};

#endif //SHOWDESKTOP_HEADER
