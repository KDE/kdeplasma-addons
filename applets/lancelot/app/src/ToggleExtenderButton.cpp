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

#include "ToggleExtenderButton.h"
#include <KDebug>

namespace Lancelot
{

ToggleExtenderButton::ToggleExtenderButton(QString name, QString title, QString description, QGraphicsItem * parent)
  : ExtenderButton(name, title, description, parent), m_pressed(false)
{
    init();
}

ToggleExtenderButton::ToggleExtenderButton(QString name, QIcon * icon, QString title, QString description, QGraphicsItem * parent)
  : ExtenderButton(name, icon, title, description, parent), m_pressed(false)
{
    init();
}

ToggleExtenderButton::ToggleExtenderButton(QString name, Plasma::Svg * icon, QString title, QString description, QGraphicsItem * parent)
  : ExtenderButton(name, icon, title, description, parent), m_pressed(false)
{
    init();
}

void ToggleExtenderButton::init()
{
    kDebug() << "Init\n";
    connect (this, SIGNAL(activated()), this, SLOT(toggle()));
}

bool ToggleExtenderButton::isPressed() const
{
    return m_pressed;
}

void ToggleExtenderButton::setPressed(bool pressed)
{
    if (pressed == m_pressed) return;

    emit toggled(m_pressed = pressed);
    update();
}

void ToggleExtenderButton::toggle()
{
    emit toggled(m_pressed = !m_pressed);
    update();
}

ToggleExtenderButton::~ToggleExtenderButton()
{
}

void ToggleExtenderButton::paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    paintBackground(painter);
    if (m_pressed) {
        paintBackground(painter, "pressed");
    }
    paintForeground(painter);
}


}

#include "ToggleExtenderButton.moc"
