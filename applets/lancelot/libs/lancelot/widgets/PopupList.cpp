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

#include "PopupList.h"
#include "PopupList_p.h"
#include <KMessageBox>
#include <QApplication>
#include <QDesktopWidget>

#define ITEM_HEIGHT 24
#define ICON_SIZE QSize(16, 16)

namespace Lancelot {

PopupList::Private::Private(PopupList * parent)
    : closeTimeout(1000),
      listModel(NULL),
      treeModel(NULL),
      q(parent)
{
    scene = new QGraphicsScene();
    list = new ActionListView();

    scene->addItem(list);
    parent->setGraphicsWidget(list);
    parent->resize(256, 384);
}

void PopupList::Private::connectSignals()
{
    connect(list->list()->itemFactory(), SIGNAL(updated()),
            q, SLOT(updateSize()));
    connect(list->list()->itemFactory(), SIGNAL(itemInserted(int)),
            q, SLOT(updateSize()));
    connect(list->list()->itemFactory(), SIGNAL(itemDeleted(int)),
            q, SLOT(updateSize()));
    connect(list->list()->itemFactory(), SIGNAL(itemAltered(int)),
            q, SLOT(updateSize()));
    connect(list->list()->itemFactory(), SIGNAL(activated(int)),
            this, SLOT(listItemActivated(int)));
}

PopupList::Private::~Private()
{
    delete list;
    delete scene;
}

void PopupList::Private::listItemActivated(int index)
{
    qDebug() << "listItemActivated"
        << "tree" << (void *) treeModel
        << "list" << (void *) listModel;
    if (treeModel && treeModel->isCategory(index)) {
        qDebug() << treeModel->isCategory(index);
        list->setModel(treeModel->child(index));
    }
}

PopupList::PopupList(QWidget * parent, Qt::WindowFlags f)
  : Plasma::Dialog(parent, f),
    d(new Private(this))
{
    setWindowFlags(Qt::Popup | Qt::WindowStaysOnTopHint);
}

PopupList::~PopupList()
{
    delete d;
}

void PopupList::setModel(ActionListModel * model)
{
    if (!model) {
        return;
    }

    d->treeModel = dynamic_cast < ActionTreeModel * > (model);

    if (!d->treeModel) {
        d->listModel = model;
    }

    d->list->setModel(model);
    d->connectSignals();
}

// ActionListView * PopupList::list() const
// {
//     return d->list;
// }

void PopupList::showEvent(QShowEvent * event)
{
    d->list->setGroupByName("PopupList");
    d->list->setItemsGroup(Instance::activeInstance()
            ->group("PopupList-Items"));

    d->list->setItemHeight(ITEM_HEIGHT, Qt::MinimumSize);
    d->list->setItemHeight(ITEM_HEIGHT, Qt::PreferredSize);
    d->list->setItemHeight(ITEM_HEIGHT, Qt::MaximumSize);

    d->list->setCategoryHeight(ITEM_HEIGHT, Qt::MinimumSize);
    d->list->setCategoryHeight(ITEM_HEIGHT, Qt::PreferredSize);
    d->list->setCategoryHeight(ITEM_HEIGHT, Qt::MaximumSize);

    d->list->setItemIconSize(ICON_SIZE);
    d->list->setCategoryIconSize(ICON_SIZE);

    Plasma::Dialog::showEvent(event);
    d->timer.stop();
}

void PopupList::setCloseTimeout(int timeout)
{
    d->closeTimeout = timeout;
}

int PopupList::closeTimeout() const
{
    return d->closeTimeout;
}

void PopupList::enterEvent(QEvent * event)
{
    d->timer.stop();
    Plasma::Dialog::enterEvent(event);
}

void PopupList::leaveEvent(QEvent * event)
{
    if (d->closeTimeout) {
        d->timer.start(d->closeTimeout, this);
    }
    Plasma::Dialog::leaveEvent(event);
}

void PopupList::timerEvent(QTimerEvent * event)
{
    if (d->timer.timerId() == event->timerId()) {
        close();
        d->timer.stop();
    }
    Plasma::Dialog::timerEvent(event);
}

void PopupList::updateSize()
{
    d->list->resize(256, d->list->list()->
            itemFactory()->itemCount() * ITEM_HEIGHT);
}

void PopupList::exec(const QPoint & p)
{
    updateSize();

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
}

} // namespace Lancelot

