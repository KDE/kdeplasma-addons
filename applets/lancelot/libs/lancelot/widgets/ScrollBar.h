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

#ifndef LANCELOT_SCROLL_BAR_H_
#define LANCELOT_SCROLL_BAR_H_

#include <lancelot/lancelot_export.h>
#include <lancelot/lancelot.h>

#include <QtGui>
#include <QtCore>
#include <lancelot/widgets/Widget.h>

namespace Lancelot
{

/**
 * A widget implementing scroll bars
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT ScrollBar: public Lancelot::Widget {
    Q_OBJECT

    Q_PROPERTY ( int minimum READ minimum WRITE setMinimum )
    Q_PROPERTY ( int maximum READ maximum WRITE setMaximum )
    Q_PROPERTY ( int value READ value WRITE setValue )
    Q_PROPERTY ( int viewSize READ viewSize WRITE setViewSize )
    Q_PROPERTY ( int pageSize READ pageSize WRITE setPageSize )
    Q_PROPERTY ( int stepSize READ stepSize WRITE setStepSize )
    Q_PROPERTY ( Qt::Orientation orientation READ orientation WRITE setOrientation )
    Q_PROPERTY ( ActivationMethod activationMethod READ activationMethod WRITE setActivationMethod )

    L_WIDGET
    L_INCLUDE(lancelot/widgets/ScrollBar.h lancelot/lancelot.h)

public:
    /**
     * Creates a new Lancelot::ScrollBar
     * @param parent parent item
     */
    ScrollBar(QGraphicsItem * parent = 0);

    /**
     * Destroys this Lancelot::ScrollBar
     */
    ~ScrollBar();

    /**
     * Sets the minimum scroll bar value
     * @param value new value
     */
    void setMinimum(int value);

    /**
     * @returns the minimum scroll bar value
     */
    int minimum() const;

    /**
     * Sets the maximum scroll bar value
     * @param value new value
     */
    void setMaximum(int value);

    /**
     * @returns the maximum scroll bar value
     */
    int maximum() const;

    /**
     * @returns the scrollbar position
     */
    int value() const;

    /**
     * Sets the size of the view on which the handle size depends
     * @param value new value
     */
    void setViewSize(int value);

    /**
     * @returns the size of the view on which the handle size depends
     */
    int viewSize() const;

    /**
     * Sets the size of one scrolling step
     * @param value new value
     */
    void setStepSize(int value);

    /**
     * @returns the size of one scrolling step
     */
    int stepSize() const;

    /**
     * Sets the size of one scrolling page
     * @param value new value
     */
    void setPageSize(int value);

    /**
     * @returns the size of one scrolling page
     */
    int pageSize() const;

    /**
     * Sets the scrollbar orientation
     * @param value new value
     */
    void setOrientation(Qt::Orientation value);

    /**
     * @returns the scrollbar orientation
     */
    Qt::Orientation orientation() const;

    /**
     * Sets the activation method
     * @param method new method
     * @note
     * ExtenderActivate is not supported and is treated as HoverActivate
     */
    void setActivationMethod(Lancelot::ActivationMethod method);

    /**
     * @returns activation method
     */
    Lancelot::ActivationMethod activationMethod() const;

    L_Override virtual void setGeometry(const QRectF & geometry);
    L_Override virtual void setGroup(WidgetGroup * group = NULL);

public Q_SLOTS:
    /**
     * Sets the scrollbar position
     * @param value new value
     */
    void setValue(int value);

    /**
     * Increases the value by one step.
     */
    void stepIncrease();

    /**
     * Decreases the value by one step.
     */
    void stepDecrease();

    /**
     * Increases the value by one page.
     */
    void pageIncrease();

    /**
     * Decrease the value by one page.
     */
    void pageDecrease();

Q_SIGNALS:
    /**
     * Signal that is emitted when the scrollbar position is changed
     * @param value new value
     */
    void valueChanged(int value);

protected:
    L_Override virtual void groupUpdated();
    L_Override virtual QSizeF sizeHint(Qt::SizeHint which,
            const QSizeF & constraint = QSizeF()) const;

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_SCROLL_BAR_H_ */
