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

#include "../widgets/Widget.h"
#include "../widgets/ExtenderButton.h"

LancelotTestWindow::LancelotTestWindow()
    : QGraphicsView()
{

    setFocusPolicy(Qt::WheelFocus);

    m_corona = new Plasma::Corona(this);
    setScene(m_corona);
    setSceneRect(QRectF(0, 0, size().width(), size().height()));

    instance = new Lancelot::Instance();

    Lancelot::ExtenderButton * widget;

    // widget->setGroupByName("SystemButtons");
    widget = new Lancelot::ExtenderButton("name", KIcon("lancelot"), "Title", "Description");
    widget->setExtenderPosition(Lancelot::LeftExtender);
    m_corona->addItem(widget);
    widget->setGeometry(50, 50, 150, 50);

    widget = new Lancelot::ExtenderButton("name", KIcon("lancelot"), "Title", "Description");
    widget->setExtenderPosition(Lancelot::BottomExtender);
    m_corona->addItem(widget);
    widget->setGeometry(50, 150, 150, 50);

    widget = new Lancelot::ExtenderButton("name", KIcon("lancelot"), "Title", "Description");
    widget->setExtenderPosition(Lancelot::RightExtender);
    m_corona->addItem(widget);
    widget->setGeometry(250, 50, 150, 50);

    widget = new Lancelot::ExtenderButton("name", KIcon("lancelot"), "Title", "Description");
    widget->setExtenderPosition(Lancelot::TopExtender);
    m_corona->addItem(widget);
    widget->setGeometry(250, 150, 150, 50);



    instance->activateAll();
}

LancelotTestWindow::~LancelotTestWindow()
{
}

#include "LancelotTestWindow.moc"

