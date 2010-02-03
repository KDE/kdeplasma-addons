/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef LANCELOT_CUSTOM_ITEM_BACKGROUND_H
#define LANCELOT_CUSTOM_ITEM_BACKGROUND_H

#include <QtGui/QIcon>

#include <Plasma/Plasma>
#include <Plasma/ItemBackground>
#include <Plasma/FrameSvg>

#include <lancelot/Global.h>

namespace Lancelot
{

/**
 * @author Ivan Cukic
 */
class CustomItemBackground: public Plasma::ItemBackground {
public:
    CustomItemBackground(QGraphicsWidget * parent = 0);

    ~CustomItemBackground();

    void setTarget(const QRectF & target);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setSvgElementPrefix(const QString & prefix = QString());
    void setGroup(Group * group);

private:
    QString m_svgPrefix;
    bool m_animation : 1;
    Group * m_group;

};

} // namespace Lancelot

#endif /* LANCELOT_CUSTOM_ITEM_BACKGROUND_H */

