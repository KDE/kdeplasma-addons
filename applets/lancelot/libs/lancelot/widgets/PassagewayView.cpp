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

#include "PassagewayView.h"

namespace Lancelot
{

class PassagewayViewSizer: public ColumnLayout::ColumnSizer {
public:
    PassagewayViewSizer()
    {
        m_sizer = ColumnLayout::ColumnSizer::create(ColumnSizer::ColumnSizer::GoldenSizer);
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
    Private(PassagewayViewModel * entranceModel,
            PassagewayViewModel * atlasModel,
            PassagewayView * p)
      : layout(NULL), buttonsLayout(NULL), listsLayout(NULL), parent(p)
    {
        parent->setLayout(layout = new NodeLayout());

        layout->addItem(
            buttonsLayout = new QGraphicsLinearLayout(Qt::Horizontal),
            NodeLayout::NodeCoordinate(0, 0, 0, 0),
            NodeLayout::NodeCoordinate(1, 0, 0, 32)
        );

        ExtenderButton * spacer =
            new ExtenderButton(parent);
        spacer->setGroupByName("PassagewayView-Spacer");
        spacer->setMaximumSize(QSizeF(EXTENDER_SIZE, 32));
        spacer->setPreferredSize(QSizeF(EXTENDER_SIZE, 32));
        spacer->setMinimumSize(QSizeF(EXTENDER_SIZE, 32));
        buttonsLayout->addItem(spacer);

        layout->addItem(
            listsLayout = new ColumnLayout(),
            NodeLayout::NodeCoordinate(0, 0, 0, 32),
            NodeLayout::NodeCoordinate(1, 1, 0, 0)
        );

        listsLayout->setColumnCount(13);
        listsLayout->setSizer(new PassagewayViewSizer());
        listsLayout->setColumnCount(3);

        buttonsLayout->setSpacing(0.0);

        next(Step("", QIcon(), entranceModel));
        next(Step("", QIcon(), atlasModel));

        lists.at(0)->setExtenderPosition(Lancelot::LeftExtender);
        lists.at(0)->setCategoriesActivable(false);
        lists.at(1)->setCategoriesActivable(true);
    }

    ~Private()
    {
        delete buttonsLayout;
        delete listsLayout;
        delete layout;

        foreach(ExtenderButton * button, buttons) {
            delete button;
        }
        foreach(ActionListView * list, lists) {
            delete list;
        }
        foreach(Step * step, path) {
            delete step;
        }
    }

    class Step {
    public:
        Step(QString t, QIcon i, PassagewayViewModel * m)
            : title(t), icon(i), model(m) {};
        QString title;
        QIcon icon;
        PassagewayViewModel * model;
    };

    void back(int steps)
    {
        for (int i = 0; i < steps; ++i) {
            if (buttons.size() > 2) {
                buttons.at(buttons.size() - 3)->setGroupByName(parent->group()->name() + "-InactiveButton");
                buttons.at(buttons.size() - 3)->setExtenderPosition(Lancelot::NoExtender);
            }
            ExtenderButton * button = buttons.takeLast();

            ActionListView * list   = lists.takeLast();
            path.takeLast();

            buttonsLayout->removeItem(button);
            layout->activate();
            listsLayout->pop();

            delete button;
            delete list;
        }
    }

    void next(Step newStep)
    {
        Step * step = new Step(newStep);
        Instance::setActiveInstanceAndLock(parent->group()->instance());
        ExtenderButton * button =
            new ExtenderButton(step->icon, step->title, "", parent);
        ActionListView * list   =
            new ActionListView(step->model, parent);
        Instance::releaseActiveInstanceLock();

        button->setIconSize(QSize(24, 24));
        button->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        button->setGroupByName(parent->group()->name() + "-InactiveButton");
        button->setExtenderPosition(Lancelot::NoExtender);

        buttons.append(button);
        button->setZValue((qreal)buttons.count());

        if (buttons.size() > 2) {
            buttons.at(buttons.size() - 3)->setGroupByName(parent->group()->name() + "-Button");
            buttons.at(buttons.size() - 3)->setExtenderPosition(Lancelot::LeftExtender);
        }

        list->setCategoriesGroupByName("ActionListView-CategoriesPass");
        list->setExtenderPosition(RightExtender);
        list->setCategoriesActivable(true);

        lists.append(list);
        path.append(step);

        buttonsLayout->addItem(button);
        listsLayout->push(list);

        QObject::connect(
            list, SIGNAL(activated(int)),
            parent, SLOT(listItemActivated(int))
        );

        QObject::connect(
            button, SIGNAL(activated()),
            parent, SLOT(pathButtonActivated())
        );
    }

    QList < Step * > path;
    QList < ExtenderButton * > buttons;
    QList < ActionListView * > lists;

    NodeLayout          * layout;
    ColumnLayout::ColumnSizer   * sizer;
    QGraphicsLinearLayout       * buttonsLayout;
    ColumnLayout                * listsLayout;
    PassagewayView              * parent;
};

PassagewayView::PassagewayView(QGraphicsItem * parent)
    : Panel(parent), d(new Private(NULL, NULL, this))
{
    setGroupByName("PassagewayView");
    L_WIDGET_SET_INITIALIZED;
}

PassagewayView::PassagewayView(PassagewayViewModel * entranceModel,
    PassagewayViewModel * atlasModel, QGraphicsItem * parent)
    : Panel(parent), d(new Private(entranceModel, atlasModel, this))
{
    setGroupByName("PassagewayView");
    L_WIDGET_SET_INITIALIZED;
}

void PassagewayView::pathButtonActivated()
{
    for (int i = d->buttons.size() - 1; i >= 0; --i) {
        if (d->buttons.at(i) == sender()) {
            d->back(d->buttons.size() - i - 2);
        }
    }
}

void PassagewayView::listItemActivated(int index)
{
    int activatedListIndex = d->lists.indexOf((ActionListView *)sender());
    if (activatedListIndex == 0) {
        // something in the entrance is clicked
        // we don't want to remove the first level
        // of atlas as well
        d->back(d->lists.size() - activatedListIndex - 2);
    } else {
        d->back(d->lists.size() - activatedListIndex - 1);
    }

    PassagewayViewModel * model = d->path.at(activatedListIndex)->model;
    if (model) {
        model = model->child(index);
        if (model) {
            d->next(Private::Step(model->modelTitle(), model->modelIcon(), model));
        }
    }
}

PassagewayView::~PassagewayView()
{
    delete d;
}

// Entrance
void PassagewayView::setEntranceModel(PassagewayViewModel * model)
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
void PassagewayView::setAtlasModel(PassagewayViewModel * model)
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

void PassagewayView::setGroup(WidgetGroup * g)
{
    Widget::setGroup(g);

    int i = d->buttons.size();
    foreach (ExtenderButton * button, d->buttons) {
        --i;
        if (i < 2) {
            button->setGroupByName(group()->name() + "-InactiveButton");
        } else {
            button->setGroupByName(group()->name() + "-Button");
        }
    }

    foreach (ActionListView * list, d->lists) {
        list->setGroupByName(group()->name() + "-List");
    }
}

} // namespace Lancelot

