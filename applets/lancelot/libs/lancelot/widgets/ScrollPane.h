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

#ifndef LANCELOT_SCROLL_PANE_H_
#define LANCELOT_SCROLL_PANE_H_

#include <lancelot/lancelot_export.h>

#include "Panel.h"

namespace Lancelot
{

class ScrollPane;

/**
 * Interface for items that support scrolling
 */
class LANCELOT_EXPORT Scrollable {
public:
    /**
     * Creates a new Lancelot::Scrollable
     */
    Scrollable();

    /**
     * Destroys this Lancelot::Scrollable
     */
    virtual ~Scrollable();

    /**
     * @returns the size of the Scrollable widget
     */
    virtual QSizeF fullSize() const = 0;

    /**
     * Sets the currently displayed area.
     * This function tells the Scrollable widget which
     * part of it is shown after scrolling.
     * \note
     *   The size of the scrolling viewport doesn't need to
     *   be equal to ScrollPane::viewportSize()
     *
     */
    virtual void viewportChanged(QRectF viewport) = 0;

    /**
     * @param direction direction for which the scroll unit
     *                  size is needed
     * @returns scroll unit size
     *
     */
    virtual qreal scrollUnit(Qt::Orientation direction) = 0;

    /**
     * Sets the scroll pane that contains this Scrollable
     * widget
     */
    virtual void setScrollPane(ScrollPane * pane);

    /**
     * @returns scroll pane that contains this Scrollable widget
     */
    virtual ScrollPane * scrollPane() const;

private:
    class Private;
    Private * d;
};

/**
 * Pane for containing scrollable items
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT ScrollPane: public Widget {
public:
    /**
     * Creates a new Lancelot::ScrollPane
     * @param name the internal name of the widget
     * @param parent parent item
     */
    ScrollPane(QString name, QGraphicsItem * parent = 0);

    /**
     * Destroys this ScrollPane
     */
    virtual ~ScrollPane();

    /**
     * Sets the widget that should be contained by this
     * ScrollPane
     * @param widget widget implementing the Scrollable interface
     */
    virtual void setScrollableWidget(Scrollable * widget);

    /**
     * @returns the maximum size that this ScrollPane widget
     * can provide without scrolling.
     */
    virtual QSizeF maximumViewportSize() const;

    /**
     * @returns the current size of this ScrollPane widget.
     *   It equals maximumViewportSize() minus the sizes
     *   of the scrolling controls
     */
    virtual QSizeF currentViewportSize() const;

    Override virtual void setGeometry(qreal x, qreal y, qreal w, qreal h);
    Override virtual void setGeometry(const QRectF & geometry);

private:
    class Private;
    Private * d;
};

} // namespace Lancelot

#endif // LANCELOT_SCROLL_PANE_H_

