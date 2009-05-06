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

#ifndef LANCELOT_SCROLL_BAR_H
#define LANCELOT_SCROLL_BAR_H

#include <lancelot/lancelot_export.h>
#include <lancelot/lancelot.h>

// #include <lancelot/widgets/Widget.h>
#include <plasma/widgets/scrollbar.h>

namespace Lancelot
{

/**
 * A widget implementing scroll bars
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT ScrollBar: public Plasma::ScrollBar {
    Q_OBJECT

    Q_PROPERTY ( ActivationMethod activationMethod READ activationMethod WRITE setActivationMethod )

    // L_WIDGET
    // L_INCLUDE(lancelot/widgets/ScrollBar.h lancelot/lancelot.h)

public:
    /**
     * Creates a new Lancelot::ScrollBar
     * @param parent parent item
     */
    ScrollBar(QGraphicsWidget * parent = 0);

    /**
     * Destroys this Lancelot::ScrollBar
     */
    ~ScrollBar();

    /**
     * Sets the activation method of the ExtenderButton.
     * If the activation method is ExtenderActivate, it will
     * be treated as HoverActivate
     * @param method new activation method
     */
    void setActivationMethod(Lancelot::ActivationMethod method);

    /**
     * @returns activation method
     */
    Lancelot::ActivationMethod activationMethod() const;

protected:
    L_Override void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    L_Override void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
    L_Override void wheelEvent(QGraphicsSceneWheelEvent * event);
    L_Override void timerEvent(QTimerEvent * event);

private:
    class Private;
    Private * const d;

    friend class ScrollPane;
};

} // namespace Lancelot

#endif /* LANCELOT_SCROLL_BAR_H */
