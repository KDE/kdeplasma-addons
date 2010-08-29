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

#include <QPixmap>
#include <QPainter>

#include <KDebug>

#include <Plasma/FrameSvg>

#include <Global.h>

namespace Lancelot
{

CustomItemBackground::CustomItemBackground(QGraphicsWidget * parent)
    : Plasma::ItemBackground(parent), m_group(NULL)
{
    m_animation = !Global::self()->config("Animation", "disableAnimations", false)
                && Global::self()->config("Animation", "itemViewBackgroundAnimation", true);
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
    Q_UNUSED(widget)

    if (!m_group || !m_group->backgroundSvg()) {
        Plasma::ItemBackground::paint(painter, option, widget);
        return;
    }

    Plasma::FrameSvg * svg = m_group->backgroundSvg();
    svg->setElementPrefix(m_svgPrefix);
    svg->resizeFrame(size().toSize());
    svg->paintFrame(painter);
}

void CustomItemBackground::setSvgElementPrefix(const QString & prefix)
{
    m_svgPrefix = prefix;
}

void CustomItemBackground::setGroup(Group * group)
{
    m_group = group;
    group->add(this);
}

} // namespace Lancelot

