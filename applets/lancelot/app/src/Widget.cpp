/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free 
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QVariant>
#include "Widget.h"
#include "Global.h"
#include <KDebug>

namespace Lancelot {

Widget::Widget(QString name, QGraphicsItem * parent) :
    Plasma::Widget(parent), m_hover(false), m_enabled(true),
    m_name(name), m_group(NULL)
{
    kDebug() << "Creating a widget " << name << "\n";
    Global::instance()->addWidget(this);
    setGroup();
    kDebug() << "Created widget " << name << "\n";
}

Widget::~Widget()
{
    
}

void Widget::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
    if (!m_enabled) return;
    m_hover = true;
    Plasma::Widget::hoverEnterEvent(event);
    emit mouseHoverEnter();
    update();
}

void Widget::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
    if (!m_enabled) return;
    m_hover = false;
    Plasma::Widget::hoverEnterEvent(event);
    emit mouseHoverLeave();
    update();
}

void Widget::setGroupByName(const QString & groupName)
{
    setGroup(WidgetGroup::group(groupName));
}

void Widget::setGroup(WidgetGroup * group)
{
    if (group == NULL) {
        group = WidgetGroup::defaultGroup();
    }
    
    if (group == m_group) return;

    if (m_group != NULL) {
        m_group->removeWidget(this);
    }
    
    m_group = group;
    m_group->addWidget(this);
    groupUpdated();
}

WidgetGroup * Widget::group() {
    return m_group;
}

void Widget::groupUpdated()
{
    update();
}

QString Widget::name() const
{
    return m_name;
}

void Widget::setName(QString name)
{
    m_name = name;
}

void Widget::setGeometry(const QRectF & geometry)
{
    if (!Global::instance()->processGeometryChanges)
        return;
    Plasma::Widget::setGeometry(geometry);
}

void Widget::update(const QRectF &rect)
{
    if (!Global::instance()->processGeometryChanges)
        return;
    Plasma::Widget::update(rect);
}

void Widget::update(qreal x, qreal y, qreal w, qreal h)
{
    if (!Global::instance()->processGeometryChanges)
        return;
    Plasma::Widget::update(x, y, w, h);
}

void Widget::paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    // TODO: Comment the next line
    //kDebug() << name() << "!\n";
    //if (!m_group)
    //    kDebug() << "Group is not assigned for" << name() << "!\n";
    //if (!m_backgroundColorNormal)
    //    kDebug() << "Group is not processed for" << name() << "!\n";
    //painter->fillRect(QRectF(QPointF(0, 0), size()), QBrush(* m_backgroundColorNormal));
    
    paintBackground(painter);
}

void Widget::enable(bool value) {
    if (m_hover && m_enabled) {
        hoverLeaveEvent(NULL);
    }
    m_enabled = value;
}

void Widget::disable() {
    enable(false);
}

bool Widget::enabled() const {
    return m_enabled;
}

void Widget::paintBackground (QPainter * painter) {
    if (!m_group) return;
    
    // Background Painting
    kDebug() << "Painting bkg ####\n";
    if (Plasma::Svg * svg = m_group->backgroundSvg()) {
        svg->resize(size());
        //QString element = m_svgElementPrefix + "button_" + (m_hover?"active":"inactive") + m_svgElementSufix;
        QString element;
        if (!m_enabled) {
            element = "disabled";
        } else if (m_hover) {
            element = "active";
        } else {
            element = "normal";
        }
        
        svg->paint(painter, 0, 0, element);
        
    } else if (const WidgetGroup::ColorScheme * scheme = m_group->backgroundColor()) {
        const QColor * color;
        if (!m_enabled) {
            color = & (scheme->disabled);
        } else if (m_hover) {
            color = & (scheme->active);
        } else {
            color = & (scheme->normal);
        }
        
        painter->fillRect(QRectF(QPointF(0, 0), size()), QBrush(*color));

    }

}

}

#include "Widget.moc"
