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

#include "../layouts/FullBorderLayout.h"

using namespace Lancelot;

LancelotTestWindow::LancelotTestWindow()
    : QGraphicsView()
{

    setFocusPolicy(Qt::WheelFocus);
    setDragMode(QGraphicsView::ScrollHandDrag);

    m_corona = new Plasma::Corona(this);
    setScene(m_corona);

    instance = new Instance();

    ExtenderButton * extenderButton;

    // ExtenderButton
    extenderButton = new ExtenderButton("name1", KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(LeftExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(50, 50, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    extenderButton = new ExtenderButton("name2", KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(BottomExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(50, 150, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    extenderButton = new ExtenderButton("name3", KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(RightExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(250, 50, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    extenderButton = new ExtenderButton("name4", KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(TopExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(250, 150, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    // Panel
    Lancelot::Panel * panel;
    panel = new Lancelot::Panel("namePanel", KIcon("lancelot"), "Title");
    m_corona->addItem(panel);
    panel->setGeometry(50, 250, 200, 200);

    // ToggleExtenderButton
    ToggleExtenderButton * toggleExtenderButton;
    toggleExtenderButton = new ToggleExtenderButton("name5", KIcon("lancelot"), "Title", "Description", panel);
    toggleExtenderButton->setActivationMethod(ClickActivate);
    //m_corona->addItem(toggleExtenderButton);
    //toggleExtenderButton->setGeometry(50, 250, 150, 150);
    toggleExtenderButton->setGroupByName("SectionButtons");
    panel->setLayoutItem(toggleExtenderButton);

    // ResizeBordersPanel
    ResizeBordersPanel * resizeBordersPanel;
    resizeBordersPanel = new ResizeBordersPanel("namePanel");
    m_corona->addItem(resizeBordersPanel);
    resizeBordersPanel->setBackground("lancelot/main-background");

    resizeBordersPanel->setGeometry(300, 250, 200, 200);

    // FullBorderLayout
    Widget * w;

    FullBorderLayout * fbl = new FullBorderLayout();

    m_corona->addItem(w = new BasicWidget("", "Title", "Desc"));
    w->setGroupByName("SystemButtons");
    fbl->addItem(w);

    m_corona->addItem(w = new BasicWidget("", "T", ""));
    w->setGroupByName("SystemButtons");
    fbl->addItem(w, FullBorderLayout::Top);
    m_corona->addItem(w = new BasicWidget("", "TL", ""));
    w->setGroupByName("SystemButtons");
    fbl->addItem(w, FullBorderLayout::TopLeft);
    m_corona->addItem(w = new BasicWidget("", "TR", ""));
    w->setGroupByName("SystemButtons");
    fbl->addItem(w, FullBorderLayout::TopRight);


    m_corona->addItem(w = new BasicWidget("", "B", ""));
    w->setGroupByName("SystemButtons");
    fbl->addItem(w, FullBorderLayout::Bottom);
    m_corona->addItem(w = new BasicWidget("", "BL", ""));
    w->setGroupByName("SystemButtons");
    fbl->addItem(w, FullBorderLayout::BottomLeft);
    m_corona->addItem(w = new BasicWidget("", "BR", ""));
    w->setGroupByName("SystemButtons");
    fbl->addItem(w, FullBorderLayout::BottomRight);

    m_corona->addItem(w = new BasicWidget("", "L", ""));
    w->setGroupByName("SystemButtons");
    fbl->addItem(w, FullBorderLayout::Left);
    m_corona->addItem(w = new BasicWidget("", "R", ""));
    w->setGroupByName("SystemButtons");
    fbl->addItem(w, FullBorderLayout::Right);

    fbl->setSize(50, FullBorderLayout::LeftBorder);
    fbl->setSize(50, FullBorderLayout::TopBorder);
    fbl->setSize(50, FullBorderLayout::BottomBorder);
    fbl->setSize(50, FullBorderLayout::RightBorder);

    fbl->setGeometry(QRectF(600, 0, 400, 400));






    instance->activateAll();
    kDebug() << "########################## " << extenderButton->geometry();
}

LancelotTestWindow::~LancelotTestWindow()
{
}

#include "LancelotTestWindow.moc"

