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

#include "taskitemdelegate.h"

#include "taskeditor.h"

#include <QApplication>

#include <Plasma/Theme>
#include <Plasma/Svg>

#include <QColor>
#include <QLinearGradient>
#include <QCheckBox>

#include <KDebug>

const qreal TaskItemDelegate::MARGIN = 1.0;
const qreal TaskItemDelegate::MARGINH = 6.0;
const qreal TaskItemDelegate::COLORTAB = MARGIN*6;

// These colors are hardcoded because they represent priorities and need to be the same for visual recognition purposes.
const QColor TaskItemDelegate::priority1 = QColor(255, 123, 0); // reddish-orange
const QColor TaskItemDelegate::priority2 = QColor(0, 132, 255); // Darker blue
const QColor TaskItemDelegate::priority3 = QColor(53, 201, 255);  // Ligher Blue
const QColor TaskItemDelegate::priorityNone = QColor(241, 241, 241); // Almost white

const QString TaskItemDelegate::tagsPrefix = i18n("Tags: ");

TaskItemDelegate::TaskItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent),
    dragging(false)
{
  setSmallTasks(false);
}

void TaskItemDelegate::setSmallTasks(bool small)
{
  smallTasks = small;
  leftMargin = small ? 0 : MARGIN;
}


bool TaskItemDelegate::eventFilter(QObject* object, QEvent* event) {
  if (event->type() == QEvent::DragMove)
    dragging = true;
  if (event->type() == QEvent::Drop || QEvent::DragLeave)
    dragging = false;
  return QStyledItemDelegate::eventFilter(object, event);
}

QColor TaskItemDelegate::itemPriorityColor(const QModelIndex &index) const
{
  switch (index.data(Qt::RTMPriorityRole).toInt()) { // Set color based on priority
    case 1:
      return priority1;
    case 2:
      return priority2;
    case 3:
      return priority3;
    case 4:
      return priorityNone;
  }
  return QColor();
}

void TaskItemDelegate::paintDueHeader(QPainter* painter, const QRectF& rect, const QModelIndex &index) const
{
  painter->save();
  
  QFontMetrics fm = QApplication::fontMetrics();
  QString dueString = index.data(Qt::DisplayRole).toString();

  QLinearGradient gradient(rect.bottomLeft(), rect.bottomRight());
  gradient.setColorAt(0, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
  gradient.setColorAt((qreal)(0.9*fm.width(dueString)+2*MARGIN)/rect.width(), Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
  gradient.setColorAt((qreal)(1.3*fm.width(dueString)+2*MARGIN)/rect.width(), Qt::transparent);
  painter->setBrush(Qt::NoBrush);
  painter->setPen(QPen(QBrush(gradient), 1));
  painter->drawLine(rect.bottomLeft()+QPoint(0, -MARGIN), rect.bottomRight()+QPoint(0, -MARGIN));
  
  painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
  painter->drawText(rect.bottomLeft()+QPoint(MARGIN, -2*MARGIN) ,dueString);
  
  painter->restore();
}

void TaskItemDelegate::paintPriorityHeader(QPainter* painter, const QRectF& rect, const QModelIndex &index) const
{
  painter->save();
  
  //Draw colored gradient as priority background
  QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
  gradient.setColorAt(0, itemPriorityColor(index));
  gradient.setColorAt(1, Qt::transparent);
  
  int h = QApplication::fontMetrics().height();
  QPainterPath path;
  path.moveTo(rect.bottomLeft());
  path.quadTo(rect.bottomLeft()+QPointF(0, -h), rect.bottomLeft()+QPointF(2, -h));
  path.lineTo(rect.bottomRight()-QPointF(2, h));
  path.quadTo(rect.bottomRight()+QPointF(0, -h), rect.bottomRight());
  painter->setBrush(Qt::NoBrush);
  QPen thickPen(itemPriorityColor(index));
  thickPen.setWidth(2);
  painter->setPen(thickPen);
  painter->drawPath(path);
  
  // Draw priority text
  QString priority = index.data(Qt::DisplayRole).toString();
  painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
  painter->setBrush(Qt::NoBrush);
  painter->drawText(rect.bottomLeft()+QPoint(MARGIN, -2*MARGIN) ,priority);
 
  painter->restore();
}

void TaskItemDelegate::paintTask(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex& index) const
{
  QRect rect = option.rect;
  QFontMetrics fm = QApplication::fontMetrics();
  
  // Painting Order:
  //    Background wash
  //    Mouseover highlight
  //    Priority Tab on left
  //    Bounding Box
  //    Task name
  //    If not small, other task information
  
  painter->save();
  if (smallTasks) { // Paint the background for a small task
    painter->save();
    painter->setOpacity(0.5);
    painter->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect.adjusted(leftMargin, MARGIN, -MARGIN, -MARGIN), 3, 3);
    painter->restore();
  }
  else { // Paint the background for a two row task
    painter->save();
    painter->setOpacity(0.5);
    painter->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    painter->drawRect(option.rect.adjusted(leftMargin, MARGIN, -MARGIN, -MARGIN));
    
    painter->restore();
    painter->drawRect(QRectF(option.rect.topLeft()+QPointF(leftMargin, MARGIN), option.rect.bottomLeft()+QPointF(COLORTAB, -MARGIN))); 
  }
  
  if (option.state & QStyle::State_MouseOver) {
    painter->save();
    painter->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::HighlightColor));
    painter->setPen(Qt::NoPen);
    if (smallTasks)
      painter->drawRoundedRect(rect.adjusted(leftMargin, MARGIN, -MARGIN, -MARGIN), 3, 3);
    else
      painter->drawRect(rect.adjusted(leftMargin, MARGIN, -MARGIN, -MARGIN));
    painter->restore();
  }
  
  //Paint Priority tab on the left
  painter->setPen(Qt::NoPen);
  painter->setBrush(itemPriorityColor(index));
  painter->drawRoundedRect(QRectF(option.rect.topLeft()+QPointF(leftMargin, MARGIN), option.rect.bottomLeft()+QPointF(COLORTAB, 0)), 3, 3);

  
  // Draw Bounding rect/box
  painter->save();
  painter->setOpacity(.5);
  painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
  painter->setBrush(Qt::NoBrush);
  if (smallTasks)
    painter->drawRoundedRect(option.rect.adjusted(leftMargin, MARGIN, -MARGIN, -MARGIN), 3, 3);
  else
    painter->drawRect(option.rect.adjusted(leftMargin, MARGIN, -MARGIN, -MARGIN));
  painter->restore();
  
  
  // Draw Task information
  painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
  
  QString task = index.data(Qt::RTMNameRole).toString();
  QString due = index.data(Qt::RTMDueRole).toDate().toString("MMM d");
  QString tags = index.data(Qt::RTMTagsRole).toStringList().join(", ");
  if (!tags.isEmpty())
    tags.prepend(tagsPrefix);
  
  QFont font = painter->font();
  font.setPointSize(font.pointSize());
  QFontMetrics largerFM(font);
  
  QDate date = index.data(Qt::RTMDueRole).toDate();
  
  if (smallTasks && date.isValid() && date < QDate::currentDate()) {
    // Elide (text with dots like th...) if wider than view and draw
    font.setBold(true);
    QFontMetrics boldLargerFM(font);
    due = due.append(" "); // So that we have a space after the date before the task name
    task = largerFM.elidedText(task, Qt::ElideRight, option.rect.width()-COLORTAB-3*MARGIN - boldLargerFM.width(due));
    painter->setFont(font);
    painter->drawText(option.rect.topLeft()+QPoint(COLORTAB+MARGIN,largerFM.height()),due);
    
    font.setBold(false);
    painter->setFont(font);
    painter->drawText(option.rect.topLeft()+QPoint(COLORTAB+3*MARGIN + boldLargerFM.width(due),boldLargerFM.height()),task);
  }
  else {
    // Elide (text with dots like th...) if wider than view and draw
    task = largerFM.elidedText(task, Qt::ElideRight, option.rect.width()-COLORTAB-3*MARGIN);
    painter->setFont(font);
    painter->drawText(option.rect.topLeft()+QPoint(COLORTAB+MARGIN,largerFM.height()),task);
  }
  
  if (smallTasks) {
    painter->restore();
    return;
  }
  
  // Set our smaller attributes and blue hue for additional text
  font.setPointSize(font.pointSize()-1);
  font.setItalic(true);
  painter->setFont(font);
  fm = QFontMetrics(font);
  QColor moreText(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
  
  qreal red = moreText.redF();
  qreal green = moreText.greenF();
  qreal blue = moreText.blueF();
  qreal colorChangeMult = 0.85;
  if (blue < colorChangeMult)
    blue /= colorChangeMult;
  else {
    red *= colorChangeMult;
    green *= colorChangeMult;
  }
  moreText.setRgbF(red, green, blue);
  painter->setPen(moreText);
  
  // Elide the due from the front because "Due on" isn't necessary
  due = fm.elidedText(due, Qt::ElideLeft, option.rect.width() - COLORTAB+3*MARGIN - fm.width(tags));
  painter->drawText(option.rect.bottomLeft()+QPoint(COLORTAB+MARGIN, -MARGINH), tags);
  painter->drawText(option.rect.bottomRight()+QPoint(-COLORTAB+MARGIN-fm.width(due), -MARGINH), due);

  painter->restore();
}

void TaskItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const
{
  switch (index.data(Qt::RTMItemType).toInt()) {
    case RTMPriorityHeader:
      paintPriorityHeader(painter, option.rect, index);
      break;
    case RTMDateHeader:
      paintDueHeader(painter, option.rect, index);
      break;
    case RTMTaskItem:
      paintTask(painter, option, index);
      break;
  }
}

QSize TaskItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index) const
{
  Q_UNUSED(option)
  if (smallTasks || index.data(Qt::RTMItemType).toInt() != RTMTaskItem) {
    QString priority = index.data(Qt::DisplayRole).toString();
    QSize size;
    if (index.data(Qt::RTMItemType).toInt() == RTMDateHeader)
      size.setHeight(QApplication::fontMetrics().height()+MARGINH*1.5);
    else 
      size.setHeight(QApplication::fontMetrics().height()+MARGINH);
    size.setWidth(QApplication::fontMetrics().width(priority));
    return size;
  }
  else {
    QString task = index.data(Qt::RTMNameRole).toString();
    QString due = index.data(Qt::RTMDueRole).toDate().toString("MMM d");
    QString tags = index.data(Qt::RTMTagsRole).toStringList().join(", ");
    if (!tags.isEmpty())
      tags.prepend(tagsPrefix);

    //QSize size = QStyledItemDelegate::sizeHint(option, index);
    QSize size;
    size.setHeight(MARGINH + MARGINH + QApplication::fontMetrics().height()*2);
    size.setWidth(MARGIN + MARGIN + QApplication::fontMetrics().width(task+due));
    return size;
  }
}
