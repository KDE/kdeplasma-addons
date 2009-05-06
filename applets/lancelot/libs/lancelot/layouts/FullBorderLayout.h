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

#ifndef LANCELOT_FULL_BORDER_LAYOUT_H
#define LANCELOT_FULL_BORDER_LAYOUT_H

#include <lancelot/lancelot_export.h>

#include <QGraphicsLayout>

namespace Lancelot {

/**
 * A modified version of Plasma::BorderLayout
 *
 * A layout which lays one item per border (left, top, bottom, right)
 * one per corner and one item in center.
 *
 * @author Ivan Cukic
 *
 */
class LANCELOT_EXPORT FullBorderLayout : public QGraphicsLayout {
public:
    /**
     * Borders enum
     */
    enum Border {
        TopBorder      = 1, /** Top border */
        BottomBorder   = 2, /** Bottom border */
        LeftBorder     = 4, /** Left border */
        RightBorder    = 8  /** Right border */
    };

    /**
     * Positions supported by FullBorderLayout
     */
    enum Place {
        Center      = 0,
        Top         = TopBorder,
        Bottom      = BottomBorder,
        Left        = LeftBorder,
        Right       = RightBorder,
        TopLeft     = TopBorder | LeftBorder,
        TopRight    = TopBorder | RightBorder,
        BottomLeft  = BottomBorder | LeftBorder,
        BottomRight = BottomBorder | RightBorder
    };

    /**
     * Creates a new Lancelot::FullBorderLayout
     * @param parent parent layout item
     */
    explicit FullBorderLayout(QGraphicsLayoutItem * parent = 0);

    /**
     * Destroys this Lancelot::FullBorderLayout
     */
    virtual ~FullBorderLayout();

    /**
     * Adds item in the center.
     * Equal to: addItem(item, Center);
     * @param item item to add
     */
    void addItem(QGraphicsLayoutItem * item);

    /**
     * Adds item at the specified position
     * @param item item to add
     * @param position position to which to add
     */
    void addItem(QGraphicsLayoutItem * item, Place position);

    /**
     * Deactivates the automatic sizing of a border widget,
     * and sets it to the specified size.
     *
     * For left and right widgets, it sets the width; while
     * for top and bottom ones, it sets the height.
     *
     * @param size size of the border
     * @param border border for which the size is being specified
     */
    void setSize(qreal size, Border border);

    /**
     * Activates the automatic sizing of a border widget,
     * according to it's sizeHint()
     * @param border border for which the auto size is being specified
     */
    void setAutoSize(Border border);

    /**
     * Returns the size of the specified border widget.
     *
     * If automatic sizing for that border widget is activated,
     * it will return a value less than zero.
     *
     * @param border border for which the size is requested
     */
    qreal size(Border border) const;

    L_Override void setGeometry(const QRectF & rect);
    L_Override int count() const;
    L_Override QGraphicsLayoutItem * itemAt(int i) const;
    L_Override void removeAt(int index);
    L_Override QSizeF sizeHint(Qt::SizeHint which,
            const QSizeF & constraint = QSizeF()) const;

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_FULL_BORDER_LAYOUT_H */

