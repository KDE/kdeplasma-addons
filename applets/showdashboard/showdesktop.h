/***************************************************************************
 *   Copyright 2007 by André Duffeck <duffeck@kde.org>                     *
 *   Copyright 2007 by Alexis Ménard <darktears31@gmail.com>               *
 *   Copyright 2007 by Marco Martin <notmart@gmail.com>                    *
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

#ifndef SHOWDESKTOP_H
#define SHOWDESKTOP_H


//KDE
#include <KIcon>

// Plasma
#include <Plasma/Applet>
#include <plasma/widgets/icon.h>
//Qt
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QPainter>

class ShowDesktop : public Plasma::Applet
{
    Q_OBJECT
    public:
        ShowDesktop(QObject *parent, const QVariantList &args);
        ~ShowDesktop() {};
        void init();
	Qt::Orientations expandingDirections() const;
    protected:
        void constraintsEvent(Plasma::Constraints constraints);
    private:
	Plasma::Icon * m_icon;
    protected slots:
        void toggleShowDesktop(bool);
};

K_EXPORT_PLASMA_APPLET(showdesktop, ShowDesktop)

#endif
