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

#include "ScrollBar.h"
#include <KDebug>

#include "ExtenderButton.h"

namespace Lancelot
{


class ScrollBar::Private {
public:
    class ScrollBarItem: public BasicWidget {
    public:
        ScrollBarItem(QGraphicsItem * parent, ScrollBar::Private * parentd)
            : BasicWidget(parent), d(parentd)
        {
        }

    protected:
        L_Override virtual void groupUpdated()
        {
            d->setItemSizeHints();
        }

    protected:
        ScrollBar::Private * d;
    };

    class ScrollBarHandle: public ScrollBarItem {
    public:
        ScrollBarHandle(QGraphicsItem * parent, ScrollBar::Private * parentd)
            : ScrollBarItem(parent, parentd)
        {
        }

    protected:
        L_Override virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * e)
        {
            ScrollBarItem::mouseMoveEvent(e);
            QPointF orig = e->buttonDownPos(Qt::LeftButton);
            kDebug() << orig << e->pos();
            if (orig.isNull()) {
                return;
            }

            orig = e->pos() - orig;
            d->handleMoved(
                (d->orientation == Qt::Horizontal)?(orig.x()):(orig.y())
            );
        }

    };

    Private(ScrollBar * parent)
      : minimum(0),
        maximum(100),
        value(0),
        viewSize(10),
        stepSize(5),
        pageSize(10),
        handleSlideSize(0),
        orientation(Qt::Vertical),
        activationMethod(ExtenderActivate),
        validating(false),
        q(parent)
    {
        upButton   = new ScrollBarItem(parent, this);
        downButton = new ScrollBarItem(parent, this);
        upBar      = new ScrollBarItem(parent, this);
        downBar    = new ScrollBarItem(parent, this);
        handle     = new ScrollBarHandle(parent, this);

        upBar->setMinimumSize(QSizeF(0, 0));
        downBar->setMinimumSize(QSizeF(0, 0));

        connect(upBar, SIGNAL(clicked()), parent, SLOT(pageDecrease()));
        connect(downBar, SIGNAL(clicked()), parent, SLOT(pageIncrease()));

        upTimer.setInterval(100);
        upTimer.setSingleShot(false);
        connect(&upTimer, SIGNAL(timeout()), parent, SLOT(stepDecrease()));

        downTimer.setInterval(100);
        downTimer.setSingleShot(false);
        connect(&downTimer, SIGNAL(timeout()), parent, SLOT(stepIncrease()));

        upButton->setZValue(2);
        downButton->setZValue(2);
        handle->setZValue(1);

        // Icons
        if (!scrollbarIconUp.isValid()) {
            scrollbarIconUp.setImagePath("lancelot/scroll-bar-button-up-icon");
            scrollbarIconLeft.setImagePath("lancelot/scroll-bar-button-left-icon");
            scrollbarIconDown.setImagePath("lancelot/scroll-bar-button-down-icon");
            scrollbarIconRight.setImagePath("lancelot/scroll-bar-button-right-icon");

            upButton->setIconSize(QSize(8, 8));
            downButton->setIconSize(QSize(8, 8));

            upButton->setIconInSvg(scrollbarIconUp);
            downButton->setIconInSvg(scrollbarIconDown);
        }
    }

    ~Private()
    {
        delete upButton;
        delete upBar;
        delete downButton;
        delete downBar;
        delete handle;
    }

    /** Flips the source rect if the orientation is Horizontal */
    QRectF orientateRect(const QRectF & rect) {
        if (orientation == Qt::Vertical) {
            return rect;
        }

        QRectF result;
        result.setLeft(rect.top());
        result.setTop(rect.left());
        result.setWidth(rect.height());
        result.setHeight(rect.width());
        return result;
    }

    void invalidate()
    {
        if (validating) {
            return;
        }
        validating = true;

        setItemSizeHints();

        QRectF geometry = orientateRect(q->geometry());
        kDebug() << geometry;
        geometry.moveTopLeft(QPointF(0, 0));

        QRectF itemRect = geometry;

        // seting the up button's position
        itemRect.setHeight(
                (orientation == Qt::Horizontal) ?
                    (upButton->preferredSize().width()) :
                    (upButton->preferredSize().height())
                );
        upButton->setGeometry(orientateRect(itemRect));

        // seting the down button's position
        itemRect.setHeight(
                (orientation == Qt::Horizontal) ?
                    (downButton->preferredSize().width()) :
                    (downButton->preferredSize().height())
                );
        itemRect.moveBottom(geometry.bottom());
        downButton->setGeometry(orientateRect(itemRect));

        positionScroll();
        validating = false;
    }

    // This macro is used in setItemSizeHints function. It gets margins of a PanelSvg
    // that is used to paint the specified object. If Svg is NULL, margins are set to a fixed size.
    // Macro sets left, top, right and bottom variables that need to be defined as qreal before
    // invoking the macro
    #define GetMargins(object) \
        if (object && object->group() && object->group()->backgroundSvg()) { \
            object->group()->backgroundSvg()->getMargins(left, top, right, bottom); \
        } else { \
            left = top = right = bottom = 8; \
        }

    // This macro sets the minimum and preferred sizes of a specified object.
    // The variable size needs to be declared QSizeF before invoking this macro
    #define SetSizes(object) \
        object->setPreferredSize(size);

    // This macro increases preferredSize according to preset size
    // variable depending on the scrollbar orientation
    #define UpdatePreferredSize() \
        if (orientation == Qt::Horizontal) { \
            preferredSize.setWidth(preferredSize.width() + size.width()); \
            preferredSize.setHeight(qMax(preferredSize.height(), size.height())); \
        } else { \
            preferredSize.setHeight(preferredSize.height() + size.height()); \
            preferredSize.setWidth(qMax(preferredSize.width(), size.width())); \
        }

    void setItemSizeHints()
    {
        qreal left, top, right, bottom;
        preferredSize = QSizeF(0, 0);
        QSizeF size;

        GetMargins(upButton);
        size = QSizeF(left + right, top + bottom);
        SetSizes(upButton);
        UpdatePreferredSize();

        GetMargins(downButton);
        size = QSizeF(left + right, top + bottom);
        SetSizes(downButton);
        UpdatePreferredSize();

        GetMargins(upBar);
        size = QSizeF(
                ((orientation == Qt::Horizontal) ? (left + right) : 0),
                ((orientation == Qt::Vertical)   ? (top + bottom) : 0)
                );
        SetSizes(upBar);

        GetMargins(downBar);
        size = QSizeF(
                ((orientation == Qt::Horizontal) ? (left + right) : 0),
                ((orientation == Qt::Vertical)   ? (top + bottom) : 0)
                );
        SetSizes(downBar);

        GetMargins(handle);
        size = QSizeF(
                ((orientation == Qt::Horizontal) ? (left + right) : 0),
                ((orientation == Qt::Vertical)   ? (top + bottom) : 0)
                );
        SetSizes(handle);
        UpdatePreferredSize();

        q->updateGeometry();
        kDebug() << preferredSize;
    }

    void handleMoved(int ammount)
    {
        q->setValue(value + (ammount / (qreal)handleSlideSize)
                * (maximum - minimum)
                );
    }

    void positionScroll()
    {
        QRectF geometry = orientateRect(q->geometry());
        geometry.moveTopLeft(QPointF(0, 0));

        // TODO: This should be changed to load sizes from upButton and downButton
        geometry.setTop(geometry.top() +
                ((orientation == Qt::Horizontal) ?
                    (upButton->preferredSize().width()) :
                    (upButton->preferredSize().height())
                ));
        geometry.setBottom(geometry.bottom() -
                ((orientation == Qt::Horizontal) ?
                    (downButton->preferredSize().width()) :
                    (downButton->preferredSize().height())
                ));

        if (minimum >= maximum) {
            handle->setPreferredSize(orientateRect(geometry).size());
            handle->setGeometry(orientateRect(geometry));
            upBar->setGeometry(QRectF());
            downBar->setGeometry(QRectF());
            return;
        }

        qreal diff;

        qreal handleSize = geometry.height() * (viewSize / (qreal)(maximum - minimum + viewSize));
        diff = 2 * geometry.width();
        if (handleSize < diff) {
            handleSize = diff;
        }

        diff = geometry.height() - handleSize;
        handleSlideSize = diff;

        QRectF itemRect = geometry;
        QRectF itemExpandedRect;

        // up bar
        itemRect.setHeight(
                diff * ((value - minimum) / (qreal)(maximum - minimum))
                );
        itemExpandedRect = itemRect;
        itemExpandedRect.setHeight(itemExpandedRect.height() + 10);
        upBar->setPreferredSize(orientateRect(itemExpandedRect).size());
        upBar->setGeometry(orientateRect(itemExpandedRect));

        itemRect.setTop(itemRect.bottom());
        itemRect.setHeight(handleSize);
        handle->setPreferredSize(orientateRect(itemRect).size());
        handle->setGeometry(orientateRect(itemRect));

        itemRect.setTop(itemRect.bottom());
        itemRect.setBottom(geometry.bottom());
        itemExpandedRect = itemRect;
        itemExpandedRect.setTop(itemExpandedRect.top() - 10);
        downBar->setPreferredSize(orientateRect(itemExpandedRect).size());
        downBar->setGeometry(orientateRect(itemExpandedRect));
    }

    int minimum;
    int maximum;
    int value;
    int viewSize;
    int stepSize;
    int pageSize;
    int handleSlideSize;
    QSizeF preferredSize;

    Qt::Orientation orientation;
    ActivationMethod activationMethod;
    bool validating;

    BasicWidget * upButton;
    BasicWidget * downButton;
    BasicWidget * upBar;
    BasicWidget * downBar;
    BasicWidget * handle;

    QTimer downTimer, upTimer;

    ScrollBar * q;

    static Plasma::Svg scrollbarIconUp;
    static Plasma::Svg scrollbarIconLeft;
    static Plasma::Svg scrollbarIconDown;
    static Plasma::Svg scrollbarIconRight;
};

Plasma::Svg ScrollBar::Private::scrollbarIconUp;
Plasma::Svg ScrollBar::Private::scrollbarIconLeft;
Plasma::Svg ScrollBar::Private::scrollbarIconDown;
Plasma::Svg ScrollBar::Private::scrollbarIconRight;

ScrollBar::ScrollBar(QGraphicsItem * parent)
  : Widget(parent), d(new Private(this))
{
    setGroupByName("ScrollBar");

    // Initially, the method is set to ExtenderActivate
    // so the following method will not just exit,
    // but will set the necessary connections.
    setActivationMethod(HoverActivate);

    L_WIDGET_SET_INITIALIZED;
}

ScrollBar::~ScrollBar()
{
    delete d;
}

void ScrollBar::setMinimum(int value)
{
    if (d->minimum == value) {
        return;
    }

    d->minimum = value;
    if (d->value < d->minimum) {
        d->value = d->minimum;
    }
    if (d->maximum < d->minimum) {
        d->maximum = d->minimum;
    }
    d->positionScroll();
}

int ScrollBar::minimum() const
{
    return d->minimum;
}

void ScrollBar::setMaximum(int value)
{
    if (d->maximum == value) {
        return;
    }

    d->maximum = value;
    if (d->value > d->maximum) {
        d->value = d->maximum;
    }
    if (d->maximum < d->minimum) {
        d->minimum = d->maximum;
    }
    d->positionScroll();
}

int ScrollBar::maximum() const
{
    return d->maximum;
}

void ScrollBar::setValue(int value)
{
    if (value < d->minimum) {
        value = d->minimum;
    } else if (value > d->maximum) {
        value = d->maximum;
    }

    if (d->value == value) {
        return;
    }

    d->value = value;
    d->positionScroll();
    emit valueChanged(d->value);
}

int ScrollBar::value() const
{
    return d->value;
}

void ScrollBar::setViewSize(int value)
{
    if (d->viewSize == value) {
        return;
    }

    d->viewSize = value;
    if (d->viewSize > d->maximum - d->minimum) {
        d->viewSize = d->maximum - d->minimum;
    }

    d->positionScroll();
}

int ScrollBar::viewSize() const
{
    return d->viewSize;
}

void ScrollBar::setStepSize(int value)
{
    d->stepSize = value;
}

int ScrollBar::stepSize() const
{
    return d->stepSize;
}

void ScrollBar::setPageSize(int value)
{
    d->pageSize = value;
}

int ScrollBar::pageSize() const
{
    return d->pageSize;
}

void ScrollBar::setOrientation(Qt::Orientation value)
{
    d->orientation = value;

    if (value == Qt::Horizontal) {
        d->upButton->setIconInSvg  (d->scrollbarIconLeft);
        d->downButton->setIconInSvg(d->scrollbarIconRight);
    } else {
        d->upButton->setIconInSvg  (d->scrollbarIconUp);
        d->downButton->setIconInSvg(d->scrollbarIconDown);
    }
    d->upButton->setIconSize(QSize(8, 8));
    d->downButton->setIconSize(QSize(8, 8));

    setGroup(group());
    d->invalidate();
}

Qt::Orientation ScrollBar::orientation() const
{
    return d->orientation;
}

void ScrollBar::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    setValue(value() - (d->stepSize * event->delta() / 40));
    event->accept();
}

void ScrollBar::setGeometry(const QRectF & g)
{
    if ((g == geometry()) || (g.width() == 0) || (g.height() == 0)) {
        return;
    }
    kDebug() << g;
    kDebug() << geometry();
    Widget::setGeometry(g);
    kDebug() << Widget::geometry();
    d->invalidate();
}

void ScrollBar::setGroup(WidgetGroup * g)
{
    Widget::setGroup(g);
    switch (d->orientation) {
        case Qt::Vertical:
            d->upButton->setGroupByName(group()->name() + "-UpButton");
            d->downButton->setGroupByName(group()->name() + "-DownButton");
            d->upBar->setGroupByName(group()->name() + "-UpBar");
            d->downBar->setGroupByName(group()->name() + "-DownBar");
            d->handle->setGroupByName(group()->name() + "-HandleVertical");
            break;
        case Qt::Horizontal:
            d->upButton->setGroupByName(group()->name() + "-LeftButton");
            d->downButton->setGroupByName(group()->name() + "-RightButton");
            d->upBar->setGroupByName(group()->name() + "-LeftBar");
            d->downBar->setGroupByName(group()->name() + "-RightBar");
            d->handle->setGroupByName(group()->name() + "-HandleHorizontal");
            break;
    }
    d->invalidate();
}

void ScrollBar::stepIncrease()
{
    setValue(value() + d->stepSize);
}

void ScrollBar::stepDecrease()
{
    setValue(value() - d->stepSize);
}

void ScrollBar::pageIncrease()
{
    setValue(value() + d->pageSize);
}

void ScrollBar::pageDecrease()
{
    setValue(value() - d->pageSize);
}

void ScrollBar::setActivationMethod(ActivationMethod method)
{
    if (method == ExtenderActivate) {
        method = HoverActivate;
    }

    if (method == d->activationMethod) {
        return;
    }

    disconnect(d->upButton, 0, &(d->upTimer), 0);
    disconnect(d->downButton, 0, &(d->downTimer), 0);

    switch (method) {
        case HoverActivate:
            d->activationMethod = HoverActivate;
            connect(
                d->upButton, SIGNAL(mouseHoverEnter()),
                &(d->upTimer), SLOT(start())
                );
            connect(
                d->upButton, SIGNAL(mouseHoverLeave()),
                &(d->upTimer), SLOT(stop())
                );
            connect(
                d->downButton, SIGNAL(mouseHoverEnter()),
                &(d->downTimer), SLOT(start())
                );
            connect(
                d->downButton, SIGNAL(mouseHoverLeave()),
                &(d->downTimer), SLOT(stop())
                );
            break;
        case ClickActivate:
            d->activationMethod = ClickActivate;
            connect(
                d->upButton, SIGNAL(pressed()),
                &(d->upTimer), SLOT(start())
                );
            connect(
                d->upButton, SIGNAL(released()),
                &(d->upTimer), SLOT(stop())
                );
            connect(
                d->downButton, SIGNAL(pressed()),
                &(d->downTimer), SLOT(start())
                );
            connect(
                d->downButton, SIGNAL(released()),
                &(d->downTimer), SLOT(stop())
                );
            break;
    }
}

ActivationMethod ScrollBar::activationMethod() const
{
    return d->activationMethod;
}

void ScrollBar::groupUpdated()
{
    Widget::groupUpdated();
    if (group()->hasProperty("ActivationMethod")) {
        setActivationMethod((ActivationMethod)(group()->property("ActivationMethod").toInt()));
    }
    d->setItemSizeHints();
}

QSizeF ScrollBar::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF result;
    switch (which) {
        case Qt::MaximumSize:
            result = MAX_WIDGET_SIZE;
            break;
        default:
            result = d->preferredSize;
            break;
    }
    if (constraint != QSizeF(-1, -1)) {
        result = result.boundedTo(constraint);
    }
    kDebug() << result;
    return result;
}

void ScrollBar::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::MidButton) {

        int position = (d->orientation == Qt::Horizontal) ?
            (event->pos().x()) : (event->pos().y());

        int minPos = (d->orientation == Qt::Horizontal) ?
            (d->upButton->size().width() + d->handleSlideSize / 2) :
            (d->upButton->size().height() + d->handleSlideSize / 2);

        int maxPos = (d->orientation == Qt::Horizontal) ?
            (size().width() - d->downButton->size().width() - d->handleSlideSize / 2) :
            (size().height() - d->downButton->size().height() - d->handleSlideSize / 2);

        if (position > maxPos) {
            position = maxPos;
        } else if (position < minPos) {
            position = minPos;
        }

        setValue(
                d->minimum +
                ((position - minPos) / (qreal) (maxPos - minPos)) *
                (d->maximum - d->minimum)
                );
        event->accept();
    }
    Widget::mouseMoveEvent(event);
}

} // namespace Lancelot

// #include "ScrollBar.moc"

