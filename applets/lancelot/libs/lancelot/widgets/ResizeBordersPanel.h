/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef LANCELOT_RESIZE_BORDERS_PANEL_H
#define LANCELOT_RESIZE_BORDERS_PANEL_H

#include <lancelot/lancelot_export.h>
#include <lancelot/widgets/Panel.h>

namespace Lancelot
{

/**
 * Panel with border resizing notifications
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT ResizeBordersPanel: public Lancelot::Panel {

    // @puck L_WIDGET
    // @puck L_INCLUDE(lancelot/widgets/ResizeBordersPanel.h)

public:
    /**
     * Creates a new ResizeBordersPanel
     * @param parent parent item
     */
    ResizeBordersPanel(QGraphicsItem * parent = 0);

    /**
     * Destroys this Lancelot::ResizeBordersPanel
     */
    ~ResizeBordersPanel();

protected:
    L_Override void resizeEvent(QGraphicsSceneResizeEvent * event);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif // LANCELOT_RESIZE_BORDERS_PANEL_H

