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

class ToggleExtenderButton::Private {
public:
    Private()
      : pressed(false)
    {
    }

    bool pressed;
};

ToggleExtenderButton::ToggleExtenderButton(QString name, QString title,
        QString description, QGraphicsItem * parent)
  : ExtenderButton(name, title, description, parent),
    d(new Private())
{
    connect (this, SIGNAL(activated()), this, SLOT(toggle()));
}

ToggleExtenderButton::ToggleExtenderButton(QString name, QIcon icon,
        QString title, QString description, QGraphicsItem * parent)
  : ExtenderButton(name, icon, title, description, parent),
    d(new Private())
{
    connect (this, SIGNAL(activated()), this, SLOT(toggle()));
}

ToggleExtenderButton::ToggleExtenderButton(QString name, Plasma::Svg * icon,
        QString title, QString description, QGraphicsItem * parent)
  : ExtenderButton(name, icon, title, description, parent),
    d(new Private())
{
    connect (this, SIGNAL(activated()), this, SLOT(toggle()));
}

bool ToggleExtenderButton::isPressed() const
{
    return d->pressed;
}

void ToggleExtenderButton::setPressed(bool pressed)
{
    if (pressed == d->pressed) return;

    emit toggled(d->pressed = pressed);
    update();
}

void ToggleExtenderButton::toggle()
{
    emit toggled(d->pressed = !d->pressed);
    update();
}

ToggleExtenderButton::~ToggleExtenderButton()
{
}

void ToggleExtenderButton::paint(QPainter * painter,
        const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    paintBackground(painter);
    if (d->pressed) {
        paintBackground(painter, "pressed");
    }
    paintForeground(painter);
}

} // namespace Lancelot

#include "ToggleExtenderButton.moc"
