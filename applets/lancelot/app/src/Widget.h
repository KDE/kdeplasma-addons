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

#ifndef LANCELOT_WIDGET_H_
#define LANCELOT_WIDGET_H_

#include <plasma/widgets/widget.h>
#include "Global.h"

namespace Lancelot
{

class Widget : public Plasma::Widget
{
public:
	Widget(QString name, QGraphicsItem * parent = 0);
	virtual ~Widget();

    void setGeometry (const QRectF & geometry);
    void update (const QRectF &rect = QRectF());
    void update (qreal x, qreal y, qreal w, qreal h);
    
    void setGroup(Group * group);
    Group * getGroup();

    QString name() const;
    void setName(QString name);

protected:
    QString m_name;
    Group * m_group;
    
    QColor * m_foregroundColorNormal;
    QColor * m_foregroundColorActive;
    QColor * m_backgroundColorNormal;
    QColor * m_backgroundColorActive;
    
    virtual void groupUpdated();

    friend class Group;
    friend class Global;
};

}

#endif /*LANCELOT_WIDGET_H_*/
