/***************************************************************************
 *   Copyright 2008 by Olivier Goffart <ogoffart@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef EYES_H
#define EYES_H

#include <Plasma/Applet>
#include <Plasma/Svg>
#include <Plasma/SvgWidget>

class Eyes : public Plasma::Applet
{
    Q_OBJECT
    public:
        Eyes(QObject *parent, const QVariantList &args);
        ~Eyes();

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect& contentsRect);
        void constraintsEvent(Plasma::Constraints constraints);
        
    protected:
        void timerEvent(QTimerEvent *);
        Plasma::SvgWidget *leftPupil, *rightPupil;
        int timerId;
        int timerInterval;
        QPoint previousMousePos;
        Plasma::Svg *m_svg;
};

K_EXPORT_PLASMA_APPLET(eyes, Eyes)

#endif
