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
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        static QString prettyName();
        static QSet<Plasma::FormFactor> availableOnFormFactors();

    protected:
        void layoutChild(QGraphicsWidget *child, const QPointF &pos);
        void resizeEvent(QGraphicsSceneResizeEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        bool eventFilter(QObject *obj, QEvent *event);
        void constraintsEvent(Plasma::Constraints constraints);

    private slots:
        void onAppletAdded(Plasma::Applet *applet, AbstractGroup *group);
        void onAppletRemoved(Plasma::Applet *applet, AbstractGroup *group);
        void onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *group);
        void onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *group);
        void onWidgetStartsMoving(QGraphicsWidget *widget);
        void onChildrenRestored();
        void adjustCells();

    private:
        bool addItem(QGraphicsWidget *widget, int row, int column);
        void removeItem(QGraphicsWidget *item, bool fillLayout = true);
        Position itemPosition(QGraphicsWidget *item) const;
        void setChildBorders(Plasma::Applet *applet, bool added);
        void setChildBorders(AbstractGroup *group, bool added);
        int isOnAColumnBorder(qreal x, int space = 0) const;
        int isOnARowBorder(qreal y, int space = 0) const;
        void saveCellsInfo();

        void insertColumnAt(int column);
        void removeColumnAt(int column);
        void insertRowAt(int row);
        void removeRowAt(int row);

        Spacer *m_spacer;
        QList<QList<QGraphicsWidget *> > m_children; //the inner list is the column, the outer one the row
                                                     // |m_children.at(0).at(0)|m_children.at(0).at(1)|
                                                     // |m_children.at(1).at(0)|m_children.at(1).at(1)|
        QMap<Plasma::Applet *, Plasma::Applet::BackgroundHints> m_savedAppletsHints;
        QMap<AbstractGroup *, AbstractGroup::BackgroundHints> m_savedGroupsHints;
        QList<qreal> m_rowHeights;
        QList<qreal> m_rowY;
        QList<qreal> m_columnWidths;
        QList<qreal> m_columnX;
        int m_movingColumn;
        int m_movingRow;
        bool m_cursorOverriden;
        Plasma::Svg *m_separator;

        friend class Spacer;
};

#endif // GRIDGROUP_H
