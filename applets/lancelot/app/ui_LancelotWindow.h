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

    }
    
    void createObjects(QFrame * object)
    {
        Q_UNUSED(object);
        
        // Components
        layoutMain = new Plasma::BorderLayout();
        
        // System area
        layoutSystem = new Plasma::HBoxLayout();
        panelSystem = new Lancelot::Panel();

        systemButtons.append(buttonSystemLockScreen = 
            new Lancelot::ExtenderButton(new KIcon("system-lock-screen"), "Lock Session", "", panelSystem));
        systemButtons.append(buttonSystemLogout = 
            new Lancelot::ExtenderButton(new KIcon("system-log-out"), "Log Out...", "", panelSystem));
        systemButtons.append(buttonSystemSwitchUser = 
            new Lancelot::ExtenderButton(new KIcon("switchuser"), "Switch User", "", panelSystem));
        
        // Sections area
        layoutSections = new Plasma::VBoxLayout();
        panelSections = new Lancelot::Panel();
        
        sectionButtons.append(buttonSectionApplications = 
            new Lancelot::ExtenderButton(new KIcon("make-kdevelop"), "Applications", "", panelSections));
        sectionButtons.append(buttonSectionContacts = 
            new Lancelot::ExtenderButton(new KIcon("kontact"), "Contacts", "", panelSections));
        sectionButtons.append(buttonSectionDocuments = 
            new Lancelot::ExtenderButton(new KIcon("applications-office"), "Documents", "", panelSections));
        sectionButtons.append(buttonSectionSystem = 
            new Lancelot::ExtenderButton(new KIcon("video-display"), "System", "", panelSections));
        
        // Search area     
        layoutSearch = new Plasma::HBoxLayout();
        panelSearch = new Lancelot::Panel();
        
        editSearch = new Plasma::LineEdit(panelSearch);
    }
    
    void setupObjects(QFrame * object)
    {
        Q_UNUSED(object);

        // System area
        layoutMain->addItem(layoutSystem, Plasma::BottomPositioned);
        
        foreach (Lancelot::ExtenderButton * button, systemButtons) {
            button->setActivationMethod(Lancelot::ExtenderButton::EXTENDER);
            button->setExtenderPosition(Lancelot::ExtenderButton::BOTTOM);
            button->setIconSize(QSize(24, 24));
            button->setZValue(SYSTEM_BUTTONS_Z_VALUE);

            m_corona->addItem(button);
            layoutSystem->addItem(button);
        }

        // Sections area
        layoutMain->addItem(layoutSections, Plasma::LeftPositioned);
        layoutSections->setSpacing(0);
        
        foreach (Lancelot::ExtenderButton * button, sectionButtons) {
            button->setActivationMethod(Lancelot::ExtenderButton::HOVER);
            button->setIconSize(QSize(48, 48));
            button->setZValue(1);
            button->setInnerOrientation(Lancelot::BaseWidget::VERTICAL);

            m_corona->addItem(button);
            layoutSections->addItem(button);
        }
        
        

        // Search area     
        layoutMain->addItem(layoutSearch, Plasma::TopPositioned);
        
        layoutSearch->addItem(editSearch);


    }
    
    void setupShell(QFrame * object) {
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
