/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2009 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef GROUPICONLIST_H
#define GROUPICONLIST_H

#include <QtGui/QGraphicsWidget>

#include <Plasma/Plasma>

class QGraphicsLinearLayout;

namespace Plasma
{
    class Animation;
    class ItemBackground;
    class ToolButton;
    class Svg;
}

class GroupIcon;

class GroupIconList : public QGraphicsWidget
{
    Q_OBJECT
    public:
        explicit GroupIconList(Plasma::Location location = Plasma::BottomEdge, QGraphicsItem *parent = 0);
        ~GroupIconList();

        void setLocation(Plasma::Location location);
        Plasma::Location location();
        void setIconSize(int size);
        int iconSize() const;

    public Q_SLOTS:
        void updateList();

    protected:
        void wheelEvent(QGraphicsSceneWheelEvent *event);
        bool eventFilter(QObject *obj, QEvent *event);

    private Q_SLOTS:
        //checks if arrows should be enabled or not
        void manageArrows();
        void scrollDownRight();
        void scrollUpLeft();
        void scrollStepFinished();
        //moves list to position 0,0
        void resetScroll();
        void iconHoverEnter(GroupIcon *icon);

    private:
        void addIcon(GroupIcon *applet);
        void scrollTo(int index);
        //see how many icons is visible at once, approximately
        int maximumAproxVisibleIconsOnList();
        //removes all the icons from the widget
        void eraseList();
        //returns the what's the visible rect of the list widget
        QRectF visibleListRect();
        //returns window's start position
        qreal visibleStartPosition();
        //returns window's end position
        qreal visibleEndPosition();
        //returns list size
        qreal listSize();
        //returns windows size relative to list
        qreal windowSize();
        //returns item position
        qreal itemPosition(int i);

        QList<GroupIcon *> m_items;
        QGraphicsLinearLayout *m_listLayout;
        QGraphicsWidget *m_scrollWidget;
        QGraphicsWidget *m_listWidget;
        QGraphicsLinearLayout *m_arrowsLayout;

        Plasma::ToolButton *m_downRightArrow;
        Plasma::ToolButton *m_upLeftArrow;
        Plasma::Svg *m_arrowsSvg;

        Qt::Orientation m_orientation;
        Plasma::Location m_location;
        Plasma::ItemBackground *m_hoverIndicator;

        int m_firstItemIndex;

        int m_scrollStep;
        int m_iconSize;
        bool m_scrollingDueToWheel;

        Plasma::Animation *m_slide;
};

#endif //ICONLIST_H
