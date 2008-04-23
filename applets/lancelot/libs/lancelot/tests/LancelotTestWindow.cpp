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

#include "LancelotTestWindow.h"

#include <KIcon>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <KDebug>

#include "../widgets/Widget.h"
#include "../widgets/ExtenderButton.h"
#include "../widgets/ToggleExtenderButton.h"
#include "../widgets/Panel.h"
#include "../widgets/ResizeBordersPanel.h"

LancelotTestWindow::LancelotTestWindow()
    : QGraphicsView()
{

    setFocusPolicy(Qt::WheelFocus);

    m_corona = new Plasma::Corona(this);
    setScene(m_corona);
    setSceneRect(QRectF(0, 0, size().width(), size().height()));

    instance = new Lancelot::Instance();

    Lancelot::ExtenderButton * extenderButton;

    // ExtenderButton
    extenderButton = new Lancelot::ExtenderButton("name1", KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(Lancelot::LeftExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(50, 50, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    extenderButton = new Lancelot::ExtenderButton("name2", KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(Lancelot::BottomExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(50, 150, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    extenderButton = new Lancelot::ExtenderButton("name3", KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(Lancelot::RightExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(250, 50, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    extenderButton = new Lancelot::ExtenderButton("name4", KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(Lancelot::TopExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(250, 150, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    // Panel
    Lancelot::Panel * panel;
    panel = new Lancelot::Panel("namePanel", KIcon("lancelot"), "Title");
    m_corona->addItem(panel);
    panel->setGeometry(50, 250, 200, 200);

    // ToggleExtenderButton
    Lancelot::ToggleExtenderButton * toggleExtenderButton;
    toggleExtenderButton = new Lancelot::ToggleExtenderButton("name5", KIcon("lancelot"), "Title", "Description", panel);
    toggleExtenderButton->setActivationMethod(Lancelot::ClickActivate);
    //m_corona->addItem(toggleExtenderButton);
    //toggleExtenderButton->setGeometry(50, 250, 150, 150);
    toggleExtenderButton->setGroupByName("SectionButtons");
    panel->setLayoutItem(toggleExtenderButton);

    // ResizeBordersPanel
    Lancelot::ResizeBordersPanel * resizeBordersPanel;
    resizeBordersPanel = new Lancelot::ResizeBordersPanel("namePanel");
    m_corona->addItem(resizeBordersPanel);
    resizeBordersPanel->setBackground("lancelot/main-background");

    resizeBordersPanel->setGeometry(300, 250, 200, 200);

    instance->activateAll();
    kDebug() << "########################## " << extenderButton->geometry();
}

LancelotTestWindow::~LancelotTestWindow()
{
}

#include "LancelotTestWindow.moc"

