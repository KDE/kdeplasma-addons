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
#include <QGraphicsSceneMouseEvent>
#include <QFontMetrics>

#include <KDebug>
#include "fifteen.h"

Piece::Piece(int id, Fifteen* parent, Plasma::Svg* svg)
    : QGraphicsWidget(parent)
{
  m_id = id;
  m_numeral = true;
  m_splitPixmap = false;
  m_svg = svg;
  m_fifteen = parent;
  m_bg = new QGraphicsRectItem(this);
  setCacheMode(DeviceCoordinateCache);
}

int Piece::id() const
{
  return m_id;
}

void Piece::setSplitImage(bool splitPixmap)
{
  m_splitPixmap = splitPixmap;
}

void Piece::setPartialPixmap(QPixmap pixmap)
{
  m_partialPixmap.setPixmap(pixmap);
  m_splitPixmap = true;
}

void Piece::setFont(const QFont &font)
{
  m_font = font;
}

void Piece::showNumeral(bool show)
{
  m_numeral = show;
}

void Piece::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
  if (m_splitPixmap) {
    m_partialPixmap.paint(painter, option, widget);
  } else {
    // here we assume that the svg has already been resized correctly by Fifteen::updatePixmaps()
    QColor c(m_fifteen->color());
    c.setAlphaF(0.5);
    painter->setBrush(c);
    painter->drawRect(boundingRect());
    m_svg->paint(painter, QPointF(0, 0));
  }

  if (!m_numeral) {
    return;
  }

  painter->setFont(m_font);

  QFontMetrics m(m_font);
  QString text = QString::number(m_id);

  QPen pen = painter->pen();

  pen.setColor(QColor(0, 0, 0, 90));
  painter->setPen(pen);
  painter->drawText((( size().width() / 2) - m.width(text) / 2) + 2,
                    (( size().height() / 2) + m.ascent() / 2) + 2, text);


  pen.setColor(QColor(Qt::white));
  painter->setPen(pen);
  painter->drawText(( size().width() / 2) - m.width(text) / 2,
                    ( size().height() / 2) + m.ascent() / 2, text);

}

void Piece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  Q_UNUSED(event);
  if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
  }

  event->accept();
  emit pressed(this);
}



