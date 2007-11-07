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

#include "LancelotWindow.h"
#include "ToggleExtenderButton.h"

#include <KRecentDocument>
#include <plasma/animator.h>
#include <plasma/phase.h>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <kworkspace/kworkspace.h>
#include "ksmserver_interface.h"
#include "screensaver_interface.h"
#include "krunner_interface.h"

#define HIDE_TIMER_INTERVAL 2000

LancelotWindow::LancelotWindow( QWidget * parent, Qt::WindowFlags f )
  : QFrame(parent, f), m_hideTimer(this), m_hovered(false), m_sectionsSignalMapper(NULL), m_phase(NULL)
{
    setWindowOpacity(0.9);
    setupUi(this);

    setAttribute(Qt::WA_NoSystemBackground);
    m_view->setAutoFillBackground(false);

    createModels();

    connect(& m_searchTimer, SIGNAL(timeout()), this, SLOT(doSearch()));
    m_searchTimer.setInterval(200);
    m_searchTimer.setSingleShot(true);

    connect(& m_hideTimer, SIGNAL(timeout()), this, SLOT(hide()));
    m_hideTimer.setInterval(HIDE_TIMER_INTERVAL);
    m_hideTimer.setSingleShot(true);

    m_phase = new Plasma::Phase(this);

    m_sectionsSignalMapper = new QSignalMapper(this);
    connect (m_sectionsSignalMapper,
        SIGNAL(mapped(const QString &)),
        this,
        SLOT(sectionActivated(const QString &))
    );

    QMapIterator<QString, Lancelot::ToggleExtenderButton * > i(sectionButtons);
    while (i.hasNext()) {
        i.next();
        connect(i.value(), SIGNAL(activated()), m_sectionsSignalMapper, SLOT(map()));
        m_sectionsSignalMapper->setMapping(i.value(), i.key());
    }

    connect(buttonSystemLockScreen, SIGNAL(activated()), this, SLOT(systemLock()));
    connect(buttonSystemLogout, SIGNAL(activated()),     this, SLOT(systemLogout()));
    connect(buttonSystemSwitchUser, SIGNAL(activated()), this, SLOT(systemSwitchUser()));

    setFocusPolicy(Qt::WheelFocus);

}

void LancelotWindow::systemLock()
{
    hide();
    QTimer::singleShot(500, this, SLOT(systemDoLock()));
}

void LancelotWindow::systemLogout()
{
    hide();
    QTimer::singleShot(500, this, SLOT(systemDoLogout()));
}

void LancelotWindow::systemDoLock()
{
    org::freedesktop::ScreenSaver screensaver(
        "org.freedesktop.ScreenSaver",
        "/ScreenSaver",
        QDBusConnection::sessionBus()
    );

    if (screensaver.isValid()) {
        screensaver.Lock();
    }
}

void LancelotWindow::systemDoLogout()
{
    //KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmDefault;
    //KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeNone;
    //KWorkSpace::requestShutDown(confirm, type);
    org::kde::KSMServerInterface smserver(
        "org.kde.ksmserver",
        "/KSMServer",
        QDBusConnection::sessionBus()
    );

    if (smserver.isValid()) {
        smserver.logout(
            KWorkSpace::ShutdownConfirmDefault,
            KWorkSpace::ShutdownTypeDefault,
            KWorkSpace::ShutdownModeDefault
        );
    }
}

void LancelotWindow::systemSwitchUser()
{
    hide();
    QTimer::singleShot(500, this, SLOT(systemDoSwitchUser()));
}

void LancelotWindow::systemDoSwitchUser()
{
    org::kde::krunner::Interface
        krunner("org.kde.krunner", "/Interface", QDBusConnection::sessionBus());
    if (krunner.isValid()) {
        krunner.switchUser();
    }
}

void LancelotWindow::sectionActivated(const QString & item) {
    foreach (Lancelot::ToggleExtenderButton * button, sectionButtons) {
        button->setPressed(false);
    }
    if (sectionButtons.contains(item)) {
        sectionButtons[item]->setPressed(true);
    }
    layoutCenter->show(item);
}

LancelotWindow::~LancelotWindow() {
    delete m_phase;
}

void LancelotWindow::leaveEvent(QEvent * event) {
    Q_UNUSED(event);
    m_hovered = false;
    m_hideTimer.start();
}

void LancelotWindow::enterEvent(QEvent * event) {
    Q_UNUSED(event);
    m_hovered = true;
    m_hideTimer.stop();
}

bool LancelotWindow::lancelotShow() {
    show();
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove );
    m_hideTimer.stop();
    setFocus();
    editSearch->setFocus();
    return true;
}

bool LancelotWindow::lancelotHide(bool immediate) {
    if (immediate) {
        hide();
        return true;
    }

    if (m_hovered) return false;
    m_hideTimer.start();
    return true;
}

bool LancelotWindow::lancelotShowItem(QString name) {
    Q_UNUSED(name);
    return false;
}

void LancelotWindow::createModels() {
    // Computer models

    m_systemLeftModel = new Lancelot::MergedActionListViewModel();
    m_systemLeftModel->addModel(
        NULL, i18n("Places"),
        m_placesModel = new Lancelot::Models::Places()
    );
    m_systemLeftModel->addModel(
        NULL, i18n("System"),
        m_systemServicesModel = new Lancelot::Models::SystemServices()
    );
    sectionListsLeft[sectComputer]->setModel(m_systemLeftModel);

    m_systemRightModel = new Lancelot::MergedActionListViewModel();
    m_systemRightModel->addModel(
        NULL, i18n("Removable"),
        m_devicesModelRemovable = new Lancelot::Models::Devices(Lancelot::Models::Devices::Removable)
    );
    m_systemRightModel->addModel(
        NULL, i18n("Fixed"),
        m_devicesModelFixed = new Lancelot::Models::Devices(Lancelot::Models::Devices::Fixed)
    );
    sectionListsRight[sectComputer]->setModel(m_systemRightModel);

    // Document models
    m_documentsLeftModel = new Lancelot::MergedActionListViewModel();
    m_documentsLeftModel->addModel(
        NULL, i18n("New:"),
        m_newDocumentsModel = new Lancelot::Models::NewDocuments()
    );
    sectionListsLeft[sectDocuments]->setModel(m_documentsLeftModel);

    m_documentsRightModel = new Lancelot::MergedActionListViewModel();
    m_documentsRightModel->addModel(
        NULL, i18n("Recent documents"),
        m_recentDocumentsModel = new Lancelot::Models::RecentDocuments()
    );
    m_documentsRightModel->addModel(
        NULL, i18n("Open documents"),
        m_openDocumentsModel = new Lancelot::Models::OpenDocuments()
    );
    sectionListsRight[sectDocuments]->setModel(m_documentsRightModel);

    // Search Models
    m_runnerModel = new Lancelot::Models::Runner();
    sectionListsLeft[sectSearch]->setModel(m_runnerModel);
    connect(
        editSearch, SIGNAL(textChanged(const QString &)),
        this, SLOT(search(const QString &))
    );

}

void LancelotWindow::search(const QString & string)
{
    if (editSearch->text() != string) {
        editSearch->setText(string);
    }
    m_searchTimer.stop();
    m_searchString = string;
    m_searchTimer.start();
}

void LancelotWindow::doSearch()
{
    m_searchTimer.stop();
    m_runnerModel->setSearchString(m_searchString);
    sectionActivated(sectSearch);
}

QStringList LancelotWindow::sectionIDs()
{
    QStringList result;
    foreach (QString section, sectionsOrder) {
        if (section == sectSearch) continue;
        result << section;
    }
    return result;
}

QStringList LancelotWindow::sectionNames()
{
    QStringList result;
    foreach (QString section, sectionsOrder) {
        if (section == sectSearch) continue;
        result << sectionsData[section].second;
    }
    return result;
}

QStringList LancelotWindow::sectionIcons()
{
    QStringList result;
    foreach (QString section, sectionsOrder) {
        if (section == sectSearch) continue;
        result << sectionsData[section].first;
    }
    return result;
}
