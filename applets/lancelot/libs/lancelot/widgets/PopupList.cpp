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
#include <lancelot/widgets/ActionListView.h>
#include <QBasicTimer>

#define ITEM_HEIGHT 32

namespace Lancelot {

class PopupList::Private {
public:
    Private(PopupList * parent)
        : closeTimeout(2000)
    {
        scene = new QGraphicsScene();
        list = new ActionListView();
        connect(list->list()->itemFactory(), SIGNAL(updated()), parent, SLOT(updateSize()));
        connect(list->list()->itemFactory(), SIGNAL(itemInserted(int)), parent, SLOT(updateSize()));
        connect(list->list()->itemFactory(), SIGNAL(itemDeleted(int)), parent, SLOT(updateSize()));
        connect(list->list()->itemFactory(), SIGNAL(itemAltered(int)), parent, SLOT(updateSize()));

        scene->addItem(list);
        parent->setGraphicsWidget(list);
        parent->resize(256, 384);
    };

    ~Private()
    {
        delete list;
        delete scene;
    }

    ActionListView * list;
    QGraphicsScene * scene;
    QBasicTimer timer;
    int closeTimeout;
};

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

ActionListView * PopupList::list() const
{
    return d->list;
}

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

    Plasma::Dialog::showEvent(event);
    d->timer.stop();
}

void PopupList::updateSize()
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    resize(width(), d->list->list()->
            itemFactory()->itemCount() * ITEM_HEIGHT + top + bottom);
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

} // namespace Lancelot

