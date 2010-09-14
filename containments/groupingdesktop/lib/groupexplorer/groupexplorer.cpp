/*
 *   Copyright (C) 2007 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *   Copyright 2010 Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "groupexplorer.h"

#include <QGraphicsLinearLayout>

#include <KIcon>

#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/ToolButton>

#include "groupiconlist.h"

class GroupExplorerPrivate
{
    public:
        GroupExplorerPrivate(GroupExplorer *w)
            : q(w),
              containment(0),
              iconSize(16) //FIXME bad!
        {
        }

        void init();
        void containmentDestroyed();
        void setOrientation(Qt::Orientation orientation);

        GroupExplorer *q;
        Plasma::ToolButton *close;
        Plasma::Containment *containment;

        /// Widget that lists the groups
        GroupIconList *groupList;
        QGraphicsLinearLayout *filteringLayout;
        QGraphicsLinearLayout *mainLayout;
        int iconSize;
};

void GroupExplorerPrivate::init()
{
    //init widgets
    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setSpacing(0);
    filteringLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    groupList = new GroupIconList(Plasma::BottomEdge);
    close = new Plasma::ToolButton;
    close->setIcon(KIcon("dialog-close"));

    QObject::connect(close, SIGNAL(clicked()), q, SIGNAL(closeClicked()));

    mainLayout->addItem(filteringLayout);
    mainLayout->addItem(groupList);
    groupList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->setAlignment(groupList, Qt::AlignTop | Qt::AlignHCenter);

    q->setLayout(mainLayout);
}

void GroupExplorerPrivate::setOrientation(Qt::Orientation orientation)
{
    if (orientation == Qt::Horizontal) {
//         mainLayout->removeItem(filteringWidget);
        mainLayout->removeItem(close);
        filteringLayout->addStretch();
        filteringLayout->addItem(close);
        filteringLayout->setAlignment(close, Qt::AlignVCenter | Qt::AlignHCenter);
    } else {
        filteringLayout->removeAt(0);
        filteringLayout->removeItem(close);
//         mainLayout->insertItem(0, filteringWidget);
        mainLayout->addItem(close);
        mainLayout->setStretchFactor(groupList, 10);
    }
}

void GroupExplorerPrivate::containmentDestroyed()
{
    containment = 0;
}

GroupExplorer::GroupExplorer(Plasma::Location location)
             : QGraphicsWidget(0),
               d(new GroupExplorerPrivate(this))
{
    d->init();
    setLocation(location);
}

GroupExplorer::~GroupExplorer()
{
     delete d;
}

void GroupExplorer::setLocation(Plasma::Location location)
{
    d->setOrientation(location == Plasma::LeftEdge || location == Plasma::RightEdge ? Qt::Vertical : Qt::Horizontal);
    d->groupList->setLocation(location);
    d->groupList->updateList();
}

void GroupExplorer::setIconSize(int size)
{
    d->groupList->setIconSize(size);
    adjustSize();
}

void GroupExplorer::setContainment(Plasma::Containment *containment)
{
    if (d->containment != containment) {
        if (d->containment) {
            d->containment->disconnect(this);
        }

        d->containment = containment;

        if (d->containment) {
            connect(d->containment, SIGNAL(destroyed(QObject*)), this, SLOT(containmentDestroyed()));
            connect(d->containment, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));
        }
    }
}

void GroupExplorer::showEvent(QShowEvent *e)
{
    QGraphicsWidget::showEvent(e);
}

void GroupExplorer::immutabilityChanged(Plasma::ImmutabilityType type)
{
    if (type != Plasma::Mutable) {
        emit closeClicked();
    }
}

#include "groupexplorer.moc"
