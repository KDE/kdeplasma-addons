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
#include "../widgets/ScrollBar.h"
#include "../widgets/ScrollPane.h"

#include "../layouts/FullBorderLayout.h"

using namespace Lancelot;

LancelotTestWindow::LancelotTestWindow()
    : QGraphicsView()
{

    setFocusPolicy(Qt::WheelFocus);
    //setDragMode(QGraphicsView::ScrollHandDrag);

    m_corona = new Plasma::Corona(this);
    setScene(m_corona);

    instance = new Instance();

    ExtenderButton * extenderButton;

    // ExtenderButton
    extenderButton = new ExtenderButton(KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(LeftExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(50, 50, 150, 50);
    extenderButton->setGroupByName("SystemButtons");
    extenderButton->setEnabled(false);
    //extenderButton->BasicWidget::debug();
    kDebug() << "sizeHint " << extenderButton->minimumSize();
    //extenderButton->updateGeometry();
    kDebug() << "sizeHint " << extenderButton->minimumSize();

    extenderButton = new ExtenderButton(KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(BottomExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(50, 150, 150, 50);
    extenderButton->setGroupByName("SystemButtons");
    extenderButton->setCheckable(true);
    extenderButton->setChecked(true);

    extenderButton = new ExtenderButton(KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(RightExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(250, 50, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    extenderButton = new ExtenderButton(KIcon("lancelot"), "Title", "Description");
    extenderButton->setExtenderPosition(TopExtender);
    m_corona->addItem(extenderButton);
    extenderButton->setGeometry(250, 150, 150, 50);
    extenderButton->setGroupByName("SystemButtons");

    // Panel
    Lancelot::Panel * panel;
    panel = new Lancelot::Panel(KIcon("lancelot"), "Title");
    m_corona->addItem(panel);
    panel->setGeometry(50, 250, 200, 200);

    // // ResizeBordersPanel
    // ResizeBordersPanel * resizeBordersPanel;
    // resizeBordersPanel = new ResizeBordersPanel("namePanel");
    // m_corona->addItem(resizeBordersPanel);
    // resizeBordersPanel->setBackground("lancelot/main-background");

    // resizeBordersPanel->setGeometry(300, 250, 200, 200);

    // ScrollPane
    ScrollPane * scrollPane = new ScrollPane();
    // Widget * scrollPane = new Widget();
    m_corona->addItem(scrollPane);
    // scrollPane->setGeometry(300, 250, 200, 200);
    scrollPane->setGroupByName("SystemButtons");
    scrollPane->setGeometry(400, 0, 200, 200);

    instance->activateAll();
    scrollPane->setGeometry(400, 0, 250, 250);
    scrollPane->update();
    kDebug() << "########################## " << extenderButton->geometry();

    // timeLine->start();
}

LancelotTestWindow::~LancelotTestWindow()
{
}

#include "LancelotTestWindow.moc"

