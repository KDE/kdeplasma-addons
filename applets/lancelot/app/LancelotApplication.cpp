
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



LancelotApplication * LancelotApplication::application = 0;

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

int LancelotApplication::main(int argc, char **argv) {
    KLocale::setMainCatalog("lancelot");

    KCmdLineArgs::init(argc, argv, 
        new KAboutData(
            QByteArray("lancelot"),
            QByteArray(""),
            ki18n("Lancelot"),
            QByteArray("0.2"),
            ki18n("Next generation application launcher"),
            KAboutData::License_GPL,
            ki18n("(C) 2006 Ivan Čukić"),
            KLocalizedString(),
            QByteArray(""),
            QByteArray("ivan(dot)cukic(at)kdemail(dot)com")
        )
    );

    LancelotApplication::application = new LancelotApplication(argc, argv);

    return LancelotApplication::application->exec();
}

bool LancelotApplication::show() {
    kDebug() << "DCOP CALL SHOW\n";
    return window->lancelotShow();
}

bool LancelotApplication::hide() {
    kDebug() << "DCOP CALL HIDE\n";
    return window->lancelotHide();
}

bool LancelotApplication::showItem(QString name) {
    return window->lancelotShowItem(name);
}

int LancelotApplication::addClient() {
    m_clientsNumber++;
    ++ m_lastID;
    m_clients.insert(m_lastID);
    return (m_lastID);
}

bool LancelotApplication::removeClient(int id) {
    if (!m_clients.contains(id)) return false;
    m_clientsNumber--;
    m_clients.remove(id);
    return true;
}
