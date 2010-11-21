/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "LancelotApplication.h"

#include <QIcon>
#include <QtDBus>

#include <KAboutData>
#include <KAction>
#include <KCmdLineArgs>
#include <KCrash>
#include <KDebug>
#include <KLocale>

#include <Lancelot/Global>

#include <Lancelot/Models/BaseModel>

#include "AboutData.h"
#include "LancelotWindow.h"
#include "lancelotadaptor.h"

#include "QtDisplay.h"

LancelotApplication * LancelotApplication::m_application = NULL;

LancelotApplication::LancelotApplication(int argc, char **argv)
    : KUniqueApplication(argc, argv), window(0),
    m_clientsNumber(0), m_lastID(-1)
{
    init();
}

LancelotApplication::LancelotApplication (Display * display, Qt::HANDLE visual, Qt::HANDLE colormap, bool configUnique)
    : KUniqueApplication(display, visual, colormap, configUnique), window(0),
    m_clientsNumber(0), m_lastID(-1)
{
    init();
}

bool LancelotApplication::event(QEvent * e)
{
    if (e->type() == QEvent::ApplicationDeactivate) {
        hide(true);
    }
    return KUniqueApplication::event(e);
}

void LancelotApplication::init()
{
    window = new LancelotWindow();

    setQuitOnLastWindowClosed(false);
    KCrash::setFlags(KCrash::AutoRestart);

    new AppAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/Lancelot", this);

    Lancelot::Models::ApplicationConnector * ac = Lancelot::Models::ApplicationConnector::self();
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
    qDebug() << "load main catalog ###";
    KLocale::setMainCatalog("lancelot");

    KAboutData * about = new AboutData();
    KCmdLineArgs::init(argc, argv, about);

#ifdef LANCELOT_HAVE_COMPOSITING
    QtDisplay * dpy = new QtDisplay();
    LancelotApplication::m_application =
        new LancelotApplication(
                dpy->display(),
                dpy->visual(),
                dpy->colormap());
#else
    LancelotApplication::m_application =
        new LancelotApplication(argc, argv);
#endif // LANCELOT_HAVE_COMPOSITING

    return LancelotApplication::m_application->exec();
}

bool LancelotApplication::showCentered()
{
    if (!m_application) return false;
    LancelotApplication::m_application->window->lancelotShowCentered();
    return true;
}

bool LancelotApplication::isShowing()
{
    if (!m_application) return false;
    return !(LancelotApplication::m_application->window->isHidden());
}

bool LancelotApplication::isShowing(const QString & section)
{
    if (!m_application) return false;
    kDebug() << LancelotApplication::m_application->window->currentSection() << section;
    return (LancelotApplication::m_application->window->currentSection() == section);
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

void LancelotApplication::configureShortcuts()
{
    if (!window) return;
    window->configureShortcuts();
}

void LancelotApplication::configurationChanged()
{
    if (!window) return;
    window->configurationChanged();
}

void LancelotApplication::showMenuEditor()
{
    if (!window) return;
    window->showMenuEditor();
}

void LancelotApplication::setImmutability(int immutable)
{
    Lancelot::Global::self()->setImmutability(
            (Plasma::ImmutabilityType) immutable);
}

int LancelotApplication::immutability() const
{
    return Lancelot::Global::self()->immutability();
}
