/***************************************************************************
 *   Copyright (C) 2007 by Jesper Thomschutz <jesperht@yahoo.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Thanks to ThomasZ and Jens B-W for making this prettier than just some
// off-centered text and gradients ;)

#include "piece.h"

#include <QPen>
#include <QPainter>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetrics>

Piece::Piece(int size, int id, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
{
  m_size = size;
  m_id = id;
  m_numeral = true;
}

int Piece::getId()
{
  return m_id;
}

void Piece::showNumeral(bool show)
{
  m_numeral = show;
}

void Piece::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
  if (m_id == 0) {
    return;
  }

  QGraphicsPixmapItem::paint(painter, option, widget);

  if (!m_numeral) {
    return;
  }

  QFont font = painter->font();
  font.setBold(true);
  font.setPointSize(14);
  painter->setFont(font);

  QFontMetrics m(font);
  QString text = QString::number(m_id);

  QPen pen = painter->pen();

  pen.setColor(QColor(0, 0, 0, 90));
  painter->setPen(pen);
  painter->drawText(((m_size / 2) - m.width(text) / 2) + 2,
                    ((m_size / 2) + m.ascent() / 2) + 2,
                    text);

  pen.setColor(QColor(Qt::white));
  painter->setPen(pen);
  painter->drawText((m_size / 2) - m.width(text) / 2,
                    (m_size / 2) + m.ascent() / 2,
                    text);
}

void Piece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  Q_UNUSED(event);
  if (m_id == 0) {
    return;
  }

  emit pressed(this);
}
