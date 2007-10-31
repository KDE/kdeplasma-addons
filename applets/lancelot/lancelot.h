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

#ifndef LANCELOT_HEADER
#define LANCELOT_HEADER

#include <Plasma/Applet>
#include <plasma/widgets/icon.h>

class QDBusInterface;

class Lancelot : public Plasma::Applet
{
    Q_OBJECT
public:
    Lancelot(QObject * parent, const QVariantList &args);
    ~Lancelot();

    QSizeF contentSizeHint () const;

protected:
    void hoverEnterEvent   (QGraphicsSceneHoverEvent * event);
    void hoverLeaveEvent   (QGraphicsSceneHoverEvent * event);

private:
    Plasma::Icon * m_icon;
    QDBusInterface * m_dbus;
    int m_clientID;
};

K_EXPORT_PLASMA_APPLET(lancelot, Lancelot)

#endif
