/*
 *   Copyright (C) 2005, 2006, 2007 by Siraj Razick <siraj@kde.org>
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

#include "QtDisplay.h"

#include <config-lancelot.h>

#ifdef LANCELOT_HAVE_COMPOSITING

QtDisplay::QtDisplay(char * display, int screen)
{
    _displayPtr = display;
    _screen = screen;
    _alpha = 0;
    _colormap = 0;
    _visualPtr = 0;

    _dpyPtr = XOpenDisplay(_displayPtr);
    this->testDisplay();
    _screen = DefaultScreen(_dpyPtr);
    setupXRender();
}

void QtDisplay::testDisplay()
{
    if (!_dpyPtr) {
        exit(1);
    }

}

QtDisplay::~QtDisplay()
{

}

/*Manpage : function Proto types
 1.) XVisualInfo *XGetVisualInfo ( Display *display , long vinfo_mask , XVisualInfo *vinfo_template , int *nitems_return );

 2.)
 */

void QtDisplay::setupXRender()
{
    int tmp, tmp2;
    if (!XRenderQueryExtension(_dpyPtr, &tmp, &tmp2))
        return;

    XRenderPictFormat * m_xrender_pict_formatPtr;

    int m_xv_count = 0;
    XVisualInfo m_xvinfo;
    XVisualInfo * m_xvinfoAddr = &m_xvinfo;

    m_xvinfo.depth = 32;
    m_xvinfo.c_class = TrueColor;
    m_xvinfo.screen = _screen;

    long m_vinfo_mask = VisualScreenMask | VisualDepthMask | VisualClassMask;

    XVisualInfo * m_xvinfoPtr;
    m_xvinfoPtr = XGetVisualInfo(_dpyPtr, m_vinfo_mask, m_xvinfoAddr, &m_xv_count);

    for (int index = 0; index < m_xv_count ; index++) {
        m_xrender_pict_formatPtr = XRenderFindVisualFormat(_dpyPtr,
            m_xvinfoPtr[index].visual);

        if (m_xrender_pict_formatPtr->type == PictTypeDirect) {
            if (m_xrender_pict_formatPtr->direct.alphaMask) {
                _visualPtr = m_xvinfoPtr[index].visual;
                _colormap = XCreateColormap(_dpyPtr, RootWindow(_dpyPtr,_screen), _visualPtr, AllocNone);
            }
        }
    }

}

#endif // LANCELOT_HAVE_COMPOSITING
