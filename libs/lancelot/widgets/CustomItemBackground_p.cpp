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

#include <lancelot/widgets/CustomItemBackground_p.h>

#include <Plasma/FrameSvg>

#include <Global.h>

namespace Lancelot
{

CustomItemBackground::CustomItemBackground(QGraphicsWidget * parent)
    : Plasma::ItemBackground(parent)
{
    KConfigGroup config(Global::self()->config(), "Main");
    m_animation = config.readEntry("itemViewBackgroundAnimation", true);
}

CustomItemBackground::~CustomItemBackground()
{

}

void CustomItemBackground::setTarget(const QRectF & target)
{
    if (m_animation) {
        Plasma::ItemBackground::setTarget(target);
    } else {
        setGeometry(target);
    }
}

void CustomItemBackground::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Plasma::ItemBackground::paint(painter, option, widget);
}

} // namespace Lancelot

