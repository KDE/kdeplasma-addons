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

#include "FlipLayout.h"
#include <QSet>
#include <QMap>

namespace Lancelot
{

FlipLayoutManager * FlipLayoutManager::m_instance = NULL;

class FlipLayoutManager::Private {
public:
    Plasma::Flip globalFlip;
    QSet < QGraphicsLayout * > globalFlipLayouts;
    QMap < QGraphicsLayout *, Plasma::Flip > flips;
};

FlipLayoutManager::FlipLayoutManager()
    : d(new Private())
{

}

FlipLayoutManager * FlipLayoutManager::instance()
{
    if (m_instance == NULL) {
        m_instance = new FlipLayoutManager();
    }
    return m_instance;
}

void FlipLayoutManager::setGlobalFlip(Plasma::Flip flip)
{
    d->globalFlip = flip;
}

Plasma::Flip FlipLayoutManager::globalFlip() const
{
    return d->globalFlip;
}

void FlipLayoutManager::setFlip(QGraphicsLayout * layout, Plasma::Flip flip)
{
    d->globalFlipLayouts.remove(layout);
    d->flips[layout] = flip;
}

void FlipLayoutManager::setUseGlobalFlip(QGraphicsLayout * layout)
{
    d->flips.remove(layout);
    d->globalFlipLayouts.insert(layout);
}

Plasma::Flip FlipLayoutManager::flip(QGraphicsLayout * layout) const
{
    if (d->globalFlipLayouts.contains(layout)) {
        return d->globalFlip;
    } else if (d->flips.contains(layout)) {
        return d->flips[layout];
    }
    return 0;
}

}

