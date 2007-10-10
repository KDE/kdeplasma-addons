/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "ActionListView.h"
#include <QLinkedListIterator>
#include <cmath>
#include <KDebug>
#include <cstdlib>

#define min(A, B) (((A) < (B)) ? (A) : (B))

#define SCROLL_BUTTON_WIDTH 66
#define SCROLL_BUTTON_HEIGHT 19

#define SCROLL_AMMOUNT 4
#define SCROLL_INTERVAL 10

namespace Lancelot {

ActionListView::ActionListView(ActionListViewModel * model,
    QGraphicsItem * parent) :
    Plasma::Widget(parent), m_topButtonVirtualY(0), m_scrollDirection(NO), m_model(model), m_itemHeight(50),
    m_extenderPosition(ExtenderButton::RIGHT), m_firstButtonIndex(0)
            
{
    createNeededButtons();
    setAcceptsHoverEvents(true);

    buttonUp = new ScrollButton(this, UP);
    buttonUp->resize(SCROLL_BUTTON_WIDTH, SCROLL_BUTTON_HEIGHT);
    buttonUp->setZValue(100);
    buttonUp->hide();

    buttonDown = new ScrollButton(this, DOWN);
    buttonDown->resize(SCROLL_BUTTON_WIDTH, SCROLL_BUTTON_HEIGHT);
    buttonDown->setZValue(100);
    buttonDown->hide();
    
    positionScrollButtons();
    
    connect ( & m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimer()));
    m_scrollTimer.setSingleShot(false);
    
}

ActionListView::~ActionListView()
{
    deleteButtons();
}

void ActionListView::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
    Q_UNUSED(event);
	QGraphicsItem * child;
	foreach(child, QGraphicsItem::children()) {
		child->setEnabled(true);
	}
}

void ActionListView::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
	QGraphicsItem * child;
	foreach(child, QGraphicsItem::children()) {
		((Lancelot::ExtenderButton * )child)->hoverLeaveEvent(event);
		child->setEnabled(false);
	}
}


void ActionListView::positionScrollButtons() {
    float left = (size().width() - EXTENDER_SIZE - SCROLL_BUTTON_WIDTH) / 2;
    if (m_extenderPosition == ExtenderButton::LEFT) left += EXTENDER_SIZE;
    buttonUp->setPos(left, 0);
    buttonDown->setPos(left, size().height() - SCROLL_BUTTON_HEIGHT);
}

Lancelot::ExtenderButton * ActionListView::createButton()
{
    Lancelot::ExtenderButton * button = new Lancelot::ExtenderButton(
            "", "", "", this);
    button->resize(size().width() - EXTENDER_SIZE, m_itemHeight);
    button->setInnerOrientation(Lancelot::ExtenderButton::HORIZONTAL);
    button->setExtenderPosition(Lancelot::ExtenderButton::RIGHT);
    button->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    return button;
}

void ActionListView::scroll(ScrollDirection direction)
{
    m_scrollDirection = direction;
    if (direction != NO) {
        m_scrollTimer.start(m_scrollInterval = SCROLL_INTERVAL);
    } else {
        m_scrollTimer.stop();
    }     
}

void ActionListView::shiftButtonList(ScrollDirection direction) {
	if (direction == UP) {
	    if (m_firstButtonIndex <= 0) {
	    	buttonUp->setVisible(false);
	    	scroll(NO);
	    	return;
	    }

	    m_buttons.prepend(m_buttons.takeLast());
		m_topButtonVirtualY -= m_itemHeight;

	    m_firstButtonIndex --;
	    m_buttons.first()->setTitle(m_model->title(m_firstButtonIndex));
	    m_buttons.first()->setDescription(m_model->description(m_firstButtonIndex));
	    m_buttons.first()->setIcon(m_model->icon(m_firstButtonIndex));
		m_buttons.first()->setVisible(true);
	    
	} else {
	    if (m_firstButtonIndex + m_buttons.size() > m_model->size()) {
	    	buttonDown->setVisible(false);
	    	scroll(NO);
	    	return;
	    }
	    
		m_buttons.append(m_buttons.takeFirst());
		m_topButtonVirtualY += m_itemHeight;

		if (m_firstButtonIndex + m_buttons.size() < m_model->size()) {
	        m_buttons.last()->setTitle(m_model->title(m_firstButtonIndex + m_buttons.size()));
	        m_buttons.last()->setDescription(m_model->description(m_firstButtonIndex + m_buttons.size()));
	        m_buttons.last()->setIcon(m_model->icon(m_firstButtonIndex + m_buttons.size()));
		} else {
			m_buttons.last()->setVisible(false);
		}
	    m_firstButtonIndex ++;
	}
}


void ActionListView::scrollTimer()
{
    if (m_buttons.size() <= 1 || m_scrollDirection == NO) return;
    
    m_topButtonVirtualY += SCROLL_AMMOUNT * m_scrollDirection;
    
    if (m_topButtonVirtualY > 0) {
    	shiftButtonList(UP);
    } else if (m_topButtonVirtualY <= - m_itemHeight) {
    	shiftButtonList(DOWN);
    }

    if (m_scrollDirection == UP) {
        buttonDown->setVisible(true);
    } else if (m_scrollDirection == DOWN) {
    	buttonUp->setVisible(true);
    }
    
    Lancelot::ExtenderButton * button;
    int index = 0;
    m_topButtonScale.reset();
    
    foreach (button, m_buttons) {
        button->setTransform(m_topButtonScale);
    	if (index == 0) {
            button->setPos(button->pos().x(), 0);
    	} else {
            button->setPos(button->pos().x(), m_itemHeight * index + m_topButtonVirtualY);
    	}
      	index++;
    }
    
    qreal scale = 1 + (m_topButtonVirtualY) / m_itemHeight;
    if (scale > 1) scale = 1;
    m_topButtonScale.reset();
   	m_topButtonScale.scale(1.0, scale);
   	m_buttons.first()->setTransform(m_topButtonScale);
   	
   	QLinkedListIterator< Lancelot::ExtenderButton * > i(m_buttons);
   	i.toBack();
   	
   	button = i.previous();
   	if (button->pos().y() > size().height()) {
   		button->setTransform(QTransform().scale(1, 0));
   	   	button = i.previous();
   	}
   	scale = (size().height() - button->pos().y()) / m_itemHeight;
    button->setTransform(QTransform().scale(1, scale));
    
}

void ActionListView::deleteButtons()
{
    while (m_buttons.size() > 0) {
        delete m_buttons.takeFirst();
    }
}

void ActionListView::createNeededButtons(bool fullReload)
{
    if (fullReload || !m_model)
        deleteButtons();
    
    if (!m_model) return;

    int noOfButtons = min(lround(ceil(size().height() / m_itemHeight) + 1), m_model->size());
    
    if (lround(floor(size().height() / m_itemHeight)) < m_model->size()) {
        buttonDown->show();
    }

    Lancelot::ExtenderButton * button;
    foreach (button, m_buttons) {
        button->resize(size().width() - EXTENDER_SIZE, m_itemHeight);
    }

    if (noOfButtons > m_buttons.size()) {
        for (int i = noOfButtons - m_buttons.size(); i > 0; i--) {
            float top;
            if (m_buttons.isEmpty())
                top = 0;
            else
                top = m_buttons.last()->pos().y() + m_buttons.last()->size().height();
            m_buttons.append(createButton());
            
            int modelIndex = m_buttons.size() - 1 + m_firstButtonIndex;
            m_buttons.last()->setTitle(m_model->title(modelIndex));
            m_buttons.last()->setDescription(m_model->description(modelIndex));
            m_buttons.last()->setIcon(m_model->icon(modelIndex));

            if (m_extenderPosition == ExtenderButton::RIGHT)
                m_buttons.last()->setPos(0, top);
            else
                m_buttons.last()->setPos(EXTENDER_SIZE, top);

            m_buttons.last()->setExtenderPosition(m_extenderPosition);
        }
    } else if (noOfButtons < m_buttons.size()) {
        for (int i = noOfButtons - m_buttons.size(); i > 0; i--) {
            delete m_buttons.takeLast();
        }
    }
    
}

void ActionListView::setExtenderPosition(
        ExtenderButton::ExtenderPosition position)
{
    if (m_extenderPosition == position)
        return;
    if (position == ExtenderButton::TOP)
        position = ExtenderButton::LEFT;
    if (position == ExtenderButton::BOTTOM)
        position = ExtenderButton::RIGHT;

    Lancelot::ExtenderButton * button;
    foreach (button, m_buttons) {
        button->setExtenderPosition(position);
        if (position == ExtenderButton::RIGHT)
        button->setPos(0, button->pos().y());
        else
        button->setPos(EXTENDER_SIZE, button->pos().y());

    }

    m_extenderPosition = position;
}

ExtenderButton::ExtenderPosition ActionListView::extenderPosition()
{
    return m_extenderPosition;
}

void ActionListView::setGeometry(const QRectF & geometry)
{
    Plasma::Widget::setGeometry(geometry);
    createNeededButtons();
    positionScrollButtons();
}

void ActionListView::setModel(ActionListViewModel * model)
{
    m_topButtonVirtualY = 0;
    m_scrollDirection = NO;
    m_firstButtonIndex = 0;    
    
    /* Weird stuff happens when the model is changed and
     * the listview is hidden - scroll buttons appear
     * although the parent is hidden */
    //bool lv = isVisible();
    //if (!lv) setVisible(true);
    m_model = model;
    createNeededButtons(true);
    //if (!lv) setVisible(false);
    update();
}

ActionListViewModel * ActionListView::model()
{
    return m_model;
}

void ActionListView::setItemHeight(int height)
{
    m_itemHeight = height;
}

int ActionListView::itemHeight()
{
    return m_itemHeight;
}

void ActionListView::paintWidget(QPainter * painter,
        const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    // TODO: Comment the next line
    painter->fillRect(QRectF(QPointF(0, 0), size()), QBrush(QColor(100, 100, 200, 100)));
}

}

#include "ActionListView.moc"
