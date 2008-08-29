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
#include <KLineEdit>
#include <Plasma/LineEdit>

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
    QGraphicsWidget * m_root = new QGraphicsWidget();
    Lancelot::FullBorderLayout * m_layout = new Lancelot::FullBorderLayout();
    m_root->setLayout(m_layout);
    m_root->setGeometry(QRectF(0, 0, 400, 400));
    m_corona->addItem(m_root);

    Lancelot::ExtenderButton * eb = new Lancelot::ExtenderButton(m_root);
    m_layout->addItem(eb, Lancelot::FullBorderLayout::Center);

    Lancelot::ScrollBar * sbH = new Lancelot::ScrollBar(m_root);
    sbH->setMinimum(100);
    sbH->setMaximum(200);
    sbH->setViewSize(40);
    sbH->setValue(30);
    sbH->setOrientation(Qt::Horizontal);
    m_layout->addItem(sbH, Lancelot::FullBorderLayout::Top);

    Lancelot::ScrollBar * sbV = new Lancelot::ScrollBar(m_root);
    sbV->setMinimum(100);
    sbV->setMaximum(200);
    sbV->setViewSize(40);
    sbV->setValue(30);
    sbV->setOrientation(Qt::Horizontal);
    sbV->setOrientation(Qt::Vertical);
    m_layout->addItem(sbV, Lancelot::FullBorderLayout::Right);

    Plasma::LineEdit * le = new Plasma::LineEdit(m_root);
    m_layout->addItem(le, Lancelot::FullBorderLayout::Bottom);

    // Test area - end   ####################################

    // Starting...
    instance->activateAll();
}

LancelotTestWindow::~LancelotTestWindow()
{
}

#include "LancelotTestWindow.moc"

