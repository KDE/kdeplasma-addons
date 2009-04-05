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

    // L_WIDGET
    // L_INCLUDE(lancelot/widgets/ScrollBar.h lancelot/lancelot.h)

public:
    /**
     * Creates a new Lancelot::ScrollBar
     * @param parent parent item
     */
    ScrollBar(QGraphicsWidget * parent = 0);

    void setMinimum(int min);
    void setMaximum(int max);
    void setViewSize(int size);
    void setPageSize(int size);
    void setStepSize(int size);

    void wheelEvent(QGraphicsSceneWheelEvent * event);

    /**
     * Destroys this Lancelot::ScrollBar
     */
    ~ScrollBar();

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_SCROLL_BAR_H */
