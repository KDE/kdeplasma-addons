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

#ifndef LANCELOT_PANEL_H_
#define LANCELOT_PANEL_H_

#include "Widget.h"
#include <plasma/layouts/layout.h>
#include <plasma/svgpanel.h>
#include "BaseActionWidget.h"


#include <QIcon>

namespace Lancelot
{

class Panel: public Widget
{
public:
    Panel(QString name, QIcon * icon, QString title = QString(), QGraphicsItem * parent = 0);
    Panel(QString name, QString title, QGraphicsItem * parent = 0);
    Panel(QString name, QGraphicsItem * parent = 0);

    virtual ~Panel();

    void setTitle(const QString & title);
    QString title() const;

    void setIcon(QIcon * icon);
    QIcon * icon() const;

    void setIconSize(QSize size);
    QSize iconSize() const;

    void setGeometry (const QRectF & geometry);

    void setLayout(Plasma::LayoutItem * layout);
    Plasma::LayoutItem * layout();

    void setWidget(Widget * widget);
    Widget * widget();

    void setBackground(const QString & imagePath);
    void clearBackground();

    void setGroup(WidgetGroup * group = NULL);

    void paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

private:
    void init();
    void invalidate();

    Plasma::LayoutItem * m_layout;
    Widget * m_widget;
    bool m_hasTitle;

    BaseActionWidget m_titleWidget;
    Plasma::SvgPanel * m_background;
};

}

#endif /*PANEL_H_*/
