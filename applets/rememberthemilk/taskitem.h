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

#ifndef TASKITEM_H
#define TASKITEM_H

#include <QGraphicsWidget>
#include <QStandardItem>
#include <Plasma/Theme>
#include <Plasma/Svg>

namespace Qt
{
  enum ItemUserDataRole { 
                  RTMSortRole = 64, 
                  RTMTaskIdRole = 65, 
                  RTMPriorityRole = 66,
                  RTMNameRole = 67,
                  RTMTagsRole = 68,
                  RTMDueRole = 69,
                  RTMTimeTRole = 70,
                  RTMCompletedRole = 71,
                  RTMItemType = 72
                };
}

enum RTMItemType {
      RTMTaskItem = 1001,
      RTMPriorityHeader = 1002,
      RTMDateHeader = 1003
};

enum RTMSortType {
      RTMSortByPriority,
      RTMSortByDate
};

class TaskItem : public QStandardItem
{
public:
  TaskItem();
  virtual bool operator<( const QStandardItem & other ) const;
  virtual int type() const;
};

class HeaderItem : public QStandardItem
{
public:
  HeaderItem(RTMItemType type);
  virtual bool operator<(const QStandardItem& other) const;
  virtual int type() const;
  
private:
  RTMItemType m_type;  
};

#endif // TASKITEM_H
