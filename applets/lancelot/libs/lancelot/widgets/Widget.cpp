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
        name(QString()),
        group(NULL)
    {
    };

    bool hover;
    QString name;
    WidgetGroup * group;
};

Widget::Widget(QString name, QGraphicsItem * parent)
  : QGraphicsWidget(parent),
    d(new Private())
{
    Instance::activeInstance()->addWidget(this);
    d->group = Instance::activeInstance()->defaultGroup();
    d->name  = name;
}

Widget::~Widget()
{
    if (d->group != NULL) {
        d->group->removeWidget(this, false);
    }
    delete d;
}

void Widget::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    if (!isEnabled()) return;
    d->hover = true;
    QGraphicsWidget::hoverEnterEvent(event);
    emit mouseHoverEnter();
    update();
}

void Widget::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    if (!isEnabled()) return;
    d->hover = false;
    QGraphicsWidget::hoverEnterEvent(event);
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

    return d->group->instance();
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

void Widget::paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
        QWidget * widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    paintBackground(painter);
}

void Widget::paintBackground(QPainter * painter) {
    if (!d->group) return;

    QString element;
    if (!isEnabled()) {
        element = "disabled";
    } else if (d->hover) {
        element = "active";
    } else {
        element = "normal";
    }

    paintBackground(painter, element);
}

void Widget::paintBackground(QPainter * painter, const QString & element) {
    if (!d->group) return;

    // Background Painting
    if (Plasma::Svg * svg = d->group->backgroundSvg()) {
        kDebug() << "Painting SVG background " << svg->imagePath();
        kDebug() << "Resizing to " << size();

        // TODO: Fix rendering - it is pixelated
        // svg->resize(size());  // something's wrong
        svg->paint(painter, QRectF(QPointF(), size()), element);

    } else if (const WidgetGroup::ColorScheme * scheme = d->group->backgroundColor()) {
        kDebug() << "Painting simple background";
        const QColor * color;
        if (!isEnabled()) {
            color = & (scheme->disabled);
        } else if (d->hover) {
            color = & (scheme->active);
        } else {
            color = & (scheme->normal);
        }

        painter->fillRect(QRectF(QPointF(0, 0), size()), QBrush(*color));

    }

}

void Widget::geometryUpdated()
{

}

void Widget::setGeometry(const QRectF & rect)
{
    QGraphicsWidget::setGeometry(rect);
    geometryUpdated();
}

void Widget::setGeometry(qreal x, qreal y, qreal w, qreal h)
{
    Widget::setGeometry(QRectF(x, y, w, h));
}

} // namespace Lancelot

#include "Widget.moc"
