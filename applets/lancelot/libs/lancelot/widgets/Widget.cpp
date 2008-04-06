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

#include <QVariant>
#include "Widget.h"
#include "Global.h"
#include <KDebug>

namespace Lancelot {

class Widget::Private {
public:
    Private()
      : hover(false),
        enabled(true),
        name(QString()),
        group(NULL)
    {};

    bool hover;
    bool enabled;
    QString name;
    WidgetGroup * group;
};

Widget::Widget(QString name, QGraphicsItem * parent) :
    Plasma::Widget(parent),
    d(new Private())
    //WidgetCore(),
    //d->hover(false),
    //d->enabled(true),
    //d->name(name),
    //d->group(NULL)
{
    Instance::activeInstance()->addWidget(this);
    d->group = Instance::activeInstance()->defaultGroup();
}

Widget::~Widget()
{
    if (d->group != NULL) {
        d->group->removeWidget(this, false);
    }
}

void Widget::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
{
    if (!d->enabled) return;
    d->hover = true;
    Plasma::Widget::hoverEnterEvent(event);
    emit mouseHoverEnter();
    update();
}

void Widget::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
{
    if (!d->enabled) return;
    d->hover = false;
    Plasma::Widget::hoverEnterEvent(event);
    emit mouseHoverLeave();
    update();
}

bool Widget::isHovered() const
{
    return d->hover;
}

void Widget::setGroupByName(const QString & groupName)
{
    setGroup(instance()->group(groupName));
}

void Widget::setGroup(WidgetGroup * group)
{
    if (group == NULL) {
        group = instance()->defaultGroup();
    }

    if (group == d->group) return;

    if (d->group != NULL) {
        d->group->removeWidget(this);
    }

    d->group = group;
    d->group->addWidget(this);
    groupUpdated();
}

WidgetGroup * Widget::group()
{
    return d->group;
}

Instance * Widget::instance()
{
    if (!d->group) return Instance::activeInstance();

    return d->group->m_instance;
}

void Widget::groupUpdated()
{
    update();
}

QString Widget::name() const
{
    return d->name;
}

void Widget::setName(QString name)
{
    d->name = name;
}

void Widget::setGeometry(const QRectF & geometry)
{
    if (!d->group) //!instance()->processGeometryChanges)
        return;
    Plasma::Widget::setGeometry(geometry);
}

void Widget::update(const QRectF &rect)
{
    if (!d->group) //!instance()->processGeometryChanges)
        return;
    Plasma::Widget::update(rect);
}

void Widget::update(qreal x, qreal y, qreal w, qreal h)
{
    if (!d->group) //!instance()->processGeometryChanges)
        return;
    Plasma::Widget::update(x, y, w, h);
}

void Widget::paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    paintBackground(painter);
}

void Widget::enable(bool value) {
    if (d->hover && d->enabled) {
        hoverLeaveEvent(NULL);
    }
    d->enabled = value;
}

void Widget::disable() {
    enable(false);
}

bool Widget::isEnabled() const {
    return d->enabled;
}

void Widget::paintBackground (QPainter * painter) {
    if (!d->group) return;

    QString element;
    if (!d->enabled) {
        element = "disabled";
    } else if (d->hover) {
        element = "active";
    } else {
        element = "normal";
    }

    paintBackground(painter, element);
}

void Widget::paintBackground (QPainter * painter, const QString & element) {
    if (!d->group) return;

    // Background Painting
    if (Plasma::Svg * svg = d->group->backgroundSvg()) {
        svg->resize(size());

        svg->paint(painter, 0, 0, element);

    } else if (const WidgetGroup::ColorScheme * scheme = d->group->backgroundColor()) {
        const QColor * color;
        if (!d->enabled) {
            color = & (scheme->disabled);
        } else if (d->hover) {
            color = & (scheme->active);
        } else {
            color = & (scheme->normal);
        }

        painter->fillRect(QRectF(QPointF(0, 0), size()), QBrush(*color));

    }

}

}

#include "Widget.moc"
