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
#include <KActionCollection>
#include <KStandardAction>
#include <KAuthorized>
//#include <KGlobalAccel>
//#include <KGlobalSettings>
#include <KLocale>


#include <lancelot/Global.h>
#include "LancelotWindow.h"
#include "LancelotApplication.h"
#include "models/BaseModel.h"

#include "lancelotadaptor.h"

#include "QtDisplay.h"

LancelotApplication * LancelotApplication::m_application = NULL;

LancelotApplication::LancelotApplication(int argc, char **argv)
    : KUniqueApplication(argc, argv), window(0), m_clientsNumber(0), m_lastID(-1)
{
    init();
}

LancelotApplication::LancelotApplication (Display * display, Qt::HANDLE visual, Qt::HANDLE colormap, bool configUnique)
    : KUniqueApplication(display, visual, colormap, configUnique), window(0), m_clientsNumber(0), m_lastID(-1)
{
    init();
}

bool LancelotApplication::event(QEvent * e)
{
    kDebug() << (void *) e;
    kDebug() << e;
    if (e->type() == QEvent::ApplicationDeactivate) {
        hide(true);
    }
    return KUniqueApplication::event(e);

}

void LancelotApplication::init()
{
    window = new LancelotWindow();

    setQuitOnLastWindowClosed(false);

    new AppAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/Lancelot", this);

    m_actionCollection = new KActionCollection(this);
    KAction * a = 0;

    if ( KAuthorized::authorizeKAction("show_lancelot")) {
        a = m_actionCollection->addAction(i18n("Lancelot"), this);
        a->setText(i18n("Lancelot"));
        a->setGlobalShortcut(KShortcut(Qt::ALT + Qt::Key_F5));
        connect(
                a, SIGNAL(triggered(bool)),
                this, SLOT(showCentered())
                );
    }

    Models::ApplicationConnector * ac = Models::ApplicationConnector::instance();
    kDebug() << "Connecting ApplicationConnector to self";
    connect(
            ac, SIGNAL(doSearch(QString)),
            this, SLOT(search(QString))
    );
    connect(
            ac, SIGNAL(doHide(bool)),
            this, SLOT(hide(bool))
    );
}

LancelotApplication::~LancelotApplication()
{
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

    QtDisplay * dpy = new QtDisplay();

    LancelotApplication::m_application = new LancelotApplication(dpy->display(),dpy->visual(),dpy->colormap());
    Lancelot::Instance::setHasApplication(true);

    return LancelotApplication::m_application->exec();
}

bool LancelotApplication::showCentered()
{
    if (!m_application) return false;
    LancelotApplication::m_application->window->lancelotShowCentered();
    return true;
}

bool LancelotApplication::show(int x, int y)
{
    if (!m_application) return false;
    LancelotApplication::m_application->window->lancelotShow(x, y);
    return true;
}

bool LancelotApplication::hide(bool immediate)
{
    if (!m_application || !(LancelotApplication::m_application->window)) return false;
    LancelotApplication::m_application->window->lancelotHide(immediate);
    return true;
}

bool LancelotApplication::showItem(int x, int y, const QString & name)
{
    if (!LancelotApplication::m_application || !(LancelotApplication::m_application->window)) return false;
    LancelotApplication::m_application->window->lancelotShowItem(x, y, name);
    return true;
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
    if (!LancelotApplication::m_application || !(LancelotApplication::m_application->window)) return false;
    m_clientsNumber--;
    m_clients.remove(id);
    return true;
}

bool LancelotApplication::search(const QString & string)
{
    if (!LancelotApplication::m_application || !(LancelotApplication::m_application->window)) return false;
    LancelotApplication::m_application->window->search(string);
    return true;
}

QStringList LancelotApplication::sectionIDs()
{
    if (!window) return QStringList();
    return window->sectionIDs();
}

QStringList LancelotApplication::sectionNames()
{
    if (!window) return QStringList();
    return window->sectionNames();
}

QStringList LancelotApplication::sectionIcons()
{
    if (!window) return QStringList();
    return window->sectionIcons();
}
