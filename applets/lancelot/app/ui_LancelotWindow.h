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

#ifndef UI_LANCELOTWINDOW_H_
#define UI_LANCELOTWINDOW_H_

#include <QtCore>
#include <QtGui>
#include <kwindowsystem.h>
#include <KIcon>

#include <plasma/svg.h>
#include <plasma/plasma.h>
#include <plasma/corona.h>

#include <plasma/widgets/pushbutton.h>
//#include <plasma/widgets/icon.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/lineedit.h>

#include <plasma/widgets/borderlayout.h>
#include <plasma/widgets/nodelayout.h>
#include <plasma/widgets/boxlayout.h>

#include "ExtenderButton.h"
#include "ActionListView.h"
#include "ActionListViewModels.h"
#include "Panel.h"
#include "CardLayout.h"
#include "Global.h"

#define SYSTEM_BUTTONS_Z_VALUE 1

// CreateSection (Panel, lAyout, LeftList, RightList)
#define CreateSection(SECTION) \
    (layoutSection ## SECTION) = new Plasma::NodeLayout(); \
    (listSection ## SECTION ## Left) = new Lancelot::ActionListView(QString("listSection") + #SECTION + "Left", new Lancelot::DummyActionListViewModel(#SECTION, 20), (panelSection ## SECTION)); \
    (listSection ## SECTION ## Right) = new Lancelot::ActionListView(QString("listSection") + #SECTION + "Right", new Lancelot::DummyActionListViewMergedModel(#SECTION, 5), (panelSection ## SECTION));

// SetupSection (lAyout, LeftList, RightList)
#define SetupSection(SECTION) \
    (layoutSection ## SECTION)->addItem( \
        (listSection ## SECTION ## Left), \
        Plasma::NodeLayout::NodeCoordinate(0, 0, 0, 0), \
        Plasma::NodeLayout::NodeCoordinate(0.5, 1.0, -4, 0) \
    ); \
    (layoutSection ## SECTION)->addItem( \
        (listSection ## SECTION ## Right), \
        Plasma::NodeLayout::NodeCoordinate(0.5, 0, 4, 0), \
        Plasma::NodeLayout::NodeCoordinate(1.0, 1.0, 0, 0) \
    ); \
    (listSection ## SECTION ## Left)->setExtenderPosition(Lancelot::ExtenderButton::Left); \
    (listSection ## SECTION ## Right)->setExtenderPosition(Lancelot::ExtenderButton::Right); \
    (panelSection ## SECTION)->setLayout(layoutSection ## SECTION);

#define DebugSection(SECTION) ;
/*\
    kDebug() << "panelSection" << #SECTION << " " << (long) (panelSection ## SECTION) << "\n"; \
    kDebug() << "layoutSection" << #SECTION << " " << (long) (layoutSection ## SECTION) << "\n"; \
    kDebug() << "listSection" << #SECTION << "Left " << (long) (listSection ## SECTION ## Left) << "\n"; \
    kDebug() << "                         " << (listSection ## SECTION ## Left)->name() << "\n"; \
    kDebug() << "listSection" << #SECTION << "Right " << (long) (listSection ## SECTION ## Right) << "\n"; \
    kDebug() << "                         " << (listSection ## SECTION ## Right)->name() << "\n";*/
    
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
    
    // System area
    Plasma::BorderLayout * layoutSystem;
    Plasma::NodeLayout * layoutSystemButtons;
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
    Plasma::NodeLayout * layoutSearch;
    Lancelot::Panel * panelSearch;
    Plasma::LineEdit * editSearch;
    Plasma::Label * labelSearch;
    
    // Center area
    Lancelot::CardLayout * layoutCenter;
    QList < Lancelot::Panel * > sectionPanels;
    Lancelot::Panel * panelSectionApplications;
    
    // Center area :: Search
    Lancelot::Panel * panelSectionSearch;
    Plasma::NodeLayout * layoutSectionSearch;
    Lancelot::ActionListView * listSectionSearchLeft;   // Applications... maybe something more
    Lancelot::ActionListView * listSectionSearchRight;  // Strigi results
    
    // Center area :: System
    Lancelot::Panel * panelSectionSystem;
    Plasma::NodeLayout * layoutSectionSystem;
    Lancelot::ActionListView * listSectionSystemLeft;   // Places (devices, media)
    Lancelot::ActionListView * listSectionSystemRight;  // Configuration
    
    // Center area :: Documents
    Lancelot::Panel * panelSectionDocuments;
    Plasma::NodeLayout * layoutSectionDocuments;
    Lancelot::ActionListView * listSectionDocumentsLeft;   // Open applications???
    Lancelot::ActionListView * listSectionDocumentsRight;  // Recent documents
    
    // Center area :: Contacts
    Lancelot::Panel * panelSectionContacts;
    Plasma::NodeLayout * layoutSectionContacts;
    Lancelot::ActionListView * listSectionContactsLeft;   // Status (mail, on-line contacts...)
    Lancelot::ActionListView * listSectionContactsRight;  // Actions (new mail?)
    
    
    // Dummy - debug vars
    
    void setupTests(QFrame * object)
    {
        Q_UNUSED(object);
    }
    
    void setupUi(QFrame * object) 
    {
        kDebug() << "Ui::LancelotWindow::setupUi() {\n";
        setupShell(object);
        createObjects(object);
        setupObjects(object);
        setupTests(object);

        object->resize(550, 500);
        
        Lancelot::Global::activateAll();

        layoutMain->setGeometry(QRectF(0, 0, 550, 500));
        
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
        layoutSystem = new Plasma::BorderLayout();
        layoutSystemButtons = new Plasma::NodeLayout();
        panelSystem = new Lancelot::Panel("panelSystem");

        systemButtons.append(buttonSystemLockScreen = 
            new Lancelot::ExtenderButton("buttonSystemLockScreen", new KIcon("system-lock-screen"), "Lock Session", "", panelSystem));
        systemButtons.append(buttonSystemLogout = 
            new Lancelot::ExtenderButton("buttonSystemLogout", new KIcon("system-log-out"), "Log Out...", "", panelSystem));
        systemButtons.append(buttonSystemSwitchUser = 
            new Lancelot::ExtenderButton("buttonSystemSwitchUser", new KIcon("switchuser"), "Switch User", "", panelSystem));
        
        // Sections area
        layoutSections = new Plasma::VBoxLayout();
        panelSections = new Lancelot::Panel("panelSections");
        
        sectionButtons.append(buttonSectionApplications = 
            new Lancelot::ExtenderButton("buttonSectionApplications", new KIcon("make-kdevelop"), "Applications", "", panelSections));
        sectionButtons.append(buttonSectionContacts = 
            new Lancelot::ExtenderButton("buttonSectionContacts", new KIcon("kontact"), "Contacts", "", panelSections));
        sectionButtons.append(buttonSectionDocuments = 
            new Lancelot::ExtenderButton("buttonSectionDocuments", new KIcon("applications-office"), "Documents", "", panelSections));
        sectionButtons.append(buttonSectionSystem = 
            new Lancelot::ExtenderButton("buttonSectionSystem", new KIcon("video-display"), "System", "", panelSections));
        
        // Search area     
        layoutSearch = new Plasma::NodeLayout();
        panelSearch = new Lancelot::Panel("panelSearch");
        
        editSearch = new Plasma::LineEdit(panelSearch);
        labelSearch = new Plasma::Label(panelSearch);
        
        // Main area
        layoutCenter = new Lancelot::CardLayout();
        
        sectionPanels.append(panelSectionApplications = 
            new Lancelot::Panel("panelSectionApplications", new KIcon("make-kdevelop"), "Applications"));
        sectionPanels.append(panelSectionContacts = 
            new Lancelot::Panel("panelSectionContacts", new KIcon("kontact"), "Contacts"));
        sectionPanels.append(panelSectionDocuments = 
            new Lancelot::Panel("panelSectionDocuments", new KIcon("applications-office"), "Documents"));
        sectionPanels.append(panelSectionSystem = 
            new Lancelot::Panel("panelSectionSystem", new KIcon("video-display"), "System"));
        sectionPanels.append(panelSectionSearch = 
            new Lancelot::Panel("panelSectionSearch", new KIcon("find"), "Search"));
        
        // Center area :: Sections
        CreateSection (Search);
        CreateSection (System);
        CreateSection (Documents);
        CreateSection (Contacts);
        DebugSection (Search);
        DebugSection (System);
        DebugSection (Documents);
        DebugSection (Contacts);

    }
    
    void setupObjects(QFrame * object)
    {
        Q_UNUSED(object);
        kDebug() << "Ui::LancelotWindow::setupObjects()\n";

        layoutMain->setSpacing(0);
        layoutMain->setMargin(0);
        
        // System area
        layoutMain->addItem(panelSystem, Plasma::BottomPositioned);
        layoutMain->setSize(50.0, Plasma::BottomPositioned);
        
        layoutSystem->setSpacing(0);
        layoutSystem->setMargin(0);
        layoutSystem->setSize(128.0, Plasma::LeftPositioned);
        layoutSystem->setSize(20.0,  Plasma::BottomPositioned);
        layoutSystem->addItem(layoutSystemButtons);
        
        layoutSystemButtons->setSpacing(0);
        layoutSystemButtons->setMargin(0);

        qreal leftCoord = 0.0;
        qreal leftShift = 0.0;
        foreach (Lancelot::ExtenderButton * button, systemButtons) {
            button->setActivationMethod(Lancelot::ExtenderButton::Extender);
            button->setExtenderPosition(Lancelot::ExtenderButton::Bottom);
            button->setIconSize(QSize(24, 24));
            button->setZValue(SYSTEM_BUTTONS_Z_VALUE);

            layoutSystemButtons->addItem(button, 
                    Plasma::NodeLayout::NodeCoordinate(leftCoord, 0, leftShift, 0),
                    Plasma::NodeLayout::NodeCoordinate(leftCoord + 1.0/3.0, 1, leftShift - 4, 0)
            );
            leftCoord += 1.0/3.0;
            leftShift += 2.0;
        }
        panelSystem->setLayout(layoutSystem);
        m_corona->addItem(panelSystem);

        // Sections area
        layoutMain->addItem(panelSections, Plasma::LeftPositioned);
        layoutMain->setSize(128.0, Plasma::LeftPositioned);
        layoutSections->setSpacing(0);
        layoutSections->setMargin(0);
        
        foreach (Lancelot::ExtenderButton * button, sectionButtons) {
            button->setActivationMethod(Lancelot::ExtenderButton::Hover);
            button->setIconSize(QSize(48, 48));
            button->setZValue(1);
            button->setInnerOrientation(Lancelot::BaseActionWidget::Vertical);

            layoutSections->addItem(button);
        }
        panelSections->setLayout(layoutSections);
        m_corona->addItem(panelSections);
        
        // Search area     
        layoutMain->addItem(panelSearch, Plasma::TopPositioned);
        layoutMain->setSize(50.0, Plasma::TopPositioned);
        
        layoutSearch->addItem(editSearch,
            Plasma::NodeLayout::NodeCoordinate(0.5, 0.5, 0, 0),
            Plasma::NodeLayout::NodeCoordinate(1.0, 0.5, -10, INFINITY) 
        );
        layoutSearch->addItem(labelSearch,
            Plasma::NodeLayout::NodeCoordinate(0.5, 0.5, 0, 0),
            1.1, 0.5
        );
        labelSearch->setText("Search:");
        
        panelSearch->setLayout(layoutSearch);
        m_corona->addItem(panelSearch);
        
        // Center area
        layoutCenter->setMargin(0);
        foreach (Lancelot::Panel * panel, sectionPanels) {
            layoutCenter->addItem(panel, panel->title()); // TODO: Change this - i18n will break it...
            m_corona->addItem(panel);
            kDebug() << "Adding " << panel->title() << " to card layout\n";
        }
        layoutMain->addItem(layoutCenter, Plasma::CenterPositioned);
        
        SetupSection (Search);
        SetupSection (System);
        SetupSection (Documents);
        SetupSection (Contacts);
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
        m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        m_view->m_background = new Plasma::Svg("lancelot/theme");
        m_view->m_background->setContentType(Plasma::Svg::ImageSet);
        
        m_layout->addWidget(m_view);
    }
};

} // namespace Ui

#endif /*UI_LANCELOTWINDOW_H_*/
