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
#include <plasma/widgets/label.h>

#include <plasma/layouts/borderlayout.h>
#include <plasma/layouts/nodelayout.h>
#include <plasma/layouts/fliplayout.h>
//#include <plasma/widgets/boxlayout.h>

#include <plasma/containment.h>

#include "WidgetPositioner.h"
#include "ExtenderButton.h"
#include "ToggleExtenderButton.h"
#include "ActionListView.h"
#include "ActionListViewModels.h"
#include "Panel.h"
#include "PassagewayView.h"
#include "CardLayout.h"
//#include "FlipLayout.h"
#include "Global.h"

#define SYSTEM_BUTTONS_Z_VALUE 1

// For compile time checking
#define sectApplications "Applications"
#define sectComputer     "Computer"
#define sectContacts     "Contacts"
#define sectDocuments    "Documents"
#define sectSearch       "Search"

#define sectionsWidth 128
#define mainWidth 422

#define AddSectionData(Name, Icon, Caption) \
    sectionsOrder << Name; \
    sectionsData[Name] = QPair < QString, QString > (Icon, Caption);

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
            painter->setCompositionMode(QPainter::CompositionMode_Clear);
            painter->fillRect(QRectF(rect.x()-2,rect.y()-2,rect.width()+2,rect.height()+2).toRect(), Qt::transparent);
            painter->setCompositionMode(QPainter::CompositionMode_Source);
            if (m_background) {
                //m_background->resize(QSizeF(mainWidth + sectionsWidth, 500));
                m_background->resize(size());
                m_background->paint(painter, 0, 0, "background");
            }
        }

    private:
        Plasma::Svg * m_background;
        friend class LancelotWindow;
    };

    QStringList sectionsOrder;
    QMap < QString, QPair < QString, QString > > sectionsData;

    Lancelot::Instance * instance;

    Plasma::Svg * m_mainTheme;
    CustomGraphicsView * m_view;
    Plasma::Corona     * m_corona;
    QVBoxLayout        * m_layout;

    //Plasma::Containment * testContainment;

    // Components
    Plasma::FlipLayout < Plasma::BorderLayout > * layoutMain;

    // System area
    Plasma::FlipLayout < Plasma::BorderLayout > * layoutSystem;
    Plasma::NodeLayout * layoutSystemButtons;

    QList < Lancelot::ExtenderButton * > systemButtons;
    Lancelot::ExtenderButton * buttonSystemLockScreen;
    Lancelot::ExtenderButton * buttonSystemLogout;
    Lancelot::ExtenderButton * buttonSystemSwitchUser;

    // Search area
    Plasma::NodeLayout * layoutSearch;
    Lancelot::Panel * panelSearch;
    Plasma::Label * labelSearch;

    //Plasma::LineEdit * editSearch;
    Lancelot::WidgetPositioner * editSearchPositioner;
    KLineEdit * editSearch;

    // Sections area
    Plasma::FlipLayout < Plasma::NodeLayout > * layoutSections; // was VBoxLaoyut
    Lancelot::Panel * panelSections;

    QMap < QString, Lancelot::ToggleExtenderButton * > sectionButtons;
    QMap < QString, Lancelot::Panel * > sectionPanels;
    QMap < QString, Plasma::NodeLayout * > sectionLayouts;

    QMap < QString, Lancelot::ActionListView * > sectionListsLeft;   // these have no Application key
    QMap < QString, Lancelot::ActionListView * > sectionListsRight;  // these have no Application key

    // Center area

    Lancelot::Widget * centerBackground;
    Lancelot::CardLayout * layoutCenter;
    Lancelot::PassagewayView * panelSectionApplications;

    void setupTests(QFrame * object)
    {
        Q_UNUSED(object);
    }

    void setupUi(QFrame * object)
    {
        // First of all we MUST create a Lancelot::Instance
        instance = new Lancelot::Instance();

        AddSectionData(sectApplications, "applications-other",  i18n("Applications"));
        AddSectionData(sectComputer,     "computer-laptop",     i18n("Computer"));
        AddSectionData(sectContacts,     "kontact",             i18n("Contacts"));
        AddSectionData(sectDocuments,    "applications-office", i18n("Documents"));
        AddSectionData(sectSearch,       "edit-find",           i18n("Search"));

        setupShell(object);
        createObjects(object);
        setupObjects(object);
        setupGroups(object);
        setupTests(object);

        instance->activateAll();

        m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        resizeWindow(object, QSize(mainWidth + sectionsWidth, 500));
    }

    void resizeWindow(QFrame * object, QSize newSize)
    {
        m_view->resetCachedContent();

        object->resize(newSize.width(), newSize.height());
        m_view->resize(newSize.width(), newSize.height());

        m_corona->setSceneRect(QRectF(0, 0, newSize.width(), newSize.height()));
        layoutMain->setGeometry(QRectF(0, 0, newSize.width(), newSize.height()));
        layoutMain->updateGeometry();

        m_view->invalidateScene();
        m_view->update();

        object->update();
    }

    void createObjects(QFrame * object)
    {
        Q_UNUSED(object);

        //testContainment = new Plasma::Containment();

        // Components
        layoutMain = new Plasma::FlipLayout < Plasma::BorderLayout > ();
        layoutMain->setFlip(Plasma::HorizontalFlip | Plasma::VerticalFlip);

        // System area
        layoutSystem = new Plasma::FlipLayout < Plasma::BorderLayout >();
        layoutSystem->setFlip(Plasma::HorizontalFlip | Plasma::VerticalFlip);
        layoutSystemButtons = new Plasma::NodeLayout();

        systemButtons.append(buttonSystemLockScreen =
            new Lancelot::ExtenderButton("buttonSystemLockScreen", new KIcon("system-lock-screen"), "Lock Session", ""));
        systemButtons.append(buttonSystemLogout =
            new Lancelot::ExtenderButton("buttonSystemLogout", new KIcon("system-log-out"), "Log Out...", ""));
        systemButtons.append(buttonSystemSwitchUser =
            new Lancelot::ExtenderButton("buttonSystemSwitchUser", new KIcon("switchuser"), "Switch User", ""));

        // Sections area
        layoutSections = new Plasma::FlipLayout < Plasma::NodeLayout > ();
        layoutSections->setFlip(Plasma::HorizontalFlip | Plasma::VerticalFlip);
        panelSections = new Lancelot::Panel("panelSections");

        layoutCenter = new Lancelot::CardLayout();
        centerBackground = new Lancelot::Widget("centerBackground");

        foreach (QString section, sectionsOrder) {
            sectionLayouts[section] = new Plasma::NodeLayout();
            if (section != sectSearch) {
                sectionButtons[section] = new Lancelot::ToggleExtenderButton(
                    "buttonSection" + section,
                    new KIcon(sectionsData[section].first),
                    sectionsData[section].second,
                    "", panelSections
                );
            }
            if (section != sectApplications) {
                sectionPanels[section] = new Lancelot::Panel(
                    "panelSection" + section,
                    new KIcon(sectionsData[section].first),
                    sectionsData[section].second
                );
                sectionListsLeft[section]  = new Lancelot::ActionListView("listSection" + section + "Left", NULL,  sectionPanels[section]);
                sectionListsRight[section] = new Lancelot::ActionListView("listSection" + section + "Right", NULL, sectionPanels[section]);
            } else {
                sectionPanels[section] = new Lancelot::PassagewayView("panelSectionApplications");
            }
        }

        // Search area
        layoutSearch = new Plasma::NodeLayout();
        panelSearch = new Lancelot::Panel("panelSearch");

        // editSearch = new Plasma::LineEdit(panelSearch); // Upgrading to KLineEdit...
        editSearch = new KLineEdit(object);
        editSearchPositioner = new Lancelot::WidgetPositioner(editSearch, m_view, panelSearch);

        labelSearch = new Plasma::Label(panelSearch);

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
        layoutMain->setSize(sectionsWidth, Plasma::LeftPositioned);

        layoutSystem->setSpacing(0);
        layoutSystem->setMargin(6);
        layoutSystem->setSize(128.0, Plasma::LeftPositioned);
        //layoutSystem->setSize(0.0,  Plasma::BottomPositioned);
        //layoutSystem->setSize(0.0,  Plasma::TopPositioned);
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

        qreal top = 1.0;
        qreal increase = 1.0 / sectionButtons.size();

        foreach (Lancelot::ToggleExtenderButton * button, sectionButtons) {
            button->setActivationMethod(Lancelot::ExtenderButton::Hover);
            button->setIconSize(QSize(48, 48));
            button->setZValue(1);
            button->setInnerOrientation(Lancelot::BaseActionWidget::Vertical);

            layoutSections->addItem(button,
                Plasma::NodeLayout::NodeCoordinate(0, top - increase),
                Plasma::NodeLayout::NodeCoordinate(1.0, top)
            );
            top -= increase;
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
        //testContainment->setFormFactor(Plasma::Planar);
        //testContainment->setLocation(Plasma::Desktop);
        //testContainment->addApplet("digital-clock");
        //testContainment->setGeometry(QRectF(0, 0, 500, 500));

        /*Plasma::Applet * applet = Plasma::Applet::loadApplet("digital-clock");
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
        applet->setGeometry(QRectF(300, 100, 120, 70));*/

        layoutCenter->addItem(centerBackground);
        m_corona->addItem(centerBackground);
        centerBackground->setGroupByName("MainPanel");
        centerBackground->disable();

        QMapIterator<QString, Lancelot::Panel *> i(sectionPanels);
        while (i.hasNext()) {
            i.next();
            layoutCenter->addItem(i.value(), i.key());
            m_corona->addItem(i.value());
        }
        layoutMain->addItem(layoutCenter, Plasma::CenterPositioned);

        foreach (QString section, sectionsOrder) {
            if (section == sectApplications) continue;
            sectionLayouts[section]->addItem(
                sectionListsLeft[section],
                Plasma::NodeLayout::NodeCoordinate(0, 0, 0, 0),
                Plasma::NodeLayout::NodeCoordinate(0.5, 1.0, -4, 0)
            );
            sectionLayouts[section]->addItem(
                sectionListsRight[section],
                Plasma::NodeLayout::NodeCoordinate(0.5, 0, 4, 0),
                Plasma::NodeLayout::NodeCoordinate(1.0, 1.0, 0, 0)
            );

            sectionListsLeft[section]->setItemsGroup(instance->group("ActionListView-Left-Items"));
            sectionListsLeft[section]->setExtenderPosition(Lancelot::ExtenderButton::Left);

            sectionListsRight[section]->setItemsGroup(instance->group("ActionListView-Right-Items"));
            sectionListsRight[section]->setExtenderPosition(Lancelot::ExtenderButton::Right);

            sectionPanels[section]->setLayout(sectionLayouts[section]);
        }
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
