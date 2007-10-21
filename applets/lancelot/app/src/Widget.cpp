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
    Plasma::Widget(parent), m_name(name), m_foregroundColorNormal(NULL),
            m_foregroundColorActive(NULL), m_backgroundColorNormal(NULL),
            m_backgroundColorActive(NULL), m_group(NULL)

{
    Global::getInstance()->addWidget(this);
    Group::getDefaultGroup()->addWidget(this);
}

Widget::~Widget()
{
    
}

void Widget::setGroup(Group * group)
{
    if (group == NULL) {
        group = Group::getDefaultGroup();
    }
    
    if (group == m_group) return;

    if (m_group != NULL) {
        m_group->removeWidget(this);
    }
    
    m_group = group;
    m_group->addWidget(this);
}

Group * Widget::getGroup() {
    return m_group;
}

void Widget::groupUpdated()
{
    kDebug() << name() << "\n";
    if (m_group->hasProperty("foregroundColorNormal")) {
        m_foregroundColorNormal
                = (QColor *) m_group->getPropertyAsPointer("foregroundColorNormal");
        kDebug() << name() << " :: foregroundColorNormal " << (long) m_foregroundColorNormal << "\n";
    }
    if (m_group->hasProperty("foregroundColorActive")) {
        m_foregroundColorActive
                = (QColor *) m_group->getPropertyAsPointer("foregroundColorActive");
    }
    if (m_group->hasProperty("backgroundColorNormal")) {
        m_backgroundColorNormal
                = (QColor *) m_group->getPropertyAsPointer("backgroundColorNormal");
    }
    if (m_group->hasProperty("backgroundColorActive")) {
        m_backgroundColorActive
                = (QColor *) m_group->getPropertyAsPointer("backgroundColorActive");
    }
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
    if (!Global::getInstance()->processGeometryChanges)
        return;
    Plasma::Widget::setGeometry(geometry);
}

void Widget::update(const QRectF &rect)
{
    if (!Global::getInstance()->processGeometryChanges)
        return;
    Plasma::Widget::update(rect);
}

void Widget::update(qreal x, qreal y, qreal w, qreal h)
{
    if (!Global::getInstance()->processGeometryChanges)
        return;
    Plasma::Widget::update(x, y, w, h);
}

}
