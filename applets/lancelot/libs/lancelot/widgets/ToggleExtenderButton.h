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

// TODO: Convert to dptr

#ifndef TOGGLEEXTENDERBUTTON_H_
#define TOGGLEEXTENDERBUTTON_H_

#include "../lancelot_export.h"

#include "ExtenderButton.h"

namespace Lancelot
{

class LANCELOT_EXPORT ToggleExtenderButton : public Lancelot::ExtenderButton {
    Q_OBJECT
public:
    ToggleExtenderButton(QString name = QString(), QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);
    ToggleExtenderButton(QString name, QIcon * icon, QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);
    ToggleExtenderButton(QString name, Plasma::Svg * icon, QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);

    virtual ~ToggleExtenderButton();

    bool isPressed() const;
    void setPressed(bool pressed = true);

    virtual void paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);


public slots:
    void toggle();

Q_SIGNALS:
    void toggled(bool pressed);

private:
    bool m_pressed;
    void init();

};

}

#endif /*TOGGLEEXTENDERBUTTON_H_*/
