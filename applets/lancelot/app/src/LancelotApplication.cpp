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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <KLocale>
#include <KDebug>
#include <QIcon>
#include <QtDBus>

#include <KAction>
#include <KStandardAction>

#include "LancelotWindow.h"
#include "LancelotApplication.h"

#include "lancelotadaptor.h"



LancelotApplication * LancelotApplication::m_application = NULL;

LancelotApplication::LancelotApplication(int argc, char **argv)
    : KUniqueApplication(argc, argv), window(0), m_clientsNumber(0), m_lastID(-1)
{
    window = new LancelotWindow();
    setQuitOnLastWindowClosed(false);

    kDebug() << "Init DBUS...\n";
    new AppAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/Lancelot", this);
    kDebug() << "DBUS registered...\n";


}

LancelotApplication::~LancelotApplication()
{
}

LancelotApplication * LancelotApplication::application()
{
    return m_application;
}


int LancelotApplication::main(int argc, char **argv)
{
    KLocale::setMainCatalog("lancelot");

    KCmdLineArgs::init(argc, argv,
        new KAboutData(
            QByteArray("lancelot"),
            QByteArray(""),
            ki18n("Lancelot"),
            QByteArray("0.2"),
            ki18n("Next generation application launcher"),
            KAboutData::License_GPL,
            ki18n("(C) 2007 Ivan Čukić"),
            KLocalizedString(),
            QByteArray(""),
            QByteArray("ivan(dot)cukic(at)kdemail(dot)com")
        )
    );

    LancelotApplication::m_application = new LancelotApplication(argc, argv);

    return LancelotApplication::m_application->exec();
}

bool LancelotApplication::show()
{
    kDebug() << "DCOP CALL SHOW\n";
    return window->lancelotShow();
}

bool LancelotApplication::hide(bool immediate)
{
    kDebug() << "DCOP CALL HIDE\n";
    return window->lancelotHide(immediate);
}

bool LancelotApplication::showItem(QString name)
{
    return window->lancelotShowItem(name);
}

int LancelotApplication::addClient()
{
    m_clientsNumber++;
    ++ m_lastID;
    m_clients.insert(m_lastID);
    return (m_lastID);
}

bool LancelotApplication::removeClient(int id)
{
    if (!m_clients.contains(id)) return false;
    m_clientsNumber--;
    m_clients.remove(id);
    return true;
}
