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
#include <KDebug>
#include "BasicWidget.h"
#include <plasma/panelsvg.h>

namespace Lancelot
{

class Panel::Private {
public:
    Private(QIcon icon, QString title, Panel * parent)
      : layoutItem(NULL),
        hasTitle(title != QString()),
        titleWidget(icon, title, "", parent),
        background(NULL),
        q(parent)
    {
        init();
    }

    Private(QString title, Panel * parent)
      : layoutItem(NULL),
        hasTitle(title != QString()),
        titleWidget(title, "", parent),
        background(NULL),
        q(parent)
    {
        init();
    }

    Private(Panel * parent)
      : layoutItem(NULL),
        hasTitle(false),
        titleWidget("", "", parent),
        background(NULL),
        q(parent)
    {
        init();
    }

    ~Private()
    {
        delete background;
    }

    void init()
    {
        titleWidget.setIconSize(QSize(16, 16));
        titleWidget.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        titleWidget.setEnabled(false);
        invalidate();
    }

    void invalidate()
    {
        QRectF rect = QRectF(QPointF(), q->size());
        if (background) {
            rect.setTop(background->marginSize(Plasma::TopMargin));
            rect.setLeft(background->marginSize(Plasma::LeftMargin));
            rect.setWidth(rect.width() - background->marginSize(Plasma::RightMargin));
            rect.setHeight(rect.height() - background->marginSize(Plasma::BottomMargin));
        }

        if (!hasTitle) {
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

    BasicWidget titleWidget;
    Plasma::PanelSvg * background;
    Panel * q;
};

Panel::Panel(QIcon icon, QString title, QGraphicsItem * parent)
  : Widget(parent), d(new Private(icon, title, this))
{
    setGroupByName("Panel");
}

Panel::Panel(QString title, QGraphicsItem * parent)
  : Widget(parent), d(new Private(title, this))
{
    setGroupByName("Panel");
}

Panel::Panel(QGraphicsItem * parent)
  : Widget(parent), d(new Private(this))
{
    setGroupByName("Panel");
}

Panel::~Panel()
{
    delete d;
}

qreal Panel::borderSize(Plasma::MarginEdge edge)
{
    if (!d->background) {
        return 0;
    }
    return d->background->marginSize(edge);
}

void Panel::setBackground(const QString & imagePath)
{
    kDebug() << " Setting background " << imagePath;
    if (!d->background) {
        d->background = new Plasma::PanelSvg(this);
        d->background->setEnabledBorders(Plasma::PanelSvg::AllBorders);
    }

    d->background->setImagePath(imagePath);
    kDebug() << " Setting background " << d->background->isValid();
    d->invalidate();
}

void Panel::clearBackground()
{
    delete d->background;
    d->background = NULL;
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

void Panel::setGeometry(qreal x, qreal y, qreal w, qreal h)
{
    setGeometry(QRectF(x, y, w, h));
}

void Panel::setGeometry(const QRectF & geometry)
{
    Widget::setGeometry(geometry);
    if (d->background) {
        d->background->resizePanel(geometry.size());
    }
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

QGraphicsLayoutItem * Panel::layoutItem()
{
    return d->layoutItem;
}

void Panel::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (d->background) {
        kDebug() << " Panel has a background " << d->background->isValid();
        // d->background->resizePanel(boundingRect().size());
        d->background->paintPanel(painter, option->rect);
    } else {
        kDebug() << " Panel has no background ";
    }
}

} // namespace Lancelot

