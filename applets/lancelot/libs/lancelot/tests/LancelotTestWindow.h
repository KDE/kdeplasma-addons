/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_TEST_WINDOW_H
#define LANCELOT_TEST_WINDOW_H

#include <plasma/svg.h>
#include <plasma/plasma.h>
#include <plasma/corona.h>
#include <KConfig>
#include <KConfigGroup>
#include <QGraphicsView>

#include <../lancelot.h>
#include <../Global.h>

class LancelotTestWindow: public QGraphicsView
{
    Q_OBJECT
public:
    LancelotTestWindow();
    virtual ~LancelotTestWindow();

private:
    Plasma::Corona      * m_corona;
    Lancelot::Instance  * instance;

    friend class CustomGraphicsView;
};

#endif /*LANCELOT_TEST_WINDOW_H*/
