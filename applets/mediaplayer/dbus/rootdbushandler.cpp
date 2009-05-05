/******************************************************************************
 * Copyright (C) 2008 Ian Monroe <ian@monroe.nu>                              *
 *           (C) 2008 Peter ZHOU <peterzhoulei@gmail.com>                     *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#include "rootdbushandler.h"


#include "rootadaptor.h"

// Marshall the DBusVersion data into a D-BUS argument
QDBusArgument &operator<<(QDBusArgument &argument, const Version &version)
{
    argument.beginStructure();
    argument << version.major << version.minor;
    argument.endStructure();
    return argument;
}

// Retrieve the DBusVersion data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, Version &version)
{
    argument.beginStructure();
    argument >> version.major >> version.minor;
    argument.endStructure();
    return argument;
}



RootDBusHandler::RootDBusHandler(QObject *parent)
    : QObject( parent )
{
    qDBusRegisterMetaType<Version>();

    setObjectName("RootDBusHandler");
    new RootAdaptor( this );
    QDBusConnection::sessionBus().registerObject("/", this);
}

QString RootDBusHandler::Identity()
{
    return QString( "%1 %2" ).arg( "PlasmaMediaPlayer", 0.1 );
}

void RootDBusHandler::Quit()
{
    //TODO: close applet? quit plasma? or just do nothing?
}

Version RootDBusHandler::MprisVersion()
{
    struct Version version;
    version.major = 1;
    version.minor = 0;
    return version;
}



#include "rootdbushandler.moc"
