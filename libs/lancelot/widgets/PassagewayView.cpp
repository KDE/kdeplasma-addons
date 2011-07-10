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

#include "PassagewayView.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QWidget>
#include <QPointer>

#include <climits>

namespace Lancelot {

class PassagewayViewSizer: public ColumnLayout::ColumnSizer {
public:
    PassagewayViewSizer()
    {
        m_sizer = ColumnLayout::ColumnSizer::create(ColumnLayout::ColumnSizer::GoldenSizer);
    }

    void init(int size)
    {
        m_size = size;
        m_pass = true;
        if (size > 2) {
            m_sizer->init(size - 1);
        }
    }

    qreal size()
    {
        if (!m_size)     return 1.0;
        if (m_size <= 2) return 1.0 / m_size;
        if (m_pass) {
            m_pass = false;
            return 0.0;
        } else {
            return m_sizer->size();
        }
    }

private:
    ColumnLayout::ColumnSizer * m_sizer;
    int m_size;
    bool m_pass;
};

class PassagewayView::Private {
public:
    Private(ActionTreeModel * entranceModel,
            ActionTreeModel * atlasModel,
            PassagewayView * p)
      : focusIndex(0), layout(NULL), buttonsLayout(NULL), listsLayout(NULL),
        parent(p), popup(NULL), popupMenus(false)
    {
        parent->setLayout(layout = new NodeLayout());
        layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
        layout->setMaximumSize(INT_MAX, INT_MAX);

        layout->addItem(
            buttonsLayout = new ColumnLayout(),
            NodeLayout::NodeCoordinate(0, 0, 0, 0),
            NodeLayout::NodeCoordinate(1, 0, 0, 32)
        );
        buttonsLayout->setSizer(
            ColumnLayout::ColumnSizer::create(
                ColumnLayout::ColumnSizer::EqualSizer
            )
        );
        buttonsLayout->setColumnCount(30);

        buttonsLayout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
        buttonsLayout->setMaximumSize(INT_MAX, INT_MAX);
        buttonsLayout->setContentsMargins(EXTENDER_SIZE, 0, 0, 0);

        // ExtenderButton * spacer =
        //     new ExtenderButton(parent);
        // spacer->setGroupByName("PassagewayView-Spacer");
        // spacer->setMaximumSize(QSizeF(EXTENDER_SIZE, 32));
        // spacer->setPreferredSize(QSizeF(EXTENDER_SIZE, 32));
        // spacer->setMinimumSize(QSizeF(EXTENDER_SIZE, 32));
        // buttonsLayout->push(spacer);

        layout->addItem(
            listsLayout = new ColumnLayout(),
            NodeLayout::NodeCoordinate(0, 0, 0, 32),
            NodeLayout::NodeCoordinate(1, 1, 0, 0)
        );

        listsLayout->setSizer(new PassagewayViewSizer());
        listsLayout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
        listsLayout->setMaximumSize(INT_MAX, INT_MAX);

        //buttonsLayout->setSpacing(0.0);

        next(Step("", QIcon(), entranceModel));
        next(Step("", QIcon(), atlasModel));
        focusIndex = 0;

        lists.at(0)->setExtenderPosition(Lancelot::LeftExtender);
        lists.at(0)->setCategoriesActivable(false);
        lists.at(1)->setCategoriesActivable(true);
    }

    ~Private()
    {
        delete buttonsLayout;
        delete listsLayout;
        delete layout;
        delete popup;

        qDeleteAll(buttons);
        qDeleteAll(lists);
        qDeleteAll(path);
    }

    class Step {
    public:
        Step(QString t, QIcon i, ActionTreeModel * m)
            : title(t), icon(i), model(m) {};
        QString title;
        QIcon icon;
        ActionTreeModel * model;
    };

    class BreadcrumbItem : public Lancelot::ExtenderButton {
    public:
        BreadcrumbItem(QIcon icon, QString title, QString description,
                QGraphicsItem * parent, PassagewayView::Private * parent_private)
            : Lancelot::ExtenderButton(icon, title, description, parent),
              d(parent_private)
        {
        }

        void mousePressEvent(QGraphicsSceneMouseEvent * event)
        {
            m_mousePos = event->pos();
            ExtenderButton::mousePressEvent(event);
        }

        void mouseMoveEvent(QGraphicsSceneMouseEvent * event)
        {
            ExtenderButton::mouseMoveEvent(event);
            if (isDown() && ((m_mousePos - event->pos()).toPoint().manhattanLength() > QApplication::startDragDistance())) {
                setDown(false);
                d->startDrag(this, event);
            }
        }

    private:
        PassagewayView::Private * d;
        QPointF m_mousePos;
    };

    void back(int steps, bool deselectLast = true)
    {
        if (steps == 0) {
            return;
        }

        for (int i = 0; i < steps; ++i) {
            if (lists.size() == 0) {
                return;
            }

            if (buttons.size() > 2) {
                buttons.at(buttons.size() - 3)->setGroupByName(parent->group()->name() + "-InactiveButton");
                buttons.at(buttons.size() - 3)->setExtenderPosition(Lancelot::NoExtender);
            }
            ExtenderButton * button = buttons.takeLast();

            lists.last()->setShowsExtendersOutside(true);
            ActionListView * list   = lists.takeLast();
            lists.last()->setShowsExtendersOutside(false);

            path.takeLast();

            // buttonsLayout->removeItem(button);
            buttonsLayout->pop();

            // This really shouldn't be needed :(
            buttonsLayout->setGeometry(buttonsLayout->geometry());

            listsLayout->pop();

            button->deleteLater();
            list->deleteLater();
        }

        if (focusIndex >= lists.size()) {
            focusIndex = lists.size() - 1;
        }

        if (deselectLast) {
            lists.last()->clearSelection();
        }
    }

    void next(Step newStep)
    {
        Step * step = new Step(newStep);
        ExtenderButton * button =
            new BreadcrumbItem(step->icon, step->title, QString(), parent, this);
        ActionListView * list   =
            new ActionListView(step->model, parent);
        list->setGroupByName(parent->group()->name() + "-Atlas");

        button->setIconSize(QSize(22, 22));
        button->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        button->setGroupByName(parent->group()->name() + "-InactiveButton");
        button->setExtenderPosition(Lancelot::NoExtender);

        buttons.append(button);
        button->setZValue((qreal)buttons.count());

        if (buttons.size() > 2) {
            buttons.at(buttons.size() - 3)->setGroupByName(parent->group()->name() + "-Button");
        }

        list->setCategoriesGroupByName("ActionListView-CategoriesPass");
        list->setCategoriesActivable(true);

        focusIndex = lists.count();
        if (lists.size() > 1) {
            lists.last()->setShowsExtendersOutside(true);
        }
        lists.append(list);
        lists.last()->setShowsExtendersOutside(false);
        lists.last()->setZValue(- lists.count());

        path.append(step);

        buttonsLayout->push(button);
        listsLayout->push(list);

        // This really shouldn't be needed :(
        buttonsLayout->setGeometry(buttonsLayout->geometry());
        listsLayout->setGeometry(listsLayout->geometry());


        QObject::connect(
            list, SIGNAL(activated(int)),
            parent, SLOT(listItemActivated(int))
        );

        QObject::connect(
            button, SIGNAL(activated()),
            parent, SLOT(pathButtonActivated())
        );
    }

    void startDrag(BreadcrumbItem * item, QGraphicsSceneMouseEvent * event)
    {
        if (Global::self()->immutability() != Plasma::Mutable) {
            return;
        }

        int index = buttons.indexOf(item, 0);
        if (index == -1) {
            return;
        }

        QMimeData * data = path.at(index)->model->selfMimeData();
        if (!data) {
            return;
        }

        // QMimeData * data = new QMimeData();
        // data->setData("text/uri-list", m_dragUrl.toAscii());
        // data->setData("text/plain", m_dragUrl.toAscii());

        QWidget * widget = NULL;
        if (item->scene() && item->scene()->views().size() > 0) {
            widget = item->scene()->views().at(0);
        }

        QDrag * drag = new QDrag(widget);
        drag->setMimeData(data);

        // Pixmap for dragger
        QPixmap pixmap(item->size().toSize());
        QPainter painter(&pixmap);
        painter.fillRect(QRect(QPoint(), pixmap.size()), QColor(100, 100, 100));
        item->paint(&painter, 0, 0);
        drag->setPixmap(pixmap);
        drag->setHotSpot(event->pos().toPoint());

        drag->exec(); //krazy:exclude=crashy
    }


    QList < Step * > path;
    QList < ExtenderButton * > buttons;
    QList < ActionListView * > lists;
    int focusIndex;

    NodeLayout                  * layout;
    ColumnLayout::ColumnSizer   * sizer;
    // QGraphicsLinearLayout       * buttonsLayout;
    ColumnLayout                * buttonsLayout;
    ColumnLayout                * listsLayout;
    PassagewayView              * parent;
    QPointer < PopupMenu >        popup;
    bool                          popupMenus;
};

PassagewayView::PassagewayView(QGraphicsItem * parent)
    : Panel(parent), d(new Private(NULL, NULL, this))
{
    setGroupByName("PassagewayView");
}

PassagewayView::PassagewayView(ActionTreeModel * entranceModel,
    ActionTreeModel * atlasModel, QGraphicsItem * parent)
    : Panel(parent), d(new Private(entranceModel, atlasModel, this))
{
    setGroupByName("PassagewayView");
}

void PassagewayView::pathButtonActivated()
{
    for (int i = d->buttons.size() - 1; i >= 0; --i) {
        if (d->buttons.at(i) == sender()) {
            d->back(d->buttons.size() - i - 2);
        }
    }
}

void PassagewayView::listItemActivated(int index, int listIndex)
{
    if (listIndex == -1) {
        listIndex = d->lists.indexOf((ActionListView *)sender());
    }

    if (listIndex == -1) {
        return;
    }

    if (listIndex == 0) {
        // something in the entrance is clicked
        // we don't want to remove the first level
        // of atlas as well
        d->back(d->lists.size() - listIndex - 2, false);
    } else {
        d->back(d->lists.size() - listIndex - 1, false);
    }

    ActionTreeModel * model = d->path.at(listIndex)->model;
    if (model) {
        model = model->child(index);
        if (model) {
            if (d->popupMenus) {
                if (!d->popup) {
                    d->popup = new PopupMenu();
                }
                d->popup->setModel(model);

                QPoint p = QCursor::pos();

                if (scene() && scene()->views().size()) {
                    QGraphicsView * view = scene()->views().first();

                    p.setX(
                        (view->pos() + view->mapFromScene(scenePos())).x()
                        + geometry().width());

                    p.setY(p.y() - 16);
                }
                d->popup->exec(p);
            } else {
                d->next(Private::Step(model->selfTitle(), model->selfIcon(), model));
            }
        } else {
            d->lists.at(listIndex)->clearSelection();
        }
    }
}

PassagewayView::~PassagewayView()
{
    delete d;
}

// Entrance
void PassagewayView::setEntranceModel(ActionTreeModel * model)
{
    if (d->lists.size() < 2) return;
    d->path.at(0)->model = model;
    d->lists.at(0)->setModel(model);
}

void PassagewayView::setEntranceTitle(const QString & title)
{
    if (d->lists.size() < 2) return;
    d->path.at(0)->title = title;
    d->buttons.at(0)->setTitle(title);
}

void PassagewayView::setEntranceIcon(QIcon icon)
{
    if (d->lists.size() < 2) return;
    d->path.at(0)->icon = icon;
    d->buttons.at(0)->setIcon(icon);
}

// Atlas
void PassagewayView::setAtlasModel(ActionTreeModel * model)
{
    if (d->lists.size() < 2) return;
    d->path.at(1)->model = model;
    d->lists.at(1)->setModel(model);
}

void PassagewayView::setAtlasTitle(const QString & title)
{
    if (d->lists.size() < 2) return;
    d->path.at(1)->title = title;
    d->buttons.at(1)->setTitle(title);
}

void PassagewayView::setAtlasIcon(QIcon icon)
{
    if (d->lists.size() < 2) return;
    d->path.at(1)->icon = icon;
    d->buttons.at(1)->setIcon(icon);
}

void PassagewayView::setGroup(Group * g)
{
    Panel::setGroup(g);

    int i = d->buttons.size();
    foreach (ExtenderButton * button, d->buttons) {
        --i;
        if (i < 2) {
            button->setGroupByName(group()->name() + "-InactiveButton");
        } else {
            button->setGroupByName(group()->name() + "-Button");
        }
    }

    i = 0;
    foreach (ActionListView * list, d->lists) {
        if (i++ == 0) {
            list->setGroupByName(group()->name() + "-Entrance");
        } else {
            list->setGroupByName(group()->name() + "-Atlas");
        }
    }
}

void PassagewayView::setActivationMethod(int value)
{
    if (value == Lancelot::ClickActivate) {
        Global::self()->group(group()->name() + "-Entrance")
            ->setProperty("extenderPosition", NoExtender);
        Global::self()->group(group()->name() + "-Atlas")
            ->setProperty("extenderPosition", NoExtender);
        Global::self()->group(group()->name() + "-Button")
            ->setProperty("extenderPosition", NoExtender);
        Global::self()->group(group()->name() + "-Button")
            ->setProperty("activationMethod", ClickActivate);
    } else {
        Global::self()->group(group()->name() + "-Entrance")
            ->setProperty("extenderPosition", LeftExtender);
        Global::self()->group(group()->name() + "-Atlas")
            ->setProperty("extenderPosition", RightExtender);
        Global::self()->group(group()->name() + "-Button")
            ->setProperty("extenderPosition", LeftExtender);
        Global::self()->group(group()->name() + "-Button")
            ->setProperty("activationMethod", ExtenderActivate);
    }
    // Global::self()->group(group()->name() + "-Entrance")
    //     ->notifyUpdated();
    // Global::self()->group(group()->name() + "-Atlas")
    //     ->notifyUpdated();
    // Global::self()->group(group()->name() + "-Button")
    //     ->notifyUpdated();
}

int PassagewayView::activationMethod() const
{
   return Lancelot::ClickActivate;
}

void PassagewayView::setColumnLimit(int limit)
{
    if (limit > 0) {
        d->listsLayout->setColumnCount(limit);
        d->popupMenus = false;
    } else {
        d->listsLayout->setColumnCount(2);
        d->popupMenus = true;
    }
}

void PassagewayView::reset()
{
    d->focusIndex = 0;
    if (d->lists.size() > 2) {
        d->back(d->lists.size() - 2);
    }
}

void PassagewayView::clearSelection()
{

}

void PassagewayView::keyPressEvent(QKeyEvent * event)
{
    // We should open a submenu on right arrow pressed,
    // but not activate item if not a submenu
    if (event->key() == Qt::Key_Right) {
        ActionTreeModel * model = d->path.at(d->focusIndex)->model;
        int index = d->lists.at(d->focusIndex)->selectedIndex();
        if (index >= 0 && model && (model = model->child(index))) {
            listItemActivated(index, d->focusIndex);
            return;
        }
    }

    // And we should shift left if user moved left
    if (event->key() == Qt::Key_Left) {
        if (d->focusIndex < d->lists.count() - 1 && d->focusIndex != 0) {
            d->back(1);
        }
    }

    // Normal handling
    bool pass = false;
    int oindex = d->focusIndex;

    switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            d->lists.at(d->focusIndex)->keyPressEvent(event);
            break;
        case Qt::Key_Left:
            d->focusIndex--;
            break;
        case Qt::Key_Right:
            d->focusIndex++;
            break;
        default:
            pass = true;
    }


    if (d->focusIndex < 0) {
        d->focusIndex = 0;
        pass = true;
    } else if (d->focusIndex >= d->lists.size()) {
        d->focusIndex = d->lists.size() - 1;
        pass = true;
    }

    if (oindex != d->focusIndex) {
        if ((oindex == 0 || oindex > d->focusIndex) && oindex < d->lists.count()) {
            d->lists.at(oindex)->clearSelection();
        }
        if (d->focusIndex == 0 || oindex < d->focusIndex) {
            d->lists.at(d->focusIndex)->initialSelection();
        }
    }

    if (pass) {
        d->lists.at(d->focusIndex)->keyPressEvent(event);
    }
}

void PassagewayView::hideEvent(QHideEvent * event)
{
    if (d->popup) {
        d->popup->hide();
    }
}

} // namespace Lancelot

