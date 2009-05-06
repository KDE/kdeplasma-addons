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

#ifndef LANCELOT_SCROLL_PANE_H
#define LANCELOT_SCROLL_PANE_H

#include <lancelot/lancelot_export.h>

#include <lancelot/widgets/Panel.h>

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
     * @returns the desired size of the Scrollable widget
     * when the container provides it with a specified
     * viewport size
     * @arg viewportSize viewport size
     */
    virtual QSizeF sizeFor(QSizeF viewportSize) const = 0;

    /**
     * Sets the currently displayed area.
     * This function tells the Scrollable widget which
     * part of it is shown after scrolling.
     * @note
     *   The size of the scrolling viewport doesn't need to
     *   be equal to ScrollPane::viewportSize()
     * @param viewport new viewport rectangle
     */
    virtual void viewportChanged(QRectF viewport) = 0;

    /**
     * @param direction direction for which the scroll unit
     *                  size is needed
     * @returns scroll unit size
     */
    virtual qreal scrollUnit(Qt::Orientation direction) const = 0;

    /**
     * Sets the scroll pane that contains this Scrollable
     * widget
     * @param pane scroll pane
     */
    virtual void setScrollPane(ScrollPane * pane);

    /**
     * @returns scroll pane that contains this Scrollable widget
     */
    virtual ScrollPane * scrollPane() const;

private:
    class Private;
    Private * const d;
};

/**
 * Pane for containing scrollable items
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT ScrollPane: public Lancelot::Widget {
    Q_OBJECT

    L_WIDGET
    L_INCLUDE(lancelot/widgets/ScrollPane.h)

public:
    enum Flag {
        ClipScrollable = 1,
        HoverShowScrollbars = 2
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    /**
     * Creates a new Lancelot::ScrollPane
     * @param parent parent item
     */
    ScrollPane(QGraphicsItem * parent = 0);

    /**
     * Destroys this ScrollPane
     */
    virtual ~ScrollPane();

    /**
     * Sets the widget that should be contained by this
     * ScrollPane
     * @param widget widget implementing the Scrollable interface
     */
    void setScrollableWidget(Scrollable * widget);

    /**
     * @returns the maximum size that this ScrollPane widget
     * can provide without scrolling.
     */
    QSizeF maximumViewportSize() const;

    /**
     * @returns the current size of this ScrollPane widget.
     *   It equals maximumViewportSize() minus the sizes
     *   of the scrolling controls
     */
    QSizeF currentViewportSize() const;

    /**
     * Turns the specified flag on
     */
    void setFlag(Flag flag);

    /**
     * Turns the specified flag off
     */
    void clearFlag(Flag flag);

    /**
     * @returns active flags
     */
    Flags flags() const;

    /**
     * Sets all flags
     */
    void setFlags(Flags flags);

    /**
     * Flips the layout of the scrollbars
     */
    void setFlip(Plasma::Flip flip);

    /**
     * Ensures that the specified area is visible.
     * @param rect area
     */
    void scrollTo(QRectF rect);

protected:
    L_Override void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    L_Override void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    L_Override void wheelEvent(QGraphicsSceneWheelEvent * event);
    L_Override void resizeEvent(QGraphicsSceneResizeEvent * event);

public Q_SLOTS:
    /**
     * Call this slot when the scrollable widget updates
     * its side
     */
    void scrollableWidgetSizeUpdateNeeded();

    /**
     * Scrolls the view horizontally to the specified value
     */
    void scrollHorizontal(int value);

    /**
     * Scrolls the view vertically to the specified value
     */
    void scrollVertical(int value);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

Q_DECLARE_OPERATORS_FOR_FLAGS(Lancelot::ScrollPane::Flags)

#endif // LANCELOT_SCROLL_PANE_H

