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

#include "ActionListView.h"
#include <QLinkedListIterator>
#include <cmath>
#include <KDebug>
#include <KIcon>

#define SCROLL_BUTTON_WIDTH 66
#define SCROLL_BUTTON_HEIGHT 19

#define SCROLL_AMMOUNT 4
#define SCROLL_INTERVAL 10
#define MAX_LIMBO_SIZE 5

namespace Lancelot {

// ActionListView::ScrollButton

ActionListView::ScrollButton::ScrollButton (ActionListView::ScrollDirection direction, ActionListView * list, QGraphicsItem * parent)
  : BaseActionWidget(list->name() + "::" + QString((direction == Up)?"up":"down"), "", "", parent), m_list(list), m_direction(direction)
{
    m_svg = new Plasma::Svg("lancelot/action_list_view");
    m_svg->setContentType(Plasma::Svg::ImageSet);
    m_svgElementPrefix = QString((m_direction == Up)?"up":"down") + "_scroll_";
    setAcceptsHoverEvents(true);
}

void ActionListView::ScrollButton::hoverEnterEvent (QGraphicsSceneHoverEvent * event)
{
    if (m_hover) return;
    m_list->scroll(m_direction);
    BaseActionWidget::hoverEnterEvent(event);
}

void ActionListView::ScrollButton::hoverLeaveEvent (QGraphicsSceneHoverEvent * event)
{
    if (!m_hover) return;
    m_list->scroll(ActionListView::No);
    BaseActionWidget::hoverLeaveEvent(event);
}

// ActionListView
ActionListView::ActionListView(QString name, ActionListViewModel * model, QGraphicsItem * parent)
  : Widget(name, parent), m_model(NULL),
    m_minimumItemHeight(32), m_maximumItemHeight(64), m_preferredItemHeight(48), m_categoryItemHeight(24),
    m_extenderPosition(ExtenderButton::No), scrollButtonUp(NULL), scrollButtonDown(NULL),
    m_scrollDirection(No), m_scrollInterval(0), m_topButtonIndex(0), m_initialButtonsCreationRunning(false)
{
    setAcceptsHoverEvents(true);
    kDebug() << "Setting the model\n";
    setModel(model);
    kDebug() << "Model is set\n";

    connect ( & m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimer()));
    m_scrollTimer.setSingleShot(false);
    
}

ActionListView::~ActionListView()
{
    deleteAllButtons();
    delete scrollButtonUp;
    delete scrollButtonDown;
}

void ActionListView::positionScrollButtons()
{
    kDebug() << name() << " " << geometry() << " Positioning\n";
    if (!scrollButtonUp) {
        // Call from constructor - create and init objects
        
        scrollButtonUp = new ScrollButton(Up, this);
        scrollButtonDown = new ScrollButton(Down, this);
    
        addChild(scrollButtonUp);
        scrollButtonUp->resize(SCROLL_BUTTON_WIDTH, SCROLL_BUTTON_HEIGHT);
        scrollButtonUp->setZValue(100);
        scrollButtonUp->show();
    
        addChild(scrollButtonDown);
        scrollButtonDown->resize(SCROLL_BUTTON_WIDTH, SCROLL_BUTTON_HEIGHT);
        scrollButtonDown->setZValue(100);
        scrollButtonDown->show();
    }

    float left = (size().width() - EXTENDER_SIZE - SCROLL_BUTTON_WIDTH) / 2;
    if (m_extenderPosition == ExtenderButton::Left) left += EXTENDER_SIZE;
    
    scrollButtonUp->setPos(left, 0);
    scrollButtonDown->setPos(left, size().height() - SCROLL_BUTTON_HEIGHT);
}

void ActionListView::scroll(ScrollDirection direction)
{
    m_scrollDirection = direction;
    if (direction != No) {
        m_scrollTimer.start(m_scrollInterval = SCROLL_INTERVAL);
    } else {
        m_scrollTimer.stop();
    }     
}

void ActionListView::scrollTimer() {
    if (m_scrollDirection == No || !m_model || m_buttons.size() == 0) {
        m_scrollTimer.stop();
        return;
    }
    
    if (m_scrollDirection == Up && m_topButtonIndex == 0 && m_buttons.first().second >= 0) {
        scroll(No);
    }
    
    int scrollAmmount = m_scrollDirection * SCROLL_AMMOUNT;
    
    
    kDebug() << m_buttons.first().second << " << this is the top of 1st btn\n";
    QPair < ExtenderButton *, int > pair;
    QMutableListIterator< QPair < ExtenderButton *, int > > i(m_buttons);
    while (i.hasNext()) {
        pair = i.next();
        pair.second += scrollAmmount;
        pair.first->setGeometry(
            pair.first->geometry().translated(0, scrollAmmount)
        );
        i.setValue(pair);
    }
    kDebug() << m_buttons.first().second << " << this is the top of 1st btn, after the move\n";
    
    //if (pair.)
}

void ActionListView::setGeometry(const QRectF & geometry)
{
    if (!geometry.isValid() || geometry.isEmpty() || this->geometry() == geometry) {
        return;
    }

    Widget::setGeometry(geometry);
    positionScrollButtons();

    if (!m_model) return;
    
    calculateItemHeight();
    
    initialButtonsCreation();
}

void ActionListView::setModel(ActionListViewModel * model)
{
    if (!model) return;
    m_model = model;
    initialButtonsCreation();
    update();
}

ActionListViewModel * ActionListView::model()
{
    return m_model;
}

void ActionListView::modelUpdated() {};
void ActionListView::modelItemInserted(int index) {};
void ActionListView::modelItemDeleted(int index) {};
void ActionListView::modelItemAltered(int index) {};

void ActionListView::paintWidget(QPainter * painter,
        const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    // TODO: Comment the next line
    painter->fillRect(QRectF(QPointF(0, 0), size()), QBrush(QColor(100, 100, 200, 100)));
}

void ActionListView::setExtenderPosition(ExtenderButton::ExtenderPosition position)
{
    m_extenderPosition = position;
    // TODO: Implement
}

ExtenderButton::ExtenderPosition ActionListView::extenderPosition()
{
    return m_extenderPosition;
}

Lancelot::ExtenderButton * ActionListView::createButton()
{
    Lancelot::ExtenderButton * button;
    
    if (!m_buttonsLimbo.empty()) {
        button = m_buttonsLimbo.takeFirst();
        button->show();
    } else {    
        button = new Lancelot::ExtenderButton(m_name + "Button",
            new KIcon("system-lock-screen"), "Caption", "Decr", this);
    
        button->setInnerOrientation(Lancelot::ExtenderButton::Horizontal);
        button->setExtenderPosition(m_extenderPosition);
        button->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return button;
}

void ActionListView::deleteButton(ListTail where)
{
    ExtenderButton * button;
    if (where == Start) {
        button = m_buttons.takeFirst().first;
    } else {
        button = m_buttons.takeLast().first;
    }

    if (m_buttonsLimbo.count() >= MAX_LIMBO_SIZE) {
        delete button;
        return;
    } else {
        m_buttonsLimbo.append(button);
        button->hide();
    }
}

void ActionListView::deleteAllButtons()
{
    while (!m_buttons.empty()) {
        delete m_buttons.takeLast().first;
    }
    while (!m_buttonsLimbo.empty()) {
        delete m_buttonsLimbo.takeFirst();
    }
}

void ActionListView::initialButtonsCreation() {
    if (m_initialButtonsCreationRunning) return;
    m_initialButtonsCreationRunning = true;
    
    deleteAllButtons();
    if (!m_model) return;

    
    int listHeight = qRound(geometry().height());
    
    kDebug() << name() << "'s height is " << listHeight << "\n adding one button...";
    
    if (!addButton(End)) return; // The model is empty or something else is wrong
    
    bool buttonCreated;    
    while (
        (m_buttons.last().second < listHeight)
        && (m_buttons.size() <= m_model->size())
        && (buttonCreated = addButton(End))) {
    }
    // If buttonCreated is true, it means that we haven't reached
    // the end of the model, but we have made one more button than we need
    // and that button is outside the listview. Move it to the limbo
    // for further use.
    if (buttonCreated) deleteButton(End);
    
    m_initialButtonsCreationRunning = false;
}

bool ActionListView::addButton(ListTail where) {
    if (m_buttons.empty()) {
        where = End;
    }
    
    kDebug() << "Get index (in model) of the element that is to be added\n";
    // Get index (in model) of the element that is to be added
    int itemIndex;
    if (where == Start) {
        itemIndex = m_topButtonIndex - 1;
        if (itemIndex < 0) return false;
    } else {
        itemIndex = m_topButtonIndex + m_buttons.size();
        if (itemIndex >= m_model->size()) return false;
    }
    kDebug() << "....... " << itemIndex << "\n";
    
    // Create and initialize the button for the item
    ExtenderButton * button = createButton();
    kDebug() << "init button " << (long)button << "\n";
    button->setTitle(m_model->title(itemIndex));
    button->setDescription(m_model->description(itemIndex));
    button->setIcon(m_model->icon(itemIndex));
    kDebug() << "setting layout\n";
    button->setLayout(NULL);
    kDebug() << "init end " << itemIndex << "\n";
    
    int itemHeight = ((m_model->isCategory(itemIndex)) ? m_categoryItemHeight : m_currentItemHeight);

    // Place the button where needed
    int buttonTop;
    if (m_buttons.empty()) {
        // If the buttons list is empty, we are adding the element on the top
        buttonTop = 0;
        m_buttons.append(QPair < ExtenderButton *, int > (button, buttonTop));
        
    } else if (where == Start) {
        // Adding the element in front of the first element
        buttonTop = m_buttons.first().second - itemHeight;
        m_buttons.prepend(QPair < ExtenderButton *, int > (button, buttonTop));
        --m_topButtonIndex;
        
    } else {
        // Adding the element after the last element
        buttonTop = m_buttons.last().second +
            ((m_model->isCategory(m_topButtonIndex + m_buttons.size() - 1)) ? m_categoryItemHeight : m_currentItemHeight)
        ;
        m_buttons.append(QPair < ExtenderButton *, int > (button, buttonTop));
        
    }
    
    button->setGeometry(QRectF(0, buttonTop, size().width() - EXTENDER_SIZE, itemHeight));
    kDebug() << "Button added\n";
    return true;
}

int ActionListView::calculateItemHeight()
{
    if (!m_model) return m_currentItemHeight = 0;
    
    int listHeight = qRound(geometry().height());
    int categoriesHeight = 0, items = 0;
    
    for (int i = 0; i < m_model->size(); i++) {
        if (m_model->isCategory(i)) {
            categoriesHeight += m_categoryItemHeight;
        } else {
            ++items;
        }
        if (categoriesHeight + items * m_minimumItemHeight > listHeight) {
            return m_currentItemHeight = m_preferredItemHeight;
        }
    }
    
    // items is from now on projected height for an item
    items = (listHeight - categoriesHeight) / items;
    
    if (items > m_maximumItemHeight)
        return m_currentItemHeight = m_maximumItemHeight;

    return m_currentItemHeight = items;
}


void ActionListView::setMinimumItemHeight(int height) { m_minimumItemHeight = height; }
int ActionListView::minimumItemHeight() { return m_minimumItemHeight; }

void ActionListView::setMaximumItemHeight(int height) { m_maximumItemHeight = height; }
int ActionListView::maximumItemHeight() { return m_maximumItemHeight; }

void ActionListView::setPreferredItemHeight(int height) { m_preferredItemHeight = height; }
int ActionListView::preferredItemHeight() { return m_preferredItemHeight; }

void ActionListView::setCategoryItemHeight(int height) { m_categoryItemHeight = height; }
int ActionListView::categoryItemHeight() { return m_categoryItemHeight; }

}

#include "ActionListView.moc"
