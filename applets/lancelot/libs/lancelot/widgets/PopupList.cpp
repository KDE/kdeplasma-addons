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
#define MENU_WIDTH 200
#define POPOFFSET 16

namespace Lancelot {

PopupList::Private::Private(PopupList * parent)
    : closeTimeout(1000),
      listModel(NULL),
      treeModel(NULL),
      child(NULL),
      openAction(PopupList::PopupNew),
      q(parent)
{
    scene = new QGraphicsScene();
    list = new ActionListView();

    scene->addItem(list);
    parent->setGraphicsWidget(list);
    parent->resize(MENU_WIDTH, 384);
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
        switch (openAction) {
            case OpenInside:
                list->setModel(treeModel->child(index));
                break;
            case PopupNew:
                if (!child) {
                    child = new PopupList();
                }
                child->setModel(treeModel->child(index));
                child->exec(QCursor::pos(), q);
                break;
            default:
                break;
                // nothing
        }
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
        d->list->setCategoriesGroup(Instance::activeInstance()
                ->group("PopupList-CategoriesPass"));
    } else {
        d->listModel = NULL;
        d->list->setCategoriesActivable(true);
        d->list->setCategoriesGroup(Instance::activeInstance()
                ->group("PopupList-CategoriesPass"));
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
    d->list->setCategoriesGroup(Instance::activeInstance()
            ->group("PopupList-Categories"));

    d->list->setItemHeight(ITEM_HEIGHT, Qt::MinimumSize);
    d->list->setItemHeight(ITEM_HEIGHT, Qt::PreferredSize);
    d->list->setItemHeight(ITEM_HEIGHT, Qt::MaximumSize);

    d->list->setCategoryHeight(ITEM_HEIGHT, Qt::MinimumSize);
    d->list->setCategoryHeight(ITEM_HEIGHT, Qt::PreferredSize);
    d->list->setCategoryHeight(ITEM_HEIGHT, Qt::MaximumSize);

    d->list->setItemIconSize(ICON_SIZE);
    d->list->setCategoryIconSize(ICON_SIZE);

    Plasma::Dialog::showEvent(event);
    d->list->setFocus();
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

PopupList::SublevelOpenAction PopupList::sublevelOpenAction() const
{
    return d->openAction;
}

void PopupList::setSublevelOpenAction(SublevelOpenAction action)
{
    d->openAction = action;
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
        if (!d->child || d->child->isHidden()) {
            close();
            d->timer.stop();
        }
    }
    Plasma::Dialog::timerEvent(event);
}

void PopupList::updateSize()
{
    d->list->resize(MENU_WIDTH, d->list->list()->
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

void PopupList::exec(const QPoint & p, PopupList * parent)
{
    updateSize();

    QRect g = geometry();
    g.moveTopLeft(p);

    QRect screen = QApplication::desktop()->screenGeometry(
            QApplication::desktop()->screenNumber(p)
        );

    g.moveLeft(parent->geometry().right() - POPOFFSET);
    if (g.right() > screen.right()) {
        g.moveRight(parent->geometry().left() + POPOFFSET);
    }

    if (g.bottom() > screen.bottom()) {
        g.moveBottom(screen.bottom());
    } else if (g.top() < screen.top()) {
        g.moveTop(screen.top());
    }

    setGeometry(g);

    show();
}

bool PopupList::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            close();
        }
    }
    return Plasma::Dialog::eventFilter(object, event);
}

} // namespace Lancelot

