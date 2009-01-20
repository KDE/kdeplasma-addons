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
#include <lancelot/models/ActionListViewModels.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QSignalMapper>

#define ITEM_HEIGHT 32

namespace Lancelot {

// ActionsModel
class ActionsModel:
    public StandardActionListViewModel {
public:
    ActionsModel();
    ~ActionsModel();

    QAction * addAction(const QIcon & icon, const QString & title);

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
    list()->setModel(d->model = new ActionsModel());
    connect(&d->mapper, SIGNAL(mapped(int)),
             this, SLOT(actionChosen(int)));

}

PopupMenu::~PopupMenu()
{
    delete d;
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
    updateSize();
    d->chosenAction = NULL;

    QRect g = geometry();
    g.moveTopLeft(p);

    QRect screen = QApplication::desktop()->screenGeometry(
        QApplication::desktop()->screenNumber(p)
        );

    if (g.right() > screen.right()) {
        g.moveRight(screen.right());
    } else if (g.left() < screen.left()) {
        g.moveLeft(screen.left());
    }

    if (g.bottom() > screen.bottom()) {
        g.moveBottom(screen.bottom());
    } else if (g.top() < screen.top()) {
        g.moveTop(screen.top());
    }

    setGeometry(g);

    show();

    while (isVisible()) {
        QApplication::processEvents();
    }
    kDebug() << "is show stopping me?";
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

