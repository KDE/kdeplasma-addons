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

#include "LancelotWindow.h"
#include "ToggleExtenderButton.h"

#include <plasma/animator.h>
#include <plasma/phase.h>

#define HIDE_TIMER_INTERVAL 2000

LancelotWindow::LancelotWindow( QWidget * parent, Qt::WindowFlags f )
  : QFrame(parent, f), m_hideTimer(this), m_hovered(false), m_sectionsSignalMapper(NULL), m_phase(NULL)
{
    setupUi(this);
    
    createModels();

    connect(& m_hideTimer, SIGNAL(timeout()), this, SLOT(hide()));
    m_hideTimer.setInterval(HIDE_TIMER_INTERVAL);

    m_phase = new Plasma::Phase(this);

    m_sectionsSignalMapper = new QSignalMapper(this);
    connect (m_sectionsSignalMapper,
        SIGNAL(mapped(const QString &)),
        this,
        SLOT(sectionActivated(const QString &))
    );

    connect(buttonSectionApplications, SIGNAL(activated()), m_sectionsSignalMapper, SLOT(map()));
    m_sectionsSignalMapper->setMapping(buttonSectionApplications, "Applications");

    connect(buttonSectionContacts, SIGNAL(activated()), m_sectionsSignalMapper, SLOT(map()));
    m_sectionsSignalMapper->setMapping(buttonSectionContacts, "Contacts");

    connect(buttonSectionDocuments, SIGNAL(activated()), m_sectionsSignalMapper, SLOT(map()));
    m_sectionsSignalMapper->setMapping(buttonSectionDocuments, "Documents");

    connect(buttonSectionSystem, SIGNAL(activated()), m_sectionsSignalMapper, SLOT(map()));
    m_sectionsSignalMapper->setMapping(buttonSectionSystem, "System");

    kDebug() << "###############################################\n";
    connect(listSectionSystemLeft, SIGNAL(activated(int)), this, SLOT(activated(int)));

}

void LancelotWindow::activated(int index) {
    kDebug() << index << " is activated\n";

}

void LancelotWindow::sectionActivated(const QString & item) {
    kDebug() << item << " is activated\n";
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
    kDebug() << "Mouse has left Lancelot window\n";
    m_hideTimer.start();
}

void LancelotWindow::enterEvent(QEvent * event) {
    Q_UNUSED(event);
    m_hovered = true;
    kDebug() << "Mouse has entered Lancelot window\n";
    m_hideTimer.stop();
}

void LancelotWindow::focusOutEvent(QFocusEvent * event) {
    Q_UNUSED(event);
    m_hovered = false;
    kDebug() << "Window out of focus\n";
    hide();
}

bool LancelotWindow::lancelotShow() {
    foreach (Lancelot::ExtenderButton * btn , systemButtons) {
        kDebug() << "Button's name is " << btn->name() << "\n";
    }
    show();
    m_hideTimer.stop();
    return true;
}

bool LancelotWindow::lancelotHide() {
    if (m_hovered) return false;
    m_hideTimer.start();
    return true;
}

bool LancelotWindow::lancelotShowItem(QString name) {
    Q_UNUSED(name);
    return false;
}

void LancelotWindow::createModels() {
    m_systemLeftModel = new Lancelot::MergedActionListViewModel();
    m_systemLeftModel->addModel(
        NULL, i18n("Places"),             
        m_placesModel = new Lancelot::Models::Places()
    );
    m_systemLeftModel->addModel(
        NULL, i18n("Media"),             
        m_devicesModel = new Lancelot::Models::Devices()
    );
    listSectionSystemLeft->setModel(m_systemLeftModel);
    listSectionSystemRight->setModel(m_devicesModel);
}

