/*
 *   Copyright 2009-2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

class QGraphicsLinearLayout;

class Spacer;
class GridManager;

class GridGroup : public AbstractGroup
{
    Q_OBJECT
    public:
        explicit GridGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        ~GridGroup();

        void init();
        QString pluginName() const;
        bool showDropZone(const QPointF &pos);
        void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group);
        void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        static GroupInfo groupInfo();

    protected:
        void resizeEvent(QGraphicsSceneResizeEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        bool eventFilter(QObject *obj, QEvent *event);
        void layoutChild(QGraphicsWidget *child, const QPointF &pos);

    private slots:
        void addNewRowOrColumn();
        void removeRowOrColumn();
        void appletRemoved(Plasma::Applet *applet);
        void subGroupRemoved(AbstractGroup *group);
        void resizeDone();

    private:
        void updateChild(QGraphicsWidget *child);
        void updateGeometries();
        QGraphicsWidget *childAt(int column, int row);
        void checkCorner(const QPointF &pos, QGraphicsWidget *widget);

        int m_rowsNumber;
        int m_colsNumber;
        bool m_showGrid;
        QHash<QGraphicsWidget *, QRect> m_childrenRects;
        QWeakPointer<Spacer> m_cornerHandle;
        Qt::Corner m_handleCorner;
        QPointF m_resizeStartPos;
        QRectF m_resizeStartGeom;
        GridManager *m_gridManager;

};

#endif // GRIDGROUP_H
