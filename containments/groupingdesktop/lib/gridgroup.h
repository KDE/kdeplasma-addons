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

namespace Plasma
{
    class Applet;
    class IconWidget;
}

class Position;
class Spacer;
class Handle;

class GridGroup : public AbstractGroup
{
    Q_OBJECT
    public:
        explicit GridGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        ~GridGroup();

        void init();
        void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const;
        void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup& group);
        QString pluginName() const;
        void showDropZone(const QPointF &pos);
        Handle *createHandleForChild(QGraphicsWidget *child);

    protected:
        void layoutChild(QGraphicsWidget *child, const QPointF &pos);
        void resizeEvent(QGraphicsSceneResizeEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        bool eventFilter(QObject *obj, QEvent *event);

    private slots:
        void onAppletAdded(Plasma::Applet *applet, AbstractGroup *group);
        void onAppletRemoved(Plasma::Applet *applet, AbstractGroup *group);
        void onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *group);
        void onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *group);
        void onWidgetStartsMoving(QGraphicsWidget *widget);
        void onImmutabilityChanged(Plasma::ImmutabilityType immutability);
        void onChildrenRestored();
        void adjustCells();

    private:
        void addItem(QGraphicsWidget *widget, int row, int column, bool update = true);
        void removeItem(QGraphicsWidget *item, bool fillLayout = true);
        Position itemPosition(QGraphicsWidget *item) const;
        void setChildBorders(QGraphicsWidget *widget);
        int isOnAColumnBorder(qreal x) const;
        int isOnARowBorder(qreal y) const;
        void saveCellsInfo();

        void insertColumnAt(int column);
        void removeColumnAt(int column);
        void insertRowAt(int row);
        void removeRowAt(int row);

        Spacer *m_spacer;
        QList<QList<QGraphicsWidget *> > m_children;
        QMap<Plasma::Applet *, Plasma::Applet::BackgroundHints> m_savedAppletsHints;
        QMap<AbstractGroup *, AbstractGroup::BackgroundHints> m_savedGroupsHints;
        QList<qreal> m_rowHeights;
        QList<qreal> m_rowY;
        QList<qreal> m_columnWidths;
        QList<qreal> m_columnX;
        int m_movingColumn;
        int m_movingRow;
        bool m_cursorOverriden;

        friend class Spacer;
};

#endif // GRIDGROUP_H
