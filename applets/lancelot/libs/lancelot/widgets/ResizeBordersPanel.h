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

#ifndef RESIZE_BORDERS_PANEL_H
#define RESIZE_BORDERS_PANEL_H

#include "../lancelot_export.h"

#include "Panel.h"

namespace Lancelot
{

/**
 * Panel with border resizing notifications
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT ResizeBordersPanel: public Panel {
public:
    /**
     * Creates a new ResizeBordersPanel
     * @param name the internal name of the widget
     * @param parent parent item
     */
    ResizeBordersPanel(QString name, QGraphicsItem * parent = 0);

    Override virtual void setGeometry(qreal x, qreal y, qreal w, qreal h);
    Override virtual void setGeometry(const QRectF & geometry);

private:
    class Private;
    Private * d;
};

} // namespace Lancelot

#endif // RESIZE_BORDERS_PANEL_H


