/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "LancelotWindow.h"

#include <plasma/animator.h>
#include <plasma/phase.h>

#define HIDE_TIMER_INTERVAL 2000

LancelotWindow::LancelotWindow( QWidget * parent, Qt::WindowFlags f )
  : QFrame(parent, f), m_hideTimer(this), m_hovered(false), m_categoriesSignalMapper(NULL), m_phase(NULL)
{
    setupUi(this);
    
    connect(& m_hideTimer, SIGNAL(timeout()), this, SLOT(hide()));
    m_hideTimer.setInterval(HIDE_TIMER_INTERVAL);
    
    m_phase = new Plasma::Phase(this);
    
    m_categoriesSignalMapper = new QSignalMapper(this);
    connect (m_categoriesSignalMapper, 
        SIGNAL(mapped(const QString &)), 
        this, 
        SLOT(itemActivated(const QString &))
    );

    /*
    connect(buttonCategoryApplications, SIGNAL(activated()), m_categoriesSignalMapper, SLOT(map()));
    m_categoriesSignalMapper->setMapping(buttonCategoryApplications, "Applications");

    connect(buttonCategoryContacts, SIGNAL(activated()), m_categoriesSignalMapper, SLOT(map()));
    m_categoriesSignalMapper->setMapping(buttonCategoryContacts, "Contacts");

    connect(buttonCategoryDocuments, SIGNAL(activated()), m_categoriesSignalMapper, SLOT(map()));
    m_categoriesSignalMapper->setMapping(buttonCategoryDocuments, "Documents");

    connect(buttonCategorySystem, SIGNAL(activated()), m_categoriesSignalMapper, SLOT(map()));
    m_categoriesSignalMapper->setMapping(buttonCategorySystem, "System");
    */
    
    
}

void LancelotWindow::itemActivated(const QString & item) {
    kDebug() << item << " is activated\n";
    
    Lancelot::Panel * showingPanel = NULL;
    Lancelot::Panel * hidingPanel = NULL;
    
    /*
    foreach (Lancelot::Panel * panel, categoryPanels) {
        if (panel->isVisible()) hidingPanel = panel;
    }

    if (item == "Applications")
        showingPanel = panelCategoryApplications;
    else if (item == "Contacts")
        showingPanel = panelCategoryContacts;
    else if (item == "Documents")
        showingPanel = panelCategoryDocuments;
    else if (item == "System")
        showingPanel = panelCategorySystem;*/

    if (showingPanel == NULL || showingPanel == hidingPanel) return;
    
    if (hidingPanel)
        hidingPanel->setZValue(hidingPanel->zValue() - 1);
    showingPanel->setZValue(showingPanel->zValue() + 1);
    showingPanel->show();
    m_phase->animateItem(showingPanel, Plasma::Phase::Appear);
    if (hidingPanel)
        hidingPanel->hide();

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
