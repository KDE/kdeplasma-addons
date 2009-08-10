/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "taskitem.h"


#include <KDebug>

TaskItem::TaskItem()
{
  setData(RTMTaskItem, Qt::RTMItemType);
}

bool TaskItem::operator<(const QStandardItem& other) const {
  return (data(Qt::RTMSortRole).toInt() < other.data(Qt::RTMSortRole).toInt());
}

int TaskItem::type() const {
  return RTMTaskItem; // FIXME: Use the Qt macro to grab an unused number per app
}


HeaderItem::HeaderItem(RTMItemType type)
{
  setData(type, Qt::RTMItemType);
  m_type = type;
}

bool HeaderItem::operator<(const QStandardItem& other) const {
  return (data(Qt::RTMSortRole).toInt() <= other.data(Qt::RTMSortRole).toInt()); // <= intentional. Allows PriorityItems to be above other items
}

int HeaderItem::type() const {
  return m_type;
}


