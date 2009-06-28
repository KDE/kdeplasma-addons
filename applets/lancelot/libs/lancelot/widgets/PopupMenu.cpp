/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "PopupMenu.h"
#include <lancelot/widgets/ActionListView.h>
#include <lancelot/models/StandardActionListModel.h>
#include <QApplication>
#include <QSignalMapper>

#define ITEM_HEIGHT 32

namespace Lancelot {

// ActionsModel
class ActionsModel:
    public StandardActionListModel {
public:
    ActionsModel();
    ~ActionsModel();

    QAction * addAction(const QIcon & icon, const QString & title);
    void addAction(QAction * action);

    L_Override void activate(int index);

    QList < QAction * > m_actions;
};

ActionsModel::ActionsModel()
{
}

ActionsModel::~ActionsModel()
{
    qDeleteAll(m_actions);
}

QAction * ActionsModel::addAction(const QIcon & icon, const QString & title)
{
    QAction * action = new QAction(icon, title, NULL);
    m_actions << action;
    add(
            title, QString(), icon, QString()
       );
    return action;
}

void ActionsModel::addAction(QAction * action)
{
    m_actions << action;
}

void ActionsModel::activate(int index)
{
    if (index < 0 || index >= m_actions.size()) {
        return;
    }

    m_actions.at(index)->trigger();
}

// PopupMenu

class PopupMenu::Private {
public:
    Private(PopupMenu * parent)
    {
      Q_UNUSED(parent);
    };

    ~Private()
    {
    }

    ActionsModel * model;
    QSignalMapper mapper;
    QAction * chosenAction;
};

PopupMenu::PopupMenu(QWidget * parent, Qt::WindowFlags f)
  : PopupList(parent, f),
    d(new Private(this))
{
    setModel(d->model = new ActionsModel());
    connect(&d->mapper, SIGNAL(mapped(int)),
             this, SLOT(actionChosen(int)));

}

PopupMenu::~PopupMenu()
{
    delete d;
}

void PopupMenu::addAction(QAction * action)
{
    d->model->addAction(action);
}

QAction * PopupMenu::addAction(const QIcon & icon, const QString & title)
{
    QAction * result = d->model->addAction(icon, title);
    connect(result, SIGNAL(triggered()),
            &d->mapper, SLOT(map()));
    d->mapper.setMapping(result, d->model->m_actions.size() - 1);
    return result;
}

QAction * PopupMenu::exec(const QPoint & p, QAction * action)
{
    Q_UNUSED(action);
    d->chosenAction = NULL;
    PopupList::exec(p);

    while (isVisible()) {
        QApplication::processEvents();
    }
    return d->chosenAction;
}

void PopupMenu::actionChosen(int index)
{
    d->chosenAction = action(index);
    close();
}

QAction * PopupMenu::action(int index)
{
    if (index < 0 || index >= d->model->m_actions.size()) {
        return NULL;
    }
    return d->model->m_actions.at(index);
}

} // namespace Lancelot

