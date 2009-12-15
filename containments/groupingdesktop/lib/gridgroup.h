/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef GRIDGROUP_H
#define GRIDGROUP_H

#include "abstractgroup.h"

class QGraphicsGridLayout;
class QGraphicsLinearLayout;

namespace Plasma
{
    class Applet;
    class IconWidget;
}

class Position;
class Spacer;
class ItemOverlay;

class GridGroup : public AbstractGroup
{
    Q_OBJECT
    public:
        GridGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        ~GridGroup();

        void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const;
        void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup& group);
        QString pluginName() const;
        virtual void showDropZone(const QPointF &pos);

    protected:
        bool eventFilter(QObject *watched, QEvent *event);
        void layoutChild(QGraphicsWidget *child, const QPointF &pos);

    private slots:
        void onAppletAdded(Plasma::Applet *applet, AbstractGroup *group);
        void onAppletRemoved(Plasma::Applet *applet, AbstractGroup *group);
        void onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *group);
        void onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *group);
        void overlayStartsMoving();
        void overlayEndsMoving();
        void overlayMoving(qreal x, qreal y, const QPointF &mousePos);
        void onItemMovedOutside(qreal x, qreal y);

    private:
        enum Orientation {
            Horizontal = 0,
            Vertical = 1
        };

        void showDropZone(const QPointF &pos, bool showAlwaysSomething);
        QGraphicsLayoutItem *removeItemAt(int row, int column, bool fillLayout = true);
        QGraphicsLayoutItem *removeItemAt(const Position &position, bool fillLayout = true);
        void removeItem(QGraphicsWidget *item, bool fillLayout = true);
        void insertItemAt(QGraphicsWidget *item, int row, int column, Orientation orientation = Horizontal);
        Position itemPosition(QGraphicsItem *item) const;
        int nearestBoundair(qreal pos, qreal size) const;

        QGraphicsGridLayout *m_layout;
        Spacer *m_spacer;
        AbstractGroup *m_interestingGroup;

        QPointF posOfClick;
        ItemOverlay *m_overlay;

        friend class Spacer;
};

#endif // GRIDGROUP_H
