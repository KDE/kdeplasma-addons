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

#include "Panel.h"
#include "BasicWidget.h"
#include <plasma/framesvg.h>

namespace Lancelot
{

class Panel::Private {
public:
    Private(QIcon icon, QString title, Panel * parent)
      : layoutItem(NULL),
        hasTitle(!title.isEmpty()),
        titleWidget(icon, title, "", parent),
        q(parent)
    {
        init();
    }

    Private(QString title, Panel * parent)
      : layoutItem(NULL),
        hasTitle(!title.isEmpty()),
        titleWidget(title, "", parent),
        q(parent)
    {
        init();
    }

    Private(Panel * parent)
      : layoutItem(NULL),
        hasTitle(false),
        titleWidget("", "", parent),
        q(parent)
    {
        init();
    }

    ~Private()
    {
    }

    void init()
    {
        showingTitle = hasTitle;
        titleWidget.setIconSize(QSize(16, 16));
        titleWidget.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        titleWidget.setEnabled(false);
        invalidate();
    }

    void invalidate()
    {
        QRectF rect = QRectF(QPointF(), q->size());
        if (q->group() && q->group()->backgroundSvg()) {
            rect.setTop(q->group()->backgroundSvg()->marginSize(Plasma::TopMargin));
            rect.setLeft(q->group()->backgroundSvg()->marginSize(Plasma::LeftMargin));
            rect.setWidth(rect.width() - q->group()->backgroundSvg()->marginSize(Plasma::RightMargin));
            rect.setHeight(rect.height() - q->group()->backgroundSvg()->marginSize(Plasma::BottomMargin));
        }

        if (!showingTitle || !hasTitle) {
            titleWidget.hide();
            if (layoutItem) {
                layoutItem->setGeometry(rect);
            }
        } else {
            qreal h = rect.height();

            titleWidget.show();
            rect.setHeight(32);
            titleWidget.setGeometry(rect);

            rect.setTop(32);
            rect.setHeight(h - 32);

            if (layoutItem) {
                layoutItem->setGeometry(rect);
            }
        }
    }

    QGraphicsLayoutItem * layoutItem;
    bool hasTitle;
    bool showingTitle;

    BasicWidget titleWidget;
    Panel * q;
};

Panel::Panel(QIcon icon, QString title, QGraphicsItem * parent)
  : Widget(parent), d(new Private(icon, title, this))
{
    setGroupByName("Panel");
    L_WIDGET_SET_INITIALIZED;
}

Panel::Panel(QString title, QGraphicsItem * parent)
  : Widget(parent), d(new Private(title, this))
{
    setGroupByName("Panel");
    L_WIDGET_SET_INITIALIZED;
}

Panel::Panel(QGraphicsItem * parent)
  : Widget(parent), d(new Private(this))
{
    setGroupByName("Panel");
    L_WIDGET_SET_INITIALIZED;
}

Panel::~Panel()
{
    delete d;
}

qreal Panel::borderSize(Plasma::MarginEdge edge) const
{
    if (!(group()) || !(group()->backgroundSvg())) {
        return 0;
    }
    return group()->backgroundSvg()->marginSize(edge);
}

void Panel::setTitle(const QString & title)
{
    d->hasTitle = (title != "");
    d->titleWidget.setTitle(title);
}

QString Panel::title() const
{
    return d->titleWidget.title();
}

void Panel::setIcon(QIcon icon)
{
    d->titleWidget.setIcon(icon);
}

QIcon Panel::icon() const
{
    return d->titleWidget.icon();
}

void Panel::setIconSize(QSize size)
{
    d->titleWidget.setIconSize(size);
}

QSize Panel::iconSize() const
{
    return d->titleWidget.iconSize();
}

void Panel::setShowingTitle(bool value)
{
    d->showingTitle = value;
    d->titleWidget.setVisible(value);
}

bool Panel::isShowingTitle() const
{
    return d->showingTitle;
}

void Panel::setGeometry(const QRectF & geometry)
{
    Widget::setGeometry(geometry);
    d->invalidate();
}

void Panel::setGroup(WidgetGroup * g)
{
    Widget::setGroup(g);
    d->titleWidget.setGroupByName(group()->name() + "-Title");
}

void Panel::setLayoutItem(QGraphicsLayoutItem * layoutItem)
{
    d->layoutItem = layoutItem;
    d->invalidate();
}

QGraphicsLayoutItem * Panel::layoutItem() const
{
    return d->layoutItem;
}

QSizeF Panel::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF result = QSizeF();

    if (!L_WIDGET_IS_INITIALIZED) {
        return result;
    }

    // TODO: Count the header as well

    if (!d->layoutItem) {
        switch (which) {
            case Qt::MinimumSize:
                result = QSizeF();
                break;
            case Qt::MaximumSize:
                result = MAX_WIDGET_SIZE;
                break;
            default:
                result = QSizeF(100, 100);
        }
    } else {
        result = d->layoutItem->effectiveSizeHint(which, constraint);
    }
    if (constraint != QSizeF(-1, -1)) {
        result = result.boundedTo(constraint);
    }
    return result;
}


} // namespace Lancelot

