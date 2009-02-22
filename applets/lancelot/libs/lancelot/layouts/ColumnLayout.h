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

#ifndef LANCELOT_COLUMN_LAYOUT_H
#define LANCELOT_COLUMN_LAYOUT_H

#include <lancelot/lancelot_export.h>
#include <QtGui/QGraphicsLayoutItem>
#include <QtGui/QGraphicsLayout>

namespace Lancelot
{

/**
 *  This layout is not a general purpose layout, and should not be
 *  used as such.
 *
 *  Column layout shows items in columns. It shows only n last items
 *  pushed into it. Items can be pushed and popped like on stack,
 *  inserting an item before the end of the list is not supported,
 *  and, likewise, removing items except the last one is not
 *  supported.
 *
 *  To make that clear, pop and push functions are introduced.
 *  Trying to invoke one of the standard item manipulation
 *  functions from QGraphicsLayout will do nothing.
 *
 *  Since ColumnLayout needs the ability to hide certain items
 *  in it, it works only for QGraphicsWidget subclasses.
 */

class LANCELOT_EXPORT ColumnLayout: public QGraphicsLayout
{
public:
    /**
     * Abstract interface for implementing column size calculation
     * depending on the column count.
     * Two basic implementations are provided - one that returns
     * equal sizes for all visible columns, and one that uses the
     * golden ratio.
     */
    class ColumnSizer {
    public:
        /**
         * Initializes the ColumnSizer.
         * This function should calculate or otherwise prepare
         * the data needed for retrieving the sizes of columns
         * depending on the column count. It should also reset
         * the current column counter to the first column.
         * @param count column count
         */
        virtual void init(int count) = 0;

        /**
         * @returns the size of the current column.
         * This function should return the size of the current
         * column and move the current column pointer to the
         * next column.
         */
        virtual qreal size() = 0;

        /**
         * Destroys the ColumnLayout::ColumnSizer
         */
        virtual ~ColumnSizer();

        enum SizerType {
            EqualSizer,  ///< Sizer that returns the same size for all visible columns
            GoldenSizer  ///< Sizer that uses golden ratio for visible column sizes
        };

        /**
         * @returns a new ColumnSizer with one of the default
         * implementations
         */
        static ColumnSizer * create(SizerType type);
    };

    /**
     * Creates a new Lancelot::ColumnLayout
     * @param parent parent layout item
     */
    ColumnLayout(QGraphicsLayoutItem * parent = 0);

    /**
     * Destroys Lancelot::ColumnLayout
     */
    ~ColumnLayout();

    /**
     * Sets the maximum number of visible columns
     */
    void setColumnCount(int count);

    /**
     * @returns the number of visible columns
     */
    int columnCount() const;

    /**
     * Pushes the specified widget to the end of stack
     */
    void push(QGraphicsWidget * widget);

    /**
     * Removes the last widget from the stack and returns it
     * @returns the last widget in stack
     */
    QGraphicsWidget * pop();

    /**
     * Sets the ColumnSizer object
     */
    void setSizer(ColumnSizer * sizer);

    /**
     * @returns the current ColumnSizer
     */
    ColumnSizer * sizer() const;

    L_Override void setGeometry(const QRectF & rect);
    L_Override int count() const;
    L_Override QGraphicsLayoutItem * itemAt(int i) const;
    L_Override QSizeF sizeHint(Qt::SizeHint which,
            const QSizeF & constraint = QSizeF()) const;

    L_Override void removeAt(int index);  /**< Not implemented. See class description. */

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_COLUMN_LAYOUT_H */

