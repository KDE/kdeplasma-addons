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

#ifndef TASKITEMDELEGATE_H
#define TASKITEMDELEGATE_H

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QModelIndex>

#include "taskitem.h"

class TaskItemDelegate : public QStyledItemDelegate
{
public:
  TaskItemDelegate(QObject *parent = 0);

  virtual void paint (QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;
  virtual QSize sizeHint (const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;

  virtual bool eventFilter(QObject* object, QEvent* event);
  
  void setSmallTasks(bool small);
  
  void paintTask(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  void paintPriorityHeader(QPainter* painter, const QRectF &rect, const QModelIndex &index) const;
  void paintDueHeader(QPainter* painter, const QRectF &rect, const QModelIndex &index) const;
  
  QColor itemPriorityColor(const QModelIndex &index) const;

  static const qreal MARGIN;
  static const qreal MARGINH;
  static const qreal COLORTAB;
  static const QString tagsPrefix;
  
  static const QColor priority1;
  static const QColor priority2;
  static const QColor priority3;
  static const QColor priorityNone;

protected:
  bool dragging;
  bool smallTasks;
  qreal leftMargin;
};

#endif // TASKITEMDELEGATE_H
