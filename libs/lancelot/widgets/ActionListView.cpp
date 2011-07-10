/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include "ActionListView_p.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>

#include <KDebug>

#include <Plasma/ItemBackground>

#define CATEGORY_MINIMUM_SIZE 20
#define CATEGORY_MAXIMUM_SIZE 35
#define CATEGORY_PREFERRED_SIZE 27

#define ITEM_MINIMUM_SIZE 40
#define ITEM_MAXIMUM_SIZE 70
#define ITEM_PREFERRED_SIZE 55

namespace Lancelot {

//> ActionListViewItem

ActionListViewItem::ActionListViewItem(ActionListViewItemFactory * factory)
    : ExtenderButton(), m_inSetSelected(false), m_factory(factory)
{
    connect(this, SIGNAL(activated()),
            this, SLOT(select()));
    m_categoryTriangle.setImagePath("lancelot/category-triangle");
}

ActionListViewItem::~ActionListViewItem()
{
}

Plasma::Svg ActionListViewItem::m_categoryTriangle;

void ActionListViewItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
        QWidget * widget) {
    ExtenderButton::paint(painter, option, widget);

    if (m_factory->m_categoriesActivable &&
        m_factory->m_model->isCategory(m_factory->m_items.indexOf(this))) {
        QSizeF diff = size() - m_categoryTriangle.size();
        m_categoryTriangle.paint(painter,
            diff.width() - 4,
            diff.height() / 2
            );
    }
}

void ActionListViewItem::select()
{
    setSelected(true);
}

void ActionListViewItem::deselect()
{
    setSelected(false);
    m_factory->m_selectedItem = NULL;
}

void ActionListViewItem::setSelected(bool selected)
{
    if (m_inSetSelected) return;
    m_inSetSelected = true;

    // setHovered(selected);

    m_factory->setSelectedItem(this, selected);

    if (!selected) {
        hoverLeaveEvent(NULL);
    } else {
        m_factory->m_view->scrollTo(geometry());
        hoverEnterEvent(NULL);
    }

    update();
    m_inSetSelected = false;
}

bool ActionListViewItem::isSelected() const
{
    return m_factory->m_selectedItem == this;
}

void ActionListViewItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
    ExtenderButton::contextMenuEvent(event);
    m_factory->itemContext(this);
    event->accept();
}

void ActionListViewItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_mousePos = event->pos();
    ExtenderButton::mousePressEvent(event);
}

void ActionListViewItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    ExtenderButton::mouseMoveEvent(event);
    if (Global::self()->immutability() == Plasma::Mutable) {
        if (isDown() && ((m_mousePos - event->pos()).toPoint().manhattanLength() > QApplication::startDragDistance())) {
            setDown(false);
            m_factory->itemDrag(this, event);
        }
    } else {
        event->ignore();
    }
}

//<

//> ActionListViewItemFactory
ActionListViewItemFactory::ActionListViewItemFactory(ActionListModel * model, ActionListView * view) //>
    : m_model(NULL),
      m_extenderPosition(NoExtender),
      m_itemsGroup(NULL), m_categoriesGroup(NULL),
      m_view(view),
      m_categoriesActivable(false), m_itemIconSize(32, 32),
      m_categoryIconSize(22, 22), m_selectedItem(NULL),
      m_displayMode(ActionListView::Standard)
{
    setItemsGroup(NULL);
    setCategoriesGroup(NULL);
    setModel(model);

    m_categoryHeight[Qt::MinimumSize]   = CATEGORY_MINIMUM_SIZE;
    m_categoryHeight[Qt::MaximumSize]   = CATEGORY_MAXIMUM_SIZE;
    m_categoryHeight[Qt::PreferredSize] = CATEGORY_PREFERRED_SIZE;

    m_itemHeight[Qt::MinimumSize]   = ITEM_MINIMUM_SIZE;
    m_itemHeight[Qt::MaximumSize]   = ITEM_MAXIMUM_SIZE;
    m_itemHeight[Qt::PreferredSize] = ITEM_PREFERRED_SIZE;

    m_selectedItemBackground = new CustomItemBackground(view->list());
} //<

ActionListViewItemFactory::~ActionListViewItemFactory() //>
{
    qDeleteAll(m_items);
    m_items.clear();
} //<

qreal ActionListViewItemFactory::preferredWidth() const //>
{
    qreal result = 0;
    foreach (ActionListViewItem * item, m_items) {
        result = qMax(result, item->preferredWidth());
    }
    return result;
}

void ActionListViewItemFactory::reload() //>
{
    while (m_items.size() > m_model->size()) {
        ActionListViewItem * item = m_items.takeLast();
        if (m_selectedItem == item) {
            m_selectedItem = NULL;
        }
        item->hide();
        item->deleteLater();
    }

    for (int i = 0; i < m_model->size(); i++) {
        itemForIndex(i, true);
    }

    emit updated();
} //<

CustomListItem * ActionListViewItemFactory::itemForIndex(int index) //>
{
    return itemForIndex(index, false);
} //<

CustomListItem * ActionListViewItemFactory::itemForIndex(int index,
        bool reload) //>
{
    ActionListViewItem * item;
    if (index < m_items.size() && m_items[index]) {
        item = m_items[index];
    } else {
        item = new ActionListViewItem(this);
        item->setGroup((m_model->isCategory(index))
                ? m_categoriesGroup : m_itemsGroup);
        reload = true;
        while (index >= m_items.size()) {
            m_items.append(NULL);
        }
        m_items[index] = item;
        setItemExtender(index);
        connect(item, SIGNAL(activated()),
                this, SLOT(itemActivated()));
        connect(item, SIGNAL(mouseHoverEnter()),
                this, SLOT(itemHovered()));
    }

    if (reload) {
        switch (m_displayMode) {
            case ActionListView::Standard:
                item->setTitle(m_model->title(index));
                item->setDescription(m_model->description(index));
                break;

            case ActionListView::DescriptionFirst:
                if (!m_model->description(index).isEmpty()) {
                    item->setTitle(m_model->description(index));
                    item->setDescription(m_model->title(index));
                } else {
                    item->setTitle(m_model->title(index));
                    item->setDescription(QString::null);
                }
                break;

            case ActionListView::SingleLineNameFirst:
                if (!m_model->description(index).isEmpty()) {
                    item->setTitle(
                        QString("%1 (%2)")
                            .arg(m_model->title(index))
                            .arg(m_model->description(index))
                        );
                } else {
                    item->setTitle(m_model->title(index));
                }

                item->setDescription(QString::null);
                break;

            case ActionListView::SingleLineDescriptionFirst:
                if (!m_model->description(index).isEmpty()) {
                    item->setTitle(
                        QString("%1 (%2)")
                            .arg(m_model->description(index))
                            .arg(m_model->title(index))
                        );
                } else {
                    item->setTitle(m_model->title(index));
                }

                item->setDescription(QString::null);
                break;

            case ActionListView::OnlyName:
                item->setTitle(m_model->title(index));
                item->setDescription(QString::null);
                break;

            case ActionListView::OnlyDescription:
                if (!m_model->description(index).isEmpty()) {
                    item->setTitle(m_model->description(index));
                } else {
                    item->setTitle(m_model->title(index));
                }
                item->setDescription(QString::null);
                break;


        }


        item->setIcon(m_model->icon(index));
        item->setMinimumHeight(itemHeight(index, Qt::MinimumSize));
        item->setPreferredHeight(itemHeight(index, Qt::PreferredSize));
        item->setMaximumHeight(itemHeight(index, Qt::MaximumSize));
        item->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        if (m_model->isCategory(index)) {
            item->setGroup(m_categoriesGroup);
            item->setIconSize(m_categoryIconSize);
        } else {
            item->setGroup(m_itemsGroup);
            item->setIconSize(m_itemIconSize);
        }
    }

    return item;
} //<

void ActionListViewItemFactory::setDisplayMode(ActionListView::ItemDisplayMode mode) //>
{
    if (m_displayMode == mode) {
        return;
    }

    m_displayMode = mode;



} //<

ActionListView::ItemDisplayMode ActionListViewItemFactory::displayMode() const //>
{
    return m_displayMode;
} //<

void ActionListViewItemFactory::setItemsGroup(Group * group) //>
{
    if (group == NULL) {
        group = Global::self()->group("ActionListView-Items");
    }

    if (group == m_itemsGroup) return;

    m_itemsGroup = group;

    int i = 0;
    foreach(ActionListViewItem * item, m_items) {
        if (!(m_model->isCategory(i))) {
            item->setGroup(group);
        }
        i++;
    }
} //<

Group * ActionListViewItemFactory::itemsGroup() const //>
{
    return m_itemsGroup;
} //<

void ActionListViewItemFactory::setCategoriesGroup(Group * group) //>
{
    if (group == NULL) {
        group = Global::self()->group("ActionListView-Categories");
    }

    if (group == m_itemsGroup) return;

    m_categoriesGroup = group;

    int i = 0;
    foreach(ActionListViewItem * item, m_items) {
        if (m_model->isCategory(i)) {
            item->setGroup(group);
            setItemExtender(i);
        }
        i++;
    }
}

Group * ActionListViewItemFactory::categoriesGroup() const //>
{
    return m_categoriesGroup;
} //<

void ActionListViewItemFactory::itemActivated() //>
{
    if (!sender()) {
        return;
    }

    activate(m_items.indexOf(
                (Lancelot::ActionListViewItem *)sender()));
} //<

void ActionListViewItemFactory::activate(int index) //>
{
    if (index < 0 || index >= m_model->size()) {
        return;
    }
    m_model->activated(index);

    emit activated(index);
} //<

int ActionListViewItemFactory::itemCount() const //>
{
    if (m_model) {
        return m_model->size();
    } else {
        return 0;
    }
} //<

void ActionListViewItemFactory::setItemHeight(int height, Qt::SizeHint which)
{
    m_itemHeight[which] = height;
    foreach (ActionListViewItem * item, m_items) {
        int index = m_items.indexOf(item);
        item->setMinimumHeight(itemHeight(index, Qt::MinimumSize));
        item->setPreferredHeight(itemHeight(index, Qt::PreferredSize));
        item->setMaximumHeight(itemHeight(index, Qt::MaximumSize));
    }
    emit updated();
}

int ActionListViewItemFactory::itemHeight(Qt::SizeHint which) const
{
    return m_itemHeight.value(which);
}

void ActionListViewItemFactory::setCategoryHeight(int height, Qt::SizeHint which)
{
    m_categoryHeight[which] = height;
    foreach (ActionListViewItem * item, m_items) {
        int index = m_items.indexOf(item);
        item->setMinimumHeight(itemHeight(index, Qt::MinimumSize));
        item->setPreferredHeight(itemHeight(index, Qt::PreferredSize));
        item->setMaximumHeight(itemHeight(index, Qt::MaximumSize));
    }
    emit updated();
}

int ActionListViewItemFactory::categoryHeight(Qt::SizeHint which) const
{
    return m_categoryHeight.value(which);
}

void ActionListViewItemFactory::setItemIconSize(QSize size)
{
    m_itemIconSize = size;

    int i = 0;
    foreach (ActionListViewItem * item, m_items) {
        if (!m_model->isCategory(i)) {
            item->setIconSize(size);
        }
        i++;
    }
    emit updated();
}

QSize ActionListViewItemFactory::itemIconSize() const
{
    return m_itemIconSize;
}

void ActionListViewItemFactory::setCategoryIconSize(QSize size)
{
    m_categoryIconSize = size;

    int i = 0;
    foreach (ActionListViewItem * item, m_items) {
        if (m_model->isCategory(i)) {
            item->setIconSize(size);
        }
        i++;
    }
    emit updated();
}

QSize ActionListViewItemFactory::categoryIconSize() const
{
    return m_categoryIconSize;
}

int ActionListViewItemFactory::itemHeight(int index, Qt::SizeHint which) const //>
{
    if (m_model->isCategory(index)) {
        return m_categoryHeight[which];
    } else {
        return m_itemHeight[which];
    }
} //<

void ActionListViewItemFactory::setModel(ActionListModel * model) //>
{
    ActionListModel * oldmodel = m_model;

    if (m_model) {
        disconnect(m_model, NULL, this, NULL);
    }

    if (!model) {
        return;
    }

    m_model = model;

    connect(model, SIGNAL(itemInserted(int)),
            this, SLOT(modelItemInserted(int)));
    connect(model, SIGNAL(itemDeleted(int)),
            this, SLOT(modelItemDeleted(int)));
    connect(model, SIGNAL(itemAltered(int)),
            this, SLOT(modelItemAltered(int)));
    connect(model, SIGNAL(updated()),
            this, SLOT(modelUpdated()));

    if (oldmodel) {
        modelUpdated();
    }
} //<

ActionListModel * ActionListViewItemFactory::model() const //>
{
    return m_model;
} //<

void ActionListViewItemFactory::modelUpdated() //>
{
    reload();
} //<

void ActionListViewItemFactory::modelItemInserted(int index) //>
{
    if (index < 0 || index > m_items.size()) {
        // If we get an illegal notification, do
        // a full reload
        reload();
    } else {
        m_items.insert(index, NULL);
        itemForIndex(index, true);
        emit itemInserted(index);
    }
} //<

void ActionListViewItemFactory::modelItemDeleted(int index) //>
{
    if (index < 0 || index >= m_items.size()) {
        // If we get an illegal notification, do
        // a full reload
        reload();
    } else {
        ActionListViewItem * item = m_items.takeAt(index);
        if (m_selectedItem == item) {
            m_selectedItem = NULL;
        }
        delete item;
        emit itemDeleted(index);
    }
} //<

void ActionListViewItemFactory::modelItemAltered(int index) //>
{
    if (index < 0 || index >= m_items.size()) {
        // If we get an illegal notification, do
        // a full reload
        reload();
    } else {
        itemForIndex(index, true);
        emit itemAltered(index);
    }
} //<

void ActionListViewItemFactory::setExtenderPosition(int position) //>
{
    if (position == TopExtender) {
        position = LeftExtender;
    }

    if (position == BottomExtender) {
        position = RightExtender;
    }

    m_extenderPosition = position;
    updateExtenderPosition();
} //<

void ActionListViewItemFactory::updateExtenderPosition() //>
{
    for (int i = 0; i < m_items.count(); i++) {
        setItemExtender(i);
    }
} //<

void ActionListViewItemFactory::setItemExtender(int index) //>
{
    ActionListViewItem * item = m_items.at(index);
    if (m_model->isCategory(index) && !m_categoriesActivable) {
        item->setExtenderPosition(NoExtender);
    } else {
        item->setExtenderPosition(m_extenderPosition);
    }
} //<

int ActionListViewItemFactory::extenderPosition() const //>
{
    return m_extenderPosition;
} //<

void ActionListViewItemFactory::itemContext(
        ActionListViewItem * sender, bool mouseCoordinate) //>
{
    int index = m_items.indexOf(sender);
    if (index < 0 || index >= m_model->size() ||
            !m_model->hasContextActions(index)) {
        return;
    }

    Lancelot::PopupMenu menu;
    QPoint popupPoint;
    m_model->setContextActions(index, &menu);
    if (!mouseCoordinate) {
        QGraphicsScene * scene = sender->scene();
        if (scene->views().size()) {
            QGraphicsView * view = scene->views().at(0);
            QPointF pos = sender->mapToScene(
                    QPointF(sender->geometry().width() * 0.7, 0));
            popupPoint = view->mapToGlobal(
                view->mapFromScene(pos)
            );
        } else {
            mouseCoordinate = true;
        }
    }

    if (mouseCoordinate) {
        popupPoint = QCursor::pos();
    }

    m_model->contextActivate(index, menu.exec(popupPoint));
} //<

void ActionListViewItemFactory::itemDrag(ActionListViewItem * sender, QGraphicsSceneMouseEvent * event) //>
{
    int index = m_items.indexOf(sender);
    if (index < 0 || index >= m_model->size()) {
        return;
    }

    QMimeData * data = m_model->mimeData(index);
    if (data == NULL) {
        return;
    }

    QDrag * drag = new QDrag(event->widget());
    drag->setMimeData(data);

    // Pixmap for dragger
    QPixmap pixmap(sender->size().toSize());
    QPainter painter(&pixmap);
    painter.fillRect(QRect(QPoint(), pixmap.size()), QColor(100, 100, 100));
    sender->paint(&painter, 0, 0);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos().toPoint());

    Qt::DropActions actions;
    Qt::DropAction defaultAction;
    m_model->setDropActions(index, actions, defaultAction);

    Qt::DropAction dropAction = drag->exec(actions, defaultAction);
    m_model->dataDragFinished(index, dropAction);

} //<

void ActionListViewItemFactory::activateSelectedItem() //>
{
    if (!m_selectedItem) {
        return;
    }

    activate(m_items.indexOf(m_selectedItem));
} //<

void ActionListViewItemFactory::contextForSelectedItem() //>
{
    if (!m_selectedItem) {
        return;
    }

    itemContext(m_selectedItem, false);
} //<

void ActionListViewItemFactory::clearSelection() //>
{
    if (m_selectedItem) {
        setSelectedItem(m_selectedItem, false);
    }
}

void ActionListViewItemFactory::setSelectedItem(ActionListViewItem * item, bool selected) //>
{
    if (m_selectedItem == item && !selected) {
        if (m_selectedItem) {
            m_selectedItem->setSelected(false);
        }
        m_selectedItem = NULL;
    } else if (m_selectedItem != item && selected) {
        if (m_selectedItem) {
            m_selectedItem->setSelected(false);
        }
        m_selectedItem = item;
        m_selectedItem->setSelected(true);
    }
        updateSelectedBackground(m_selectedItem);
} //<

void ActionListViewItemFactory::itemHovered() //>
{
    if (!sender()) {
        return;
    }

    Lancelot::ActionListViewItem * item =
        static_cast < Lancelot::ActionListViewItem * > (sender());
    updateSelectedBackground(item);

} //<

void ActionListViewItemFactory::updateSelectedBackground(ActionListViewItem * item) //>
{
    if (!item || !item->isEnabled()) {
        item = m_selectedItem;
    }

    if (item) {

        QTransform transform = item->transform();
        if (transform.isIdentity()) {
            m_selectedItemBackground->setTarget(item->geometry());
        } else {
            QRectF g = item->geometry();
            if (transform.m32() != 0) {
                g.setHeight(g.height() * transform.m22());
                g.moveTop(g.top() + transform.m32());
            } else {
                g.setHeight(g.height() * transform.m22());
            }
            m_selectedItemBackground->setTarget(g);
        }

        m_selectedItemBackground->show();
    } else {
        m_selectedItemBackground->hide();
    }
} //<

void ActionListViewItemFactory::selectRelItem(int rel) //>
{
    int index = -1;
    if (m_selectedItem) {
        index = m_items.indexOf(m_selectedItem);
    }

    if (index == -1) {
        if (rel == 1) {
            index = 0;
        } else {
            index = m_items.count() - 1;
        }
    } else {
        if (rel == 1) {
            index++;
            if (index >= m_items.count()) {
                index = 0;
            }
        } else {
            index--;
            if (index < 0) {
                index = m_items.count() - 1;
            }
        }
    }

    if (!m_categoriesActivable) {
        int oindex = index;
        while (m_model->isCategory(index)) {
            index += rel;
            if (index == oindex) return;

            if (index < 0) {
                index = m_items.count() - 1;
            } else if (index >= m_items.count()) {
                index = 0;
            }
        }
    }

    if (index >= 0 && index < m_items.count()) {
        m_items.at(index)->setSelected();
    }
} //<

//<

//> ActionListView
ActionListView::Private::Private(ActionListView * listView) //>
    : itemFactory(NULL), q(listView)
{
    listView->setFlag(ScrollPane::HoverShowScrollbars);
    listView->clearFlag(ScrollPane::ClipScrollable);
    listView->setFocusPolicy(Qt::WheelFocus);

    Plasma::Svg * svg = new Plasma::Svg();
    svg->setImagePath("lancelot/action-list-view-drop-indicator");
    svg->setContainsMultipleImages(true);
    dropIndicator = new Plasma::SvgWidget(listView->list());
    dropIndicator->setSvg(svg);
    dropIndicator->setElementID("drop-indicator");
    dropIndicator->hide();

    connect(
        Global::self(), SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
        this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));
} //<

ActionListView::Private::~Private() //>
{
    delete itemFactory;
} //<

void ActionListView::Private::immutabilityChanged(const Plasma::ImmutabilityType immutable) //>
{
    q->setAcceptDrops(Plasma::Mutable == immutable);
} //<

void ActionListView::Private::sizeHintUpdateNeeded() //>
{
    q->updateGeometry();
} //<

ActionListView::ActionListView(QGraphicsItem * parent) //>
    : CustomListView(parent), d(new Private(this))
{
    setModel(NULL);
} //<

ActionListView::ActionListView(ActionListModel * model, QGraphicsItem * parent) //>
    : CustomListView(parent),
      d(new Private(this))
{
    setModel(model);
    // d->itemFactory = (ActionListViewItemFactory *) list()->itemFactory();
    connect(
            d->itemFactory, SIGNAL(activated(int)),
            this, SIGNAL(activated(int)));
} //<

bool ActionListView::sceneEventFilter(QGraphicsItem * object,
        QEvent * event)
{
    return CustomListView::sceneEventFilter(object, event);
}

bool ActionListView::sceneEvent(QEvent * event)
{

    QGraphicsSceneDragDropEvent * dndEvent
            = dynamic_cast < QGraphicsSceneDragDropEvent * > (event);
    QRectF g;
    switch (event->type()) {
        case QEvent::GraphicsSceneDragEnter:
            dndEvent->acceptProposedAction();
            d->dropIndicator->show();
            break;
        case QEvent::GraphicsSceneDragLeave:
            d->dropIndicator->hide();
            break;
        case QEvent::GraphicsSceneDragMove:
        {
            int top = dndEvent->pos().y() - list()->geometry().top();
            int index = list()->itemAtPosition(top);
            QGraphicsWidget * item = dynamic_cast < QGraphicsWidget * >
                    (list()->itemFactory()->itemForIndex(index));
            if (item) {
                g = item->geometry();
                if (top - g.top() < g.bottom() - top) {
                    g.setTop(item->geometry().top() - 2);
                } else {
                    g.setTop(item->geometry().bottom() - 2);
                }
                g.setHeight(4);
                d->dropIndicator->setGeometry(g);

                bool allow = d->itemFactory->model()->dataDropAvailable(index, dndEvent->mimeData());
                d->dropIndicator->setVisible(allow);
                dndEvent->setAccepted(allow);
            }
            break;
        }
        case QEvent::GraphicsSceneDrop:
        {
            int top = dndEvent->pos().y() - list()->geometry().top();
            int index = list()->itemAtPosition(top);
            // QGraphicsWidget * item = dynamic_cast < QGraphicsWidget * >
            //         (list()->itemFactory()->itemForIndex(index));
            // if (item) {
            //     g = item->geometry();
            //     if (top - g.top() >= g.bottom() - top) {
            //         index++;
            //     }
            // }
            d->itemFactory->model()->dataDropped(index, dndEvent->mimeData());
            d->dropIndicator->hide();
            break;
        }
        default:
            // nothing
            break;
    }

    return CustomListView::sceneEvent(event);
}

ActionListView::~ActionListView() //>
{
    delete d;
} //<

void ActionListView::setModel(ActionListModel * model) //>
{
    if (!d->itemFactory) {
        d->itemFactory = new ActionListViewItemFactory(
                model, this);
        list()->setItemFactory(d->itemFactory);
        setAcceptDrops(Plasma::Mutable == Global::self()->immutability());
        connect(d->itemFactory, SIGNAL(updated()),
             d, SLOT(sizeHintUpdateNeeded()));
        connect(d->itemFactory, SIGNAL(itemInserted(int)),
             d, SLOT(sizeHintUpdateNeeded()));
        connect(d->itemFactory, SIGNAL(itemDeleted(int)),
             d, SLOT(sizeHintUpdateNeeded()));
        connect(d->itemFactory, SIGNAL(itemAltered(int)),
             d, SLOT(sizeHintUpdateNeeded()));
    } else {
        d->itemFactory->setModel(model);
    }
} //<

ActionListModel * ActionListView::model() const //>
{
    if (!d->itemFactory) {
        return NULL;
    }
    return d->itemFactory->model();
} //<

void ActionListView::setItemHeight(int height, Qt::SizeHint which)
{
    if (!d->itemFactory) {
        return;
    }
    return d->itemFactory->setItemHeight(height, which);
}

int ActionListView::itemHeight(Qt::SizeHint which) const
{
    if (!d->itemFactory) {
        return 0;
    }
    return d->itemFactory->itemHeight(which);
}

void ActionListView::setCategoryHeight(int height, Qt::SizeHint which)
{
    if (!d->itemFactory) {
        return;
    }
    return d->itemFactory->setCategoryHeight(height, which);
}

int ActionListView::categoryHeight(Qt::SizeHint which) const
{
    if (!d->itemFactory) {
        return 0;
    }
    return d->itemFactory->categoryHeight(which);
}

void ActionListView::setItemIconSize(QSize size)
{
    if (!d->itemFactory) {
        return;
    }
    return d->itemFactory->setItemIconSize(size);
}

QSize ActionListView::itemIconSize() const
{
    if (!d->itemFactory) {
        return QSize();
    }
    return d->itemFactory->itemIconSize();
}

void ActionListView::setCategoryIconSize(QSize size)
{
    if (!d->itemFactory) {
        return;
    }
    return d->itemFactory->setCategoryIconSize(size);
}

QSize ActionListView::categoryIconSize() const
{
    if (!d->itemFactory) {
        return QSize();
    }
    return d->itemFactory->categoryIconSize();
}

void ActionListView::setShowsExtendersOutside(bool value)
{
    d->showsExtendersOutside = value;
    if (value) {
        clearFlag(ScrollPane::ClipScrollable);
    } else {
        setFlag(ScrollPane::ClipScrollable);
    }

    if (!d->itemFactory) {
        return;
    }
    // relayouting:
    setExtenderPosition(d->itemFactory->extenderPosition());
}

bool ActionListView::showsExtendersOutside() const
{
    return d->showsExtendersOutside;
}


void ActionListView::setExtenderPosition(int position) //>
{
    if (!d->itemFactory) {
        return;
    }
    d->itemFactory->setExtenderPosition(position);

    if (d->itemFactory->extenderPosition() == LeftExtender) {
        list()->setMargin(Plasma::LeftMargin,
                d->showsExtendersOutside ? 0 : EXTENDER_SIZE);
        list()->setMargin(Plasma::RightMargin, 0);
        setFlip(Plasma::NoFlip);
    } else if (d->itemFactory->extenderPosition() == RightExtender) {
        list()->setMargin(Plasma::LeftMargin, 0);
        list()->setMargin(Plasma::RightMargin,
                d->showsExtendersOutside ? 0 : EXTENDER_SIZE);
        setFlip(Plasma::HorizontalFlip);
    } else {
        list()->setMargin(Plasma::LeftMargin, 0);
        list()->setMargin(Plasma::RightMargin, 0);
        setFlip(Plasma::NoFlip);
    }

} //<

int ActionListView::extenderPosition() const //>
{
    if (!d->itemFactory) {
        return NoExtender;
    }
    return d->itemFactory->extenderPosition();
} //<

void ActionListView::setDisplayMode(ActionListView::ItemDisplayMode mode) //>
{
    if (!d->itemFactory) {
        return;
    }

    d->itemFactory->setDisplayMode(mode);
} //<

ActionListView::ItemDisplayMode ActionListView::displayMode() const //>
{
    if (!d->itemFactory) {
        return Standard;
    }

    return d->itemFactory->displayMode();
} //<

void ActionListView::setItemsGroup(Group * group) //>
{
    if (!d->itemFactory) {
        return;
    }

    d->itemFactory->setItemsGroup(group);
} //<

void ActionListView::setItemsGroupByName(const QString & group) //>
{
    setItemsGroup(Global::self()->group(group));
} //<

Group * ActionListView::itemsGroup() const //>
{
    if (!d->itemFactory) {
        return NULL;
    }

    return d->itemFactory->itemsGroup();
} //<

void ActionListView::setCategoriesGroup(Group * group) //>
{
    if (!d->itemFactory) {
        return;
    }

    d->itemFactory->setCategoriesGroup(group);
} //<

void ActionListView::setCategoriesGroupByName(const QString & group) //>
{
    setCategoriesGroup(Global::self()->group(group));
} //<

Group * ActionListView::categoriesGroup() const //>
{
    if (!d->itemFactory) {
        return NULL;
    }

    return d->itemFactory->categoriesGroup();
} //<

void ActionListView::keyPressEvent(QKeyEvent * event) //>
{
    if (!d->itemFactory) {
        return;
    }

    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        if (event->modifiers() & Qt::AltModifier) {
            d->itemFactory->contextForSelectedItem();
        } else {
            d->itemFactory->activateSelectedItem();
        }
    } else if (event->key() == Qt::Key_Menu) {
        d->itemFactory->contextForSelectedItem();
    } else if (event->key() == Qt::Key_Down) {
        d->itemFactory->selectRelItem(+1);
    } else if (event->key() == Qt::Key_Up) {
        d->itemFactory->selectRelItem(-1);
    }
} //<

void ActionListView::clearSelection() //>
{
    d->itemFactory->clearSelection();
} //<

void ActionListView::initialSelection() //>
{
    d->itemFactory->clearSelection();
    d->itemFactory->selectRelItem(+1);
} //<

int ActionListView::selectedIndex() const //>
{
    if (!d->itemFactory->m_selectedItem) {
        return -1;
    }

    return d->itemFactory->m_items.indexOf(d->itemFactory->m_selectedItem);
} //>

bool ActionListView::areCategoriesActivable() const //>
{
    return d->itemFactory->m_categoriesActivable;
} //<

void ActionListView::setCategoriesActivable(bool value) //>
{
    d->itemFactory->m_categoriesActivable = value;
    d->itemFactory->updateExtenderPosition();
} //<

QSizeF ActionListView::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const //>
{
    QSizeF result;
    if (!group()) return result;

    switch (which) {
        case Qt::MinimumSize:
            result = QSizeF();
            break;
        case Qt::MaximumSize:
            result = MAX_WIDGET_SIZE;
            break;
        case Qt::PreferredSize:
            result = QSizeF(d->itemFactory->preferredWidth(), 400);
            result += CustomListView::sizeHint(which, constraint);
            // scrollbars...
            result.rwidth() += 50;
            break;
        default:
            result = QSizeF();
            break;
    }
    if (constraint.isValid()) {
        result = result.boundedTo(constraint);
    }
    return result;
} //<

void ActionListView::resizeEvent(QGraphicsSceneResizeEvent * event) //>
{
    CustomListView::resizeEvent(event);

    if (d->itemFactory) {
        d->itemFactory->updateSelectedBackground();
    }
} //<

void ActionListView::hoverLeaveEvent(QGraphicsSceneHoverEvent * event) //>
{
    CustomListView::hoverLeaveEvent(event);

    if (d->itemFactory) {
        d->itemFactory->updateSelectedBackground();
    }
} //<

void ActionListView::show()
{
    setVisible(true);
}

void ActionListView::hide()
{
    setVisible(false);
}

//<

} // namespace Lancelot
