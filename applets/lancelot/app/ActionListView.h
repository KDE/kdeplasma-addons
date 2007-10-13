/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free 
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LISTVIEW_H_
#define LISTVIEW_H_

#include <QtGui>
#include <QtCore>
#include <KDebug>
#include <plasma/widgets/widget.h>
#include "ExtenderButton.h"

namespace Lancelot
{

class ActionListViewModel: public QObject {
    Q_OBJECT
public:
    ActionListViewModel() {};
    virtual ~ActionListViewModel() {};
    
    virtual QString title(int index) const = 0;
    virtual QString description(int index) const { Q_UNUSED(index); return QString(); };
    virtual QIcon * icon(int index) const = 0;
    virtual int size() const = 0;
    
Q_SIGNALS:
    void updated();
};    

class ActionListView : public Plasma::Widget
{
    Q_OBJECT
public:
    
    
    ActionListView(ActionListViewModel * model = 0, QGraphicsItem * parent = 0);
    virtual ~ActionListView();
    
    void setModel(ActionListViewModel * model);
    ActionListViewModel * model();
    
    void setItemHeight(int height);
    int itemHeight();
    void paintWidget ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget );
    
    void setExtenderPosition(ExtenderButton::ExtenderPosition position);
    ExtenderButton::ExtenderPosition extenderPosition();
    
    void setGeometry (const QRectF & geometry);

    virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );

Q_SIGNALS:
    void activated(int index);
    
protected slots:
    void scrollTimer();
    
protected:
    enum ScrollDirection {
        UP = 1, NO = 0, DOWN = -1
    };

    class ScrollButton : public Lancelot::BaseWidget {
    public:
        ScrollButton (ActionListView * list, ActionListView::ScrollDirection direction)
          : BaseWidget("", "", "", list), m_list(list), m_direction(direction) {
            m_svg = new Plasma::Svg("lancelot/action_list_view");
            m_svg->setContentType(Plasma::Svg::ImageSet);
            m_svgElementPrefix = QString((m_direction == UP)?"up":"down") + "_scroll_";
            setAcceptsHoverEvents(true);
        };
        void hoverEnterEvent ( QGraphicsSceneHoverEvent * event) {
            m_list->scroll(m_direction);
            BaseWidget::hoverEnterEvent(event);
        };
        void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event) {
            m_list->scroll(ActionListView::NO);
            BaseWidget::hoverLeaveEvent(event);
        };
        //void hide() {};
        //void show() {};
    private:
        ActionListView * m_list;
        ActionListView::ScrollDirection m_direction;
    };
    
    qreal m_topButtonVirtualY;
    QTransform m_topButtonScale;
    QTransform m_bottomButtonScale;
    
    void scroll(ScrollDirection direction);
    ScrollDirection m_scrollDirection;
    QTimer m_scrollTimer;
    int m_scrollInterval;
    
    ActionListViewModel * m_model;
    int m_itemHeight;
    
    ExtenderButton::ExtenderPosition m_extenderPosition;

    void deleteButtons();
    void createNeededButtons(bool fullReload = false);
    ExtenderButton * createButton();
    
    void shiftButtonList(ScrollDirection direction);
    
    QLinkedList < ExtenderButton * > m_buttons;
    
    int m_firstButtonIndex;
    
    void positionScrollButtons();
    ScrollButton * buttonUp, * buttonDown;
    
    friend class ScrollButton;
};

}

#endif /*LISTVIEW_H_*/
