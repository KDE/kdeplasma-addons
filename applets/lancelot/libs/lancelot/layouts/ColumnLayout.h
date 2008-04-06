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

#ifndef COLUMNLAYOUT_H_
#define COLUMNLAYOUT_H_

#include "../lancelot_export.h"

#include <plasma/widgets/widget.h>
#include <plasma/layouts/layout.h>
#include <plasma/layouts/layoutanimator.h>

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
 *  functions from Plasma::Layout will do nothing.
 *
 *  Since ColumnLayout needs the ability to hide certain items
 *  in it, it works only for Plasma::Widget subclasses.
 */

class LANCELOT_EXPORT ColumnLayout: public Plasma::Layout
{
public:
    class ColumnSizer
    {
    public:
        virtual void init(int count)  = 0;
        virtual qreal size() = 0;
        virtual ~ColumnSizer();

        enum SizerType {
            EqualSizer,
            GoldenSizer
        };

        static ColumnSizer * create(SizerType type);
    };

    ColumnLayout(Plasma::LayoutItem * parent = 0);
    ~ColumnLayout();

    void insertItem(int index, Plasma::LayoutItem * item); /**< Not implemented. See class description. */
    void addItem(Plasma::LayoutItem * item); /**< Not implemented. See class description. */
    void removeItem(Plasma::LayoutItem * item); /**< Not implemented. See class description. */
    LayoutItem * takeAt(int i); /**< Not implemented. See class description. */

    int indexOf(Plasma::LayoutItem * item) const;
    LayoutItem * itemAt(int i) const;
    Qt::Orientations expandingDirections() const;

    int count() const;
    void setAnimator(Plasma::LayoutAnimator * animator);

    void setColumnCount(int count);
    int columnCount() const;

    void push(Plasma::Widget * widget);
    Plasma::Widget * pop();

    QSizeF sizeHint() const;

    void setSizer(ColumnSizer * sizer);
    ColumnSizer * sizer() const;

protected:
    void relayout();
    void releaseManagedItems();

private:
    class Private;
    Private * const d;
};

}

#endif /*COLUMNLAYOUT_H_*/
