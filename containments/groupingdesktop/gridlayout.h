/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include <QtGui/QGraphicsWidget>

class QGraphicsGridLayout;

namespace Plasma {
    class Applet;
    class FrameSvg;
};

struct Position {
    int row;
    int column;
};

class Spacer;

class GridLayout : public QGraphicsWidget
{
    Q_OBJECT
    public:
        GridLayout(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
        ~GridLayout();

        void assignApplet(Plasma::Applet *applet, int row, int column);
        void assignApplet(Plasma::Applet *applet, const QPointF &pos);
        Position itemPosition(QGraphicsWidget *item);
        QList<Plasma::Applet *> assignedApplets();

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

    protected:
        void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
        void dropEvent(QGraphicsSceneDragDropEvent *event);
        void resizeEvent(QGraphicsSceneResizeEvent *event);

    private:
        enum Orientation {
            Horizontal = 0,
            Vertical = 1
        };

        void insertItemAt(QGraphicsWidget *item, int row, int column, Orientation orientation);
        void removeItemAt(int row, int column);
        QGraphicsLayoutItem *takeItemAt(int row, int column);
        int nearestBoundair(qreal pos, qreal size);
        void showItemDropZone(QGraphicsWidget *widget, const QPointF &pos);

        QGraphicsGridLayout *m_layout;
        QList<Plasma::Applet *> m_applets;
        Plasma::FrameSvg *m_background;
        Spacer *m_spacer;

        friend class Spacer;
};

#endif // GRIDLAYOUT_H
