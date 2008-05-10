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
#include "../widgets/Panel.h"
#include "../widgets/ResizeBordersPanel.h"
#include "../widgets/ScrollBar.h"
#include "../widgets/ScrollPane.h"

#include "../layouts/FullBorderLayout.h"
#include "../layouts/FlipLayout.h"
#include "../layouts/NodeLayout.h"
#include "../layouts/CardLayout.h"

using namespace Lancelot;

LancelotTestWindow::LancelotTestWindow()
    : QGraphicsView()
{

    // Initialization
    setFocusPolicy(Qt::WheelFocus);
    m_corona = new Plasma::Corona(this);
    setScene(m_corona);
    instance = new Instance();

    // Test area - begin ####################################
    Lancelot::FlipLayout < Lancelot::FullBorderLayout > * mainLayout;
    mainLayout = new Lancelot::FlipLayout < Lancelot::FullBorderLayout >();
    mainLayout->setFlip(Plasma::HorizontalFlip);

    Lancelot::ExtenderButton * button;

    button = new Lancelot::ExtenderButton("TL");
    button->setGroupByName("SystemButtons");
    button->setCheckable(true);
    mainLayout->addItem(button, Lancelot::FullBorderLayout::TopLeft);
    m_corona->addItem(button);

    button = new Lancelot::ExtenderButton("BR");
    button->setGroupByName("SystemButtons");
    button->setCheckable(true);
    button->setChecked(true);
    mainLayout->addItem(button, Lancelot::FullBorderLayout::BottomRight);
    m_corona->addItem(button);

    Lancelot::NodeLayout * centerLayout;
    centerLayout = new Lancelot::NodeLayout();
    mainLayout->addItem(centerLayout, Lancelot::FullBorderLayout::Center);

    button = new Lancelot::ExtenderButton("ND1");
    button->setGroupByName("SystemButtons");
    centerLayout->addItem(button,
            Lancelot::NodeLayout::NodeCoordinate(),
            Lancelot::NodeLayout::NodeCoordinate(0.5, 0.5)
            );
    m_corona->addItem(button);

    button = new Lancelot::ExtenderButton("ND2");
    button->setGroupByName("SystemButtons");
    centerLayout->addItem(button,
            Lancelot::NodeLayout::NodeCoordinate(0.5, 0.5),
            Lancelot::NodeLayout::NodeCoordinate(1, 1)
            );
    m_corona->addItem(button);

    // Test area - end   ####################################

    // Starting...
    mainLayout->setGeometry(QRectF(8, 8, 400, 300));
    instance->activateAll();
}

LancelotTestWindow::~LancelotTestWindow()
{
}

#include "LancelotTestWindow.moc"

