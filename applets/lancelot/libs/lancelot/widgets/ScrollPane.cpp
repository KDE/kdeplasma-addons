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

#include <KDebug>

#include "ScrollPane.h"
#include "ScrollBar.h"
#include "Widget.h"
#include <lancelot/layouts/FullBorderLayout.h>

#define SCROLLBAR_WIDTH 16

namespace Lancelot
{

// Scrollable interface common functions implementation

class Scrollable::Private {
public:
    ScrollPane * pane;
};

Scrollable::Scrollable()
  : d(new Private())
{
    d->pane = NULL;
}

Scrollable::~Scrollable()
{
    delete d;
}

void Scrollable::setScrollPane(ScrollPane * pane)
{
    if (pane == d->pane) return;
    d->pane = pane;
    d->pane->setScrollableWidget(this);
}

ScrollPane * Scrollable::scrollPane() const
{
    return d->pane;
}

// ScrollPane implementation

class ScrollPane::Private {
public:
    Private()
        : widget(NULL), layout(NULL),
          vertical(NULL), horizontal(NULL)
    {

    }

    Scrollable   * widget;
    FullBorderLayout * layout;
    ScrollBar * vertical;
    ScrollBar * horizontal;
};

ScrollPane::ScrollPane(QGraphicsItem * parent)
    : Widget(parent), d(new Private())
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    d->layout = new FullBorderLayout(this);

    d->vertical   = new ScrollBar(this);
    d->vertical->setOrientation(Qt::Vertical);
    d->horizontal = new ScrollBar(this);
    d->horizontal->setOrientation(Qt::Horizontal);

    d->layout->addItem(d->vertical, FullBorderLayout::Right);
    d->layout->addItem(d->horizontal, FullBorderLayout::Bottom);
    // d->layout->addItem(new Widget(this));

    setLayout(d->layout);
    L_WIDGET_SET_INITIALIZED;
}

ScrollPane::~ScrollPane()
{
    delete d;
}

void ScrollPane::setScrollableWidget(Scrollable * widget)
{
    d->widget = widget;
    d->layout->addItem(
            dynamic_cast<QGraphicsWidget *>(widget),
            FullBorderLayout::Center);
    scrollableWidgetSizeUpdated();
}

QSizeF ScrollPane::maximumViewportSize() const
{
    return size();
}

QSizeF ScrollPane::currentViewportSize() const
{
    return size();
}

void ScrollPane::scrollableWidgetSizeUpdated()
{
    if (!d->widget) {
        return;
    }

    d->horizontal->setMinimum(0);
    d->horizontal->setMaximum(d->widget->fullSize().width());

    d->vertical->setMinimum(0);
    d->vertical->setMaximum(d->widget->fullSize().height());
}

} // namespace Lancelot

