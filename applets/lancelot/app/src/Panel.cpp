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

namespace Lancelot
{

Panel::Panel(QString name, QIcon * icon, QString title, QGraphicsItem * parent)
  : Widget(name, parent),
    m_layout(NULL), m_widget(NULL), m_hasTitle(title != QString()),
    m_titleWidget(name + "::TitleWidget", icon, title, "", this)
{
    init();
}

Panel::Panel(QString name, QString title, QGraphicsItem * parent)
  : Widget(name, parent),
    m_layout(NULL), m_widget(NULL), m_hasTitle(title != QString()),
    m_titleWidget(name + "::TitleWidget", title, "", this)
{
    init();
}

Panel::Panel(QString name, QGraphicsItem * parent)
  : Widget(name, parent),
    m_layout(NULL), m_widget(NULL), m_hasTitle(false),
    m_titleWidget(name + "::TitleWidget", "", "" , this)
{
    init();
}

Panel::~Panel()
{
}


void Panel::init()
{
    m_titleWidget.setIconSize(QSize(16, 16));
    m_titleWidget.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_titleWidget.disable();
    setGroupByName("Panel");
    invalidate();
}

void Panel::setTitle(const QString & title)
{
    m_hasTitle = (title != "");
    m_titleWidget.setTitle(title);
}

QString Panel::title() const {
    return m_titleWidget.title();
}

void Panel::setIcon(QIcon * icon) {
    m_titleWidget.setIcon(icon);
}

QIcon * Panel::icon() const {
    return m_titleWidget.icon();
}

void Panel::setIconSize(QSize size) {
    m_titleWidget.setIconSize(size);
}

QSize Panel::iconSize() const {
    return m_titleWidget.iconSize();
}

void Panel::setGeometry (const QRectF & geometry) {
    Widget::setGeometry(geometry);
    invalidate();
}

void Panel::setGroup(WidgetGroup * group)
{
    Widget::setGroup(group);
    m_titleWidget.setGroupByName(m_group->name() + "-Title");
}

void Panel::invalidate() {
    if (!m_hasTitle) {
        m_titleWidget.hide();
        if (m_widget) {
            m_widget->setGeometry(QRectF(QPointF(), size()));
        }
        if (m_layout) {
            m_layout->setGeometry(QRectF(QPointF(), size()));
        }
    } else {
        m_titleWidget.show();
        QRectF rect(0, 0, size().width(), 32);
        m_titleWidget.setGeometry(rect);
        rect.setTop(32);
        rect.setHeight(size().height() - 32);

        if (m_layout) {
            m_layout->setGeometry(rect);
        }

        if (m_widget) {
            m_widget->setGeometry(rect);
        }
    }
}

void Panel::setLayout(Plasma::LayoutItem * layout) {
    m_layout = layout;
    invalidate();
}

Plasma::LayoutItem * Panel::layout() {
    return m_layout;
}

void Panel::setWidget(Widget * widget) {
    m_widget = widget;
    widget->setParentItem(this);
    invalidate();
}

Widget * Panel::widget() {
    return m_widget;
}

}
