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

#ifndef UI_LANCELOTWINDOW_H_
#define UI_LANCELOTWINDOW_H_

#include <QtCore>
#include <QtGui>
#include <kwindowsystem.h>
#include <KIcon>

#include <plasma/svg.h>
#include <plasma/plasma.h>
#include <plasma/corona.h>

//#include <plasma/widgets/pushbutton.h>
//#include <plasma/widgets/icon.h>
//#include <plasma/widgets/label.h>
#include <plasma/widgets/lineedit.h>

#include <plasma/widgets/borderlayout.h>
#include <plasma/widgets/boxlayout.h>

//#include "PositionLayout.h"

#include "ExtenderButton.h"
#include "ActionListView.h"
#include "Panel.h"

#define SYSTEM_BUTTONS_Z_VALUE 1

namespace Ui {
class LancelotWindow
{
    //Q_OBJECT
    
protected:
    // Plasma shell
    class CustomGraphicsView : public QGraphicsView {
    public:
        CustomGraphicsView  ( QWidget * parent = 0 )
            : QGraphicsView(parent), m_background(NULL) {};
        CustomGraphicsView  ( QGraphicsScene * scene, QWidget * parent = 0 )
            : QGraphicsView(scene, parent), m_background(NULL) {};
        void drawBackground (QPainter * painter, const QRectF & rect)
        {
            Q_UNUSED(rect);
            if (m_background) {
                m_background->resize(QSizeF(550, 500));
                m_background->paint(painter, 0, 0, "background");
            }
        };

    private:
        Plasma::Svg * m_background;
        friend class LancelotWindow;
    };
    
    Plasma::Svg * m_mainTheme;
    CustomGraphicsView * m_view;
    Plasma::Corona     * m_corona;
    QVBoxLayout        * m_layout;
    
    // Components
    Plasma::BorderLayout * layoutMain;
    
    Plasma::HBoxLayout * layoutCenter;
    
    // System area
    Plasma::HBoxLayout * layoutSystem;
    Lancelot::Panel * panelSystem;

    QList < Lancelot::ExtenderButton * > systemButtons;
    Lancelot::ExtenderButton * buttonSystemLockScreen;
    Lancelot::ExtenderButton * buttonSystemLogout;
    Lancelot::ExtenderButton * buttonSystemSwitchUser;
    
    // Sections area
    Plasma::VBoxLayout * layoutSections;
    Lancelot::Panel * panelSections;
    
    QList < Lancelot::ExtenderButton * > sectionButtons;
    Lancelot::ExtenderButton * buttonSectionApplications;
    Lancelot::ExtenderButton * buttonSectionContacts;
    Lancelot::ExtenderButton * buttonSectionDocuments;
    Lancelot::ExtenderButton * buttonSectionSystem;
    
    // Search area     
    Plasma::HBoxLayout * layoutSearch;
    Lancelot::Panel * panelSearch;

    
    Plasma::LineEdit * editSearch;
    
    void setupUi(QFrame * object) 
    {
        kDebug() << "Ui::LancelotWindow::setupUi() {\n";
        setupShell(object);
        createObjects(object);
        setupObjects(object);

        // TODO: Make it show exactly what you want!!! It works?
        object->resize(550, 500);
        
        layoutMain->setGeometry(QRectF(0, 0, 550, 500));
        layoutMain->setSize(150, Plasma::LeftPositioned);
        layoutMain->setSize(50, Plasma::TopPositioned);
        layoutMain->setSize(50, Plasma::BottomPositioned);
        
        m_corona->setSceneRect(QRectF(0, 0, 550, 500));
        kDebug() << "} // Ui::LancelotWindow::setupUi()\n";
    }
    
    void createObjects(QFrame * object)
    {
        Q_UNUSED(object);
        kDebug() << "Ui::LancelotWindow::createObjects()\n";
        
        // Components
        layoutMain = new Plasma::BorderLayout();
        
        // System area
        layoutSystem = new Plasma::HBoxLayout();
        panelSystem = new Lancelot::Panel();

        systemButtons.clear();
        systemButtons.append(buttonSystemLockScreen = 
            new Lancelot::ExtenderButton("buttonSystemLockScreen", new KIcon("system-lock-screen"), "Lock Session", "", panelSystem));
        systemButtons.append(buttonSystemLogout = 
            new Lancelot::ExtenderButton("buttonSystemLogout", new KIcon("system-log-out"), "Log Out...", "", panelSystem));
        systemButtons.append(buttonSystemSwitchUser = 
            new Lancelot::ExtenderButton("buttonSystemSwitchUser", new KIcon("switchuser"), "Switch User", "", panelSystem));
        
        // Sections area
        layoutSections = new Plasma::VBoxLayout();
        panelSections = new Lancelot::Panel();
        
        sectionButtons.clear();
        sectionButtons.append(buttonSectionApplications = 
            new Lancelot::ExtenderButton("buttonSectionApplications", new KIcon("make-kdevelop"), "Applications", "", panelSections));
        sectionButtons.append(buttonSectionContacts = 
            new Lancelot::ExtenderButton("buttonSectionContacts", new KIcon("kontact"), "Contacts", "", panelSections));
        sectionButtons.append(buttonSectionDocuments = 
            new Lancelot::ExtenderButton("buttonSectionDocuments", new KIcon("applications-office"), "Documents", "", panelSections));
        sectionButtons.append(buttonSectionSystem = 
            new Lancelot::ExtenderButton("buttonSectionSystem", new KIcon("video-display"), "System", "", panelSections));
        
        // Search area     
        layoutSearch = new Plasma::HBoxLayout();
        panelSearch = new Lancelot::Panel();
        
        editSearch = new Plasma::LineEdit(panelSearch);
    }
    
    void setupObjects(QFrame * object)
    {
        Q_UNUSED(object);
        kDebug() << "Ui::LancelotWindow::setupObjects()\n";

        // System area
        layoutMain->addItem(panelSystem, Plasma::BottomPositioned);

        foreach (Lancelot::ExtenderButton * button, systemButtons) {
            button->setActivationMethod(Lancelot::ExtenderButton::EXTENDER);
            button->setExtenderPosition(Lancelot::ExtenderButton::BOTTOM);
            button->setIconSize(QSize(24, 24));
            button->setZValue(SYSTEM_BUTTONS_Z_VALUE);

            //m_corona->addItem(button);
            //layoutSystem->addItem(button);
        }
        panelSystem->setLayout(layoutSystem);
        m_corona->addItem(panelSystem);

        // Sections area
        layoutMain->addItem(panelSections, Plasma::LeftPositioned);
        layoutSections->setSpacing(0);
        
        foreach (Lancelot::ExtenderButton * button, sectionButtons) {
            button->setActivationMethod(Lancelot::ExtenderButton::HOVER);
            button->setIconSize(QSize(48, 48));
            button->setZValue(1);
            button->setInnerOrientation(Lancelot::BaseWidget::VERTICAL);

            //m_corona->addItem(button);
            //layoutSections->addItem(button);
        }
        panelSections->setLayout(layoutSections);
        m_corona->addItem(panelSections);
        
        // Search area     
        layoutMain->addItem(panelSearch, Plasma::TopPositioned);
        
        layoutSearch->addItem(editSearch);
        panelSearch->setLayout(layoutSearch);
        m_corona->addItem(panelSearch);



    }
    
    void setupShell(QFrame * object) {
        kDebug() << "Ui::LancelotWindow::setupShell()\n";
        
        object->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
        KWindowSystem::setState( object->winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove );
        
        m_corona = new Plasma::Corona(object);
        m_layout = new QVBoxLayout(object);
        m_layout->setContentsMargins (0, 0, 0, 0);
        object->setLayout(m_layout);
        
        m_view = new CustomGraphicsView(m_corona, object);
        m_view->setFrameStyle(QFrame::NoFrame);
        m_view->setCacheMode(QGraphicsView::CacheBackground);
        m_view->setInteractive(true);
        m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // TODO: Change to off
        m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   // TODO: Change to off

        m_view->m_background = new Plasma::Svg("lancelot/theme");
        m_view->m_background->setContentType(Plasma::Svg::ImageSet);
        
        m_layout->addWidget(m_view);
    }
};

} // namespace Ui

#endif /*UI_LANCELOTWINDOW_H_*/
