/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "lancelot.h"
#include "KDebug"
#include "KIcon"

#include <QDBusInterface>
#include <QDBusReply>


Lancelot::Lancelot(QObject *parent, const QVariantList &args) :
    Plasma::Applet(parent, args), m_icon(NULL), m_dbus(NULL), m_clientID(-1)
{
    m_icon = new Plasma::Icon(KIcon("lancelot"), "Lancelot", this);
    m_icon->setGeometry(QRectF(0, 0, 64, 64));
    setAcceptsHoverEvents(true);
    //setHandlesChildEvents(true);
    m_dbus = new QDBusInterface( "org.kde.lancelot", "/Lancelot", "org.kde.lancelot.App");

    //kDebug() << "Calling DBUS " << m_dbus->call( "addClient" ).errorMessage() << "\n";
    kDebug() << "Registering this applet with Lancelot application\n";
    QDBusReply<int> reply = m_dbus->call("addClient");

    if (reply.isValid()) {
        m_clientID = reply.value();
    }

    //m_icon->setIcon(KIcon("lanabout-kde"));
}

Lancelot::~Lancelot() {
    kDebug() << "Calling DBUS " << m_dbus->call( "removeClient", "id" ).errorMessage() << "\n";
    delete m_icon;
    delete m_dbus;
}

void Lancelot::hoverEnterEvent (QGraphicsSceneHoverEvent * event) {
    Q_UNUSED(event);
    kDebug() << "Calling DBUS " << m_dbus->call( "show" ).errorMessage() << "\n";
}

void Lancelot::hoverLeaveEvent (QGraphicsSceneHoverEvent * event) {
    Q_UNUSED(event);
    kDebug() << "Calling DBUS " << m_dbus->call( "hide" ).errorMessage() << "\n";
}

QSizeF Lancelot::contentSizeHint () const {
    return QSizeF(64, 64);
}


#include "lancelot.moc"
