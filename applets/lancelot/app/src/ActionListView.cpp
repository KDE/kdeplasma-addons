/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
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

#define SCROLL_AMMOUNT 6
#define SCROLL_INTERVAL 10
#define MAX_LIMBO_SIZE 5

namespace Lancelot {

// ActionListView::ScrollButton

#define itemHeightFromIndex(A) ((m_model->isCategory(A)) ? m_categoryItemHeight : m_currentItemHeight)

ActionListView::ScrollButton::ScrollButton (ActionListView::ScrollDirection direction, ActionListView * list, QGraphicsItem * parent)
  : BaseActionWidget(list->name() + "::" + QString((direction == Up)?"up":"down"), "", "", parent), m_list(list), m_direction(direction)
{
    setAcceptsHoverEvents(true);
}

void ActionListView::ScrollButton::hoverEnterEvent (QGraphicsSceneHoverEvent * event)
{
    if (m_hover) return;
    m_list->m_scrollTimes = -1;
    m_list->scroll(m_direction);
    BaseActionWidget::hoverEnterEvent(event);
}

void ActionListView::ScrollButton::hoverLeaveEvent (QGraphicsSceneHoverEvent * event)
{
    if (!m_hover) return;
    m_list->m_scrollTimes = -1;
    m_list->scroll(ActionListView::No);
    BaseActionWidget::hoverLeaveEvent(event);
}

// ActionListView
ActionListView::ActionListView(QString name, ActionListViewModel * model, QGraphicsItem * parent)
  : Widget(name, parent), m_model(NULL),
    m_minimumItemHeight(32), m_maximumItemHeight(64), m_preferredItemHeight(48), m_categoryItemHeight(24),
    m_extenderPosition(ExtenderButton::No), scrollButtonUp(NULL), scrollButtonDown(NULL),
    m_scrollDirection(No), m_scrollInterval(0), m_scrollTimes(-1), m_topButtonIndex(0), m_signalMapper(this),
    m_initialButtonsCreationRunning(false)
{
    setGroupByName("ActionListView");
    m_itemsGroup = instance()->group("ActionListView-Items");

    setAcceptsHoverEvents(true);

    setModel(model);

    connect(&m_signalMapper, SIGNAL(mapped(int)),
                 this, SLOT(itemActivated(int)));

    connect ( & m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimer()));
    m_scrollTimer.setSingleShot(false);
}

void ActionListView::itemActivated(int index) {
    if (!m_model) return;
    m_model->activated(index);
    emit activated(index);
}

ActionListView::~ActionListView()
{
    deleteAllButtons();
    delete scrollButtonUp;
    delete scrollButtonDown;
}

void ActionListView::positionScrollButtons()
{
    if (!scrollButtonUp) {
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

        scrollButtonUp->setGroupByName(m_group->name() + "-Scroll-Up");
        scrollButtonDown->setGroupByName(m_group->name() + "-Scroll-Down");

    }

    float left = (size().width() - EXTENDER_SIZE - SCROLL_BUTTON_WIDTH) / 2;
    if (m_extenderPosition == ExtenderButton::Left) left += EXTENDER_SIZE;

    scrollButtonUp->setPos(left, 0);
    scrollButtonDown->setPos(left, size().height() - SCROLL_BUTTON_HEIGHT);
}

void ActionListView::setGroup(WidgetGroup * group)
{
    Widget::setGroup(group);
    if (scrollButtonUp) {
        scrollButtonUp->setGroupByName(m_group->name() + "-Scroll-Up");
        scrollButtonDown->setGroupByName(m_group->name() + "-Scroll-Down");
    }
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
    scrollBy(m_scrollDirection * SCROLL_AMMOUNT);
}

void ActionListView::scrollBy(int scrollAmmount) {
    if (!m_model || m_buttons.size() == 0 || m_scrollTimes == 0) {
        m_scrollTimer.stop();
        return;
    }

    if (m_scrollTimes > 0) --m_scrollTimes;

    if (m_scrollDirection == Up && m_buttons.first().second >= 0) {
        if (m_topButtonIndex <= 0) {
            scroll(No);
            if (scrollButtonUp) {
                scrollButtonUp->setOpacity(0.3);
            }
            return;
        } else {
            m_buttons.first().first->setTransform(QTransform());
            addButton(Start);
        }
    }

    if (m_scrollDirection == Down
        && itemHeightFromIndex(m_topButtonIndex + m_buttons.size() - 1)
            + m_buttons.last().second <= geometry().height()) {
        if (m_topButtonIndex + m_buttons.size() >= m_model->size()) {
            scroll(No);
            if (scrollButtonDown) {
                scrollButtonDown->setOpacity(0.3);
            }
            return;
        } else {
            m_buttons.last().first->setTransform(QTransform());
            addButton(End);
        }
    }

    if (scrollButtonUp) {
        scrollButtonUp->setOpacity(1);
        scrollButtonDown->setOpacity(1);
    }


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

    // Delete buttons that are not needed
    while (m_buttons.size() > 1) {
        if (m_buttons.at(1).second <= 0) {
            deleteButton(Start);
        } else break;
    }

    while (m_buttons.size() > 1) {
        if (m_buttons.last().second >= geometry().height()) {
            deleteButton(End);
        } else break;
    }

    // Scale first and last button as needed
    int partHeight = m_buttons.first().second;
    int height = itemHeightFromIndex(m_topButtonIndex);
    qreal scale = (qreal)(height + partHeight) / height;
    m_topButtonScale.reset();
    m_topButtonScale.translate(0, - partHeight);
    m_topButtonScale.scale(1, scale);

    m_buttons.first().first->setOpacity(scale);
    m_buttons.first().first->setTransform(m_topButtonScale);

    /*int*/ partHeight = qRound(geometry().height()) - m_buttons.last().second;
    /*int*/ height = itemHeightFromIndex(m_topButtonIndex + m_buttons.size() - 1);
    /*qreal*/ scale = qMin(qreal(1.0), (qreal)(partHeight) / height);
    m_bottomButtonScale.reset();
    m_bottomButtonScale.scale(1, scale);

    m_buttons.last().first->setOpacity(scale);
    m_buttons.last().first->setTransform(m_bottomButtonScale);

}

void ActionListView::setGeometry(const QRectF & geometry)
{
    if (!geometry.isValid() || geometry.isEmpty() || this->geometry() == geometry) {
        return;
    }

    Widget::setGeometry(geometry);
    positionScrollButtons();

    if (!m_model) return;

    initialButtonsCreation();
}

void ActionListView::setModel(ActionListViewModel * model)
{
    if (!model) return;

    if (m_model) {
        // disconnencting from old model
        disconnect ( m_model, 0, this, 0 );
    }

    m_model = model;

    connect(m_model, SIGNAL(updated()),
            this, SLOT(modelUpdated()));
    connect(m_model, SIGNAL(itemInserted(int)),
            this, SLOT(modelItemInserted(int)));
    connect(m_model, SIGNAL(itemDeleted(int)),
            this, SLOT(modelItemDeleted(int)));
    connect(m_model, SIGNAL(itemAltered(int)),
            this, SLOT(modelItemAltered(int)));

    initialButtonsCreation();
    update();
}

ActionListViewModel * ActionListView::model()
{
    return m_model;
}

// TODO: Model updates
void ActionListView::modelUpdated()
{
    initialButtonsCreation();
}

void ActionListView::modelItemInserted(int index)
{
    Q_UNUSED(index);
    modelUpdated();
}

void ActionListView::modelItemDeleted(int index)
{
    Q_UNUSED(index);
    modelUpdated();
}

void ActionListView::modelItemAltered(int index)
{
    // TODO: What if an item became a category, or vice versa?
    int buttonIndex = index - m_topButtonIndex;
    if (buttonIndex >= 0 && buttonIndex < m_buttons.size()) {
        m_buttons.at(buttonIndex).first->setTitle(m_model->title(index));
        m_buttons.at(buttonIndex).first->setDescription(m_model->description(index));
        m_buttons.at(buttonIndex).first->setIcon(m_model->icon(index));
    }
}

/*void ActionListView::paintWidget(QPainter * painter,
        const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    Q_UNUSED(painter);
    //painter->fillRect(QRectF(QPointF(0, 0), size()), QBrush(QColor(100, 100, 200, 100)));
}*/

void ActionListView::setItemsGroupByName(const QString & group) {
    setItemsGroup(instance()->group(group));
}

void ActionListView::setItemsGroup(WidgetGroup * group)
{
    if (group == NULL) {
        group = instance()->defaultGroup();
    }

    if (group == m_itemsGroup) return;

    m_itemsGroup = group;

    QPair < ExtenderButton *, int > pair;
    foreach(pair, m_buttons) {
        pair.first->setGroup(group);
    }

    ExtenderButton * button;
    foreach(button, m_buttonsLimbo) {
        button->setGroup(group);
    }
}

WidgetGroup * ActionListView::itemsGroup() {
    return m_itemsGroup;
}

void ActionListView::setExtenderPosition(ExtenderButton::ExtenderPosition position)
{
    if (position == ExtenderButton::Top) position = ExtenderButton::Left;
    if (position == ExtenderButton::Bottom) position = ExtenderButton::Right;

    if (m_extenderPosition == position) return;
    m_extenderPosition = position;

    int left = 0;
    qreal width = geometry().width();

    switch (position) {
    case ExtenderButton::Left:
        left = EXTENDER_SIZE;
        // break; // We neet to have width -= EXTENDER_SIZE for Left too
    case ExtenderButton::Right:
        width -= EXTENDER_SIZE;
        break;
    default:
        break;
    }

    QPair < ExtenderButton *, int > pair;
    foreach(pair, m_buttons) {
        pair.first->setExtenderPosition(position);
        pair.first->setGeometry(
            QRectF(left, pair.first->geometry().top(),
                   width, pair.first->geometry().height())
        );
    }

    ExtenderButton * button;
    foreach(button, m_buttonsLimbo) {
        button->setExtenderPosition(position);
    }
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
        button->enable();
        button->show();
    } else {
        button = new Lancelot::ExtenderButton(m_name + "Button",
            (QIcon *)NULL, "", "", this);

        button->setInnerOrientation(Lancelot::ExtenderButton::Horizontal);
        button->setExtenderPosition(m_extenderPosition);
        button->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        button->setGroup(m_itemsGroup);

        connect(button, SIGNAL(activated()), &m_signalMapper, SLOT(map()));
    }
    return button;
}

void ActionListView::deleteButton(ListTail where)
{
    ExtenderButton * button;
    if (where == Start) {
        button = m_buttons.takeFirst().first;
        ++m_topButtonIndex;
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
    calculateItemHeight();

    deleteAllButtons();
    if (!m_model) return;

    int listHeight = qRound(geometry().height());

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

    scrollTimer();
}

bool ActionListView::addButton(ListTail where) {
    if (!m_model) return false;

    if (m_buttons.empty()) {
        where = End;
    }

    // Get index (in model) of the element that is to be added
    int itemIndex;
    if (where == Start) {
        itemIndex = m_topButtonIndex - 1;
        if (itemIndex < 0) return false;
    } else {
        itemIndex = m_topButtonIndex + m_buttons.size();
        if (itemIndex >= m_model->size()) return false;
    }

    // Create and initialize the button for the item
    ExtenderButton * button = createButton();
    button->setTitle(m_model->title(itemIndex));
    button->setDescription(m_model->description(itemIndex));
    button->setIcon(m_model->icon(itemIndex));
    button->setLayout(NULL);
    button->enable(!m_model->isCategory(itemIndex));
    m_signalMapper.setMapping(button, itemIndex);

    int itemHeight = itemHeightFromIndex(itemIndex);

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
            itemHeightFromIndex(m_topButtonIndex + m_buttons.size() - 1)
        ;
        m_buttons.append(QPair < ExtenderButton *, int > (button, buttonTop));

    }

    int left = 0;
    qreal width = geometry().width();

    switch (m_extenderPosition) {
    case ExtenderButton::Left:
        left = EXTENDER_SIZE;
        // break; // We neet to have width -= EXTENDER_SIZE for Left too
    case ExtenderButton::Right:
        width -= EXTENDER_SIZE;
        break;
    default:
        break;
    }

    button->setGeometry(QRectF(left, buttonTop, width, itemHeight));

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

    if (items == 0) return 0;

    // items is from now on projected height for an item
    items = (listHeight - categoriesHeight) / items;

    if (items > m_maximumItemHeight)
        return m_currentItemHeight = m_maximumItemHeight;

    return m_currentItemHeight = items;
}

void ActionListView::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
    if (event->delta() > 0)
        m_scrollDirection = Up;
    else
        m_scrollDirection = Down;

    for (int i = 0; i < SCROLL_AMMOUNT; i++) {
        scrollBy(m_scrollDirection * SCROLL_AMMOUNT);
    }
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
