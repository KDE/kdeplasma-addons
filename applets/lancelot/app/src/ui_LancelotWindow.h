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

#ifndef UI_LANCELOTWINDOW_H_
#define UI_LANCELOTWINDOW_H_

#include <QtCore>
#include <QtGui>
#include <kwindowsystem.h>
#include <KIcon>

#include <QVariantList>
#include <klineedit.h>

#include <plasma/svg.h>
#include <plasma/plasma.h>
#include <plasma/corona.h>

#include <plasma/widgets/pushbutton.h>
//#include <plasma/widgets/icon.h>
#include <plasma/widgets/label.h>
//#include <plasma/widgets/lineedit.h>

#include <plasma/widgets/borderlayout.h>
#include <plasma/widgets/nodelayout.h>
#include <plasma/widgets/boxlayout.h>

#include <plasma/containment.h>

#include "WidgetPositioner.h"
#include "ExtenderButton.h"
#include "ToggleExtenderButton.h"
#include "ActionListView.h"
#include "ActionListViewModels.h"
#include "Panel.h"
#include "PassagewayView.h"
#include "CardLayout.h"
#include "Global.h"

#define SYSTEM_BUTTONS_Z_VALUE 1

// CreateSection (Panel, lAyout, LeftList, RightList)
#define CreateSection(SECTION) \
    (layoutSection ## SECTION) = new Plasma::NodeLayout(); \
    (listSection ## SECTION ## Left) = new Lancelot::ActionListView(QString("listSection") + #SECTION + "Left", new Lancelot::DummyActionListViewModel(#SECTION, 20), (panelSection ## SECTION)); \
    (listSection ## SECTION ## Right) = new Lancelot::ActionListView(QString("listSection") + #SECTION + "Right", new Lancelot::DummyMergedActionListViewModel(#SECTION, 5), (panelSection ## SECTION));

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
    (listSection ## SECTION ## Left)->setItemsGroup(Lancelot::WidgetGroup::group("ActionListView-Left-Items")); \
    (listSection ## SECTION ## Right)->setItemsGroup(Lancelot::WidgetGroup::group("ActionListView-Right-Items")); \
    (listSection ## SECTION ## Left)->setExtenderPosition(Lancelot::ExtenderButton::Left); \
    (listSection ## SECTION ## Right)->setExtenderPosition(Lancelot::ExtenderButton::Right); \
    (panelSection ## SECTION)->setLayout(layoutSection ## SECTION);
    //(listSection ## SECTION ## Left)->setGroupByName("ActionListView-Left");
    //(listSection ## SECTION ## Right)->setGroupByName("ActionListView-Right");


namespace Ui {
class LancelotWindow
{
    //Q_OBJECT

protected:
    // Plasma shell
    class CustomGraphicsView : public QGraphicsView {
    public:
        CustomGraphicsView  ( QWidget * parent = 0 )
            : QGraphicsView(parent), m_background(NULL) {}
        CustomGraphicsView  ( QGraphicsScene * scene, QWidget * parent = 0 )
            : QGraphicsView(scene, parent), m_background(NULL) {}
        void drawBackground (QPainter * painter, const QRectF & rect)
        {
            Q_UNUSED(rect); // Q_UNUSED(painter);
            painter->setCompositionMode(QPainter::CompositionMode_Source);
            painter->fillRect(QRectF(rect.x()-2,rect.y()-2,rect.width()+2,rect.height()+2).toRect(), Qt::transparent);
            if (m_background) {
                m_background->resize(QSizeF(550, 500));
                m_background->paint(painter, 0, 0, "background");
            }
        }

    private:
        Plasma::Svg * m_background;
        friend class LancelotWindow;
    };

    Plasma::Svg * m_mainTheme;
    CustomGraphicsView * m_view;
    Plasma::Corona     * m_corona;
    QVBoxLayout        * m_layout;

    Plasma::Containment * testContainment;

    // Components
    Plasma::BorderLayout * layoutMain;

    // System area
    Plasma::BorderLayout * layoutSystem;
    Plasma::NodeLayout * layoutSystemButtons;

    QList < Lancelot::ExtenderButton * > systemButtons;
    Lancelot::ExtenderButton * buttonSystemLockScreen;
    Lancelot::ExtenderButton * buttonSystemLogout;
    Lancelot::ExtenderButton * buttonSystemSwitchUser;

    // Sections area
    Plasma::VBoxLayout * layoutSections;
    Lancelot::Panel * panelSections;

    QMap < QString, Lancelot::ToggleExtenderButton * > sectionButtons;
    Lancelot::ToggleExtenderButton * buttonSectionApplications;
    Lancelot::ToggleExtenderButton * buttonSectionContacts;
    Lancelot::ToggleExtenderButton * buttonSectionDocuments;
    Lancelot::ToggleExtenderButton * buttonSectionSystem;

    // Search area
    Plasma::NodeLayout * layoutSearch;
    Lancelot::Panel * panelSearch;
    Plasma::Label * labelSearch;

    //Plasma::LineEdit * editSearch;
    Lancelot::WidgetPositioner * editSearchPositioner;
    KLineEdit * editSearch;

    // Center area
    QStringList sections;

    Lancelot::CardLayout * layoutCenter;
    QMap < QString, Lancelot::Panel * > sectionPanels;
    Lancelot::PassagewayView * panelSectionApplications;

    // Center area :: Search
    Lancelot::Panel * panelSectionSearch;
    Plasma::NodeLayout * layoutSectionSearch;
    Lancelot::ActionListView * listSectionSearchLeft;   // KRunner
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
        setupShell(object);
        createObjects(object);
        setupObjects(object);
        setupGroups(object);
        setupTests(object);

        object->resize(550, 500);

        Lancelot::Global::instance()->activateAll();

        layoutMain->setGeometry(QRectF(0, 0, 550, 500));

        m_corona->setSceneRect(QRectF(0, 0, 550, 500));
    }

    void createObjects(QFrame * object)
    {
        Q_UNUSED(object);

        testContainment = new Plasma::Containment();

        // Components
        layoutMain = new Plasma::BorderLayout();

        // System area
        layoutSystem = new Plasma::BorderLayout();
        layoutSystemButtons = new Plasma::NodeLayout();

        systemButtons.append(buttonSystemLockScreen =
            new Lancelot::ExtenderButton("buttonSystemLockScreen", new KIcon("system-lock-screen"), "Lock Session", ""));
        systemButtons.append(buttonSystemLogout =
            new Lancelot::ExtenderButton("buttonSystemLogout", new KIcon("system-log-out"), "Log Out...", ""));
        systemButtons.append(buttonSystemSwitchUser =
            new Lancelot::ExtenderButton("buttonSystemSwitchUser", new KIcon("switchuser"), "Switch User", ""));

        // Sections area
        // sections << "Applications" << "Computer" << "Contacts" << "Documents"; // << "Search";
        sections << "Documents" << "Contacts" << "Computer" << "Applications"; // << "Search";

        layoutSections = new Plasma::VBoxLayout();
        panelSections = new Lancelot::Panel("panelSections");

        sectionButtons.insert("Computer", buttonSectionSystem =
            new Lancelot::ToggleExtenderButton("buttonSectionSystem", new KIcon("video-display"), i18n("Computer"), "", panelSections));
        sectionButtons.insert("Documents", buttonSectionDocuments =
            new Lancelot::ToggleExtenderButton("buttonSectionDocuments", new KIcon("applications-office"), i18n("Documents"), "", panelSections));
        sectionButtons.insert("Contacts", buttonSectionContacts =
            new Lancelot::ToggleExtenderButton("buttonSectionContacts", new KIcon("kontact"), i18n("Contacts"), "", panelSections));
        sectionButtons.insert("Applications", buttonSectionApplications =
            new Lancelot::ToggleExtenderButton("buttonSectionApplications", new KIcon("make-kdevelop"), i18n("Applications"), "", panelSections));

        // Search area
        layoutSearch = new Plasma::NodeLayout();
        panelSearch = new Lancelot::Panel("panelSearch");

        // editSearch = new Plasma::LineEdit(panelSearch); // Upgrading to KLineEdit...
        editSearch = new KLineEdit(object);
        editSearchPositioner = new Lancelot::WidgetPositioner(editSearch, m_view, panelSearch);

        labelSearch = new Plasma::Label(panelSearch);

        // Main area
        layoutCenter = new Lancelot::CardLayout();

        sectionPanels.insert("Applications", panelSectionApplications =
            new Lancelot::PassagewayView("panelSectionApplications"));
        sectionPanels.insert("Contacts", panelSectionContacts =
            new Lancelot::Panel("panelSectionContacts", new KIcon("kontact"), i18n("Contacts")));
        sectionPanels.insert("Documents", panelSectionDocuments =
            new Lancelot::Panel("panelSectionDocuments", new KIcon("applications-office"), i18n("Documents")));
        sectionPanels.insert("Computer", panelSectionSystem =
            new Lancelot::Panel("panelSectionSystem", new KIcon("video-display"), i18n("Computer")));
        sectionPanels.insert("Search", panelSectionSearch =
            new Lancelot::Panel("panelSectionSearch", new KIcon("find"), i18n("Search")));

        // Center area :: Sections
        CreateSection (Search);
        CreateSection (System);
        CreateSection (Documents);
        CreateSection (Contacts);
    }

    void setupGroups(QFrame * object)
    {
        Q_UNUSED(object);

        foreach (Lancelot::ExtenderButton * button, systemButtons) {
            button->setGroupByName("SystemButtons");
        }

        foreach (Lancelot::ToggleExtenderButton * button, sectionButtons) {
            button->setGroupByName("SectionButtons");
        }
    }

    void setupObjects(QFrame * object)
    {
        Q_UNUSED(object);

        layoutMain->setSpacing(4);
        layoutMain->setMargin(15);

        // System area
        layoutMain->addItem(layoutSystem, Plasma::BottomPositioned);
        layoutMain->setSize(40.0, Plasma::BottomPositioned);
        layoutMain->setSize(30.0, Plasma::TopPositioned);
        layoutMain->setSize(128.0, Plasma::LeftPositioned);

        layoutSystem->setSpacing(0);
        layoutSystem->setMargin(6);
        layoutSystem->setSize(128.0, Plasma::LeftPositioned);
        layoutSystem->setSize(0.0,  Plasma::BottomPositioned);
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
            m_corona->addItem(button);
        }

        // Sections area
        layoutMain->addItem(panelSections, Plasma::LeftPositioned);
        layoutSections->setSpacing(0);
        layoutSections->setMargin(0);

        foreach (Lancelot::ToggleExtenderButton * button, sectionButtons) {
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

        layoutSearch->addItem(editSearchPositioner,
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

        delete object->layout();

        editSearch->setClearButtonShown(true);
        editSearch->setFrame(false);

        // Center area
        layoutCenter->setMargin(0);

        //layoutCenter->addItem(testContainment);
        //m_corona->addItem(testContainment);
        testContainment->setFormFactor(Plasma::Planar);
        testContainment->setLocation(Plasma::Desktop);
        testContainment->addApplet("digital-clock");
        testContainment->setGeometry(QRectF(0, 0, 500, 500));

        Plasma::Applet * applet = Plasma::Applet::loadApplet("digital-clock");
        //m_corona->addItem(applet);
        applet->setParentItem(panelSectionContacts);
        applet->setDrawStandardBackground(false);
        applet->setGeometry(QRectF(120, 100, 120, 70));

        applet = Plasma::Applet::loadApplet("clock");
        //m_corona->addItem(applet);
        applet->setParentItem(panelSectionContacts);
        applet->setDrawStandardBackground(false);
        applet->setGeometry(QRectF(0, 0, 200, 200));

        applet = Plasma::Applet::loadApplet("battery");
        //m_corona->addItem(applet);
        applet->setParentItem(panelSectionContacts);
        applet->setDrawStandardBackground(false);
        applet->setGeometry(QRectF(300, 100, 120, 70));

        QMapIterator<QString, Lancelot::Panel *> i(sectionPanels);
        while (i.hasNext()) {
            i.next();
            layoutCenter->addItem(i.value(), i.key());
            m_corona->addItem(i.value());
        }
        layoutMain->addItem(layoutCenter, Plasma::CenterPositioned);

        SetupSection (Search);
        SetupSection (System);
        SetupSection (Documents);
        SetupSection (Contacts);
    }

    void setupShell(QFrame * object) {
        object->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);// | Qt::Popup);
        KWindowSystem::setState(object->winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove);

        m_corona = new Plasma::Corona(object);
        m_layout = new QVBoxLayout(object);
        m_layout->setContentsMargins (0, 0, 0, 0);
        object->setLayout(m_layout);

        m_view = new CustomGraphicsView(m_corona, object);
        m_view->setWindowFlags(Qt::FramelessWindowHint);
        m_view->setFrameStyle(QFrame::NoFrame);
        m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        m_view->m_background = new Plasma::Svg("lancelot/theme");
        m_view->m_background->setContentType(Plasma::Svg::ImageSet);

        m_layout->addWidget(m_view);
    }
};

} // namespace Ui

#endif /*UI_LANCELOTWINDOW_H_*/
