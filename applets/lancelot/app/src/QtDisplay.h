/*
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kde.org>
 *   Copyright (C) 2007 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef QT_DISPLAY
#define QT_DISPLAY

#include <config-lancelot.h>

#ifdef LANCELOT_HAVE_COMPOSITING

#include <QTextStream>
#include <QImage>
#include <QPaintDevice>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QMainWindow>

#include <X11/extensions/Xrender.h>

/**
 About QtDisplay Class
 This is  simple class to initialize a
 X session and open the display. The class
 has two member functions to find out
 the visuals and the color map of the
 display
 **/

class QtDisplay {
public:
    explicit QtDisplay(char * display = 0, int screen = 0);
    virtual ~QtDisplay();
    virtual void testDisplay();
    void setupXRender();

    Qt::HANDLE visual() {return Qt::HANDLE(_visualPtr); }
    Qt::HANDLE colormap() {return Qt::HANDLE(_colormap); }
    Display * display() {return _dpyPtr; }

private:
    int _alpha;
    int _screen;
    char * _displayPtr;
    Colormap _colormap;
    Visual * _visualPtr;
    Display * _dpyPtr;

};

#endif // LANCELOT_HAVE_COMPOSITING

#endif // QT_DISPLAY

