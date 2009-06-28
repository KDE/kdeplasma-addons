/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_FLIP_LAYOUT_H
#define LANCELOT_FLIP_LAYOUT_H

#include <QGraphicsLayoutItem>
#include <QGraphicsLayout>
#include <lancelot/lancelot_export.h>
#include <plasma/plasma.h>

namespace Lancelot
{

class LANCELOT_EXPORT FlipLayoutManager {
public:
    /**
     * Sets the global flip
     * @param flip new global flip
     */
    void setGlobalFlip(Plasma::Flip flip);

    /**
     * @returns the global flip
     */
    Plasma::Flip globalFlip() const;

    /**
     * Sets the flip for a specified layout
     * @param layout layout
     * @param flip new flip
     */
    void setFlip(const QGraphicsLayout * layout, Plasma::Flip flip);

    /**
     * Makes the specified layout to use the global flip
     * @param layout layout
     */
    void setUseGlobalFlip(const QGraphicsLayout * layout);

    /**
     * @returns the flip for the specified layout. If
     * the layout uses global flip, the global flip is
     * returned
     */
    Plasma::Flip flip(const QGraphicsLayout * layout) const;

    /**
     * @returns the singleton instance of the FlipLayoutManager
     */
    static FlipLayoutManager * instance();

    /**
     * Implementation of the actual flipping
     */
    void setGeometry(QGraphicsLayout * layout) const;

private:
    FlipLayoutManager();
    ~FlipLayoutManager();
    static FlipLayoutManager * m_instance;

    class Private;
    Private * const d;
};

template <typename SuperLayout>
class FlipLayout : public SuperLayout {
public:
    FlipLayout(QGraphicsLayoutItem * parent = NULL)
        : SuperLayout(parent)
    {

    }

    void setFlip(Plasma::Flip flip)
    {
        FlipLayoutManager::instance()->setFlip(this, flip);
    }

    void setUseGlobalFlip()
    {
        FlipLayoutManager::instance()->setUseGlobalFlip(this);
    }

    Plasma::Flip flip() const
    {
        return FlipLayoutManager::instance()->flip(this);
    }

    L_Override void setGeometry(const QRectF & rect)
    {
        SuperLayout::setGeometry(rect);
        FlipLayoutManager::instance()->setGeometry(this);
    }

};

} // namespace Lancelot

#endif /* LANCELOT_FLIP_LAYOUT_H */

