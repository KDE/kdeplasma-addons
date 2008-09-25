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

#ifndef LANCELOT_FLIP_LAYOUT_H_
#define LANCELOT_FLIP_LAYOUT_H_

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

    L_Override virtual void setGeometry(const QRectF & geometry)
    {
        SuperLayout::setGeometry(geometry);
        QRectF rect = SuperLayout::geometry();

        int count = SuperLayout::count();

        if (flip() == Plasma::NoFlip) {
            return;
        }

        QRectF childGeometry;
        for (int i = 0; i < count; i++) {
            QGraphicsLayoutItem * item = SuperLayout::itemAt(i);

            if (!item) continue;

            childGeometry = item->geometry();
            if (flip() & Plasma::HorizontalFlip) {
                // 2 * rect.left() - twice because we already have one
                // value of rect.left() inside the childGeometry.left()
                childGeometry.moveLeft(
                    2 * rect.left()       + rect.width()
                      - childGeometry.left()  - childGeometry.width()
                );
            }
            if (flip() & Plasma::VerticalFlip) {
                // 2 * rect.top() - same reason as aforemontioned
                childGeometry.moveTop(
                    2 * rect.top()      + rect.height()
                      - childGeometry.top() - childGeometry.height()
                );
            }
            item->setGeometry(childGeometry);
        }
    }

};

} // namespace Lancelot

#endif /* LANCELOT_FLIP_LAYOUT_H_ */

