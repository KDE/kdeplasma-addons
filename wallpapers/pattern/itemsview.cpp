/*
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "itemsview.h"

#include <QtGui/QScrollBar>

ItemsView::ItemsView(QWidget* parent)
    : QListView(parent)
{
}

void ItemsView::wheelEvent(QWheelEvent* event)
{
    // this is a workaround because scrolling by mouse wheel is broken in Qt list views for big items
    verticalScrollBar()->setSingleStep(10);
    QListView::wheelEvent(event);
}
