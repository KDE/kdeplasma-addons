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

Piece::Piece(int id, QGraphicsItem *parent, Plasma::Svg *svg, int gamePos)
    : QGraphicsItem(parent)
{
  m_id = id;
  m_numeral = true;
  m_gamePos = gamePos;
  m_svg = svg;
  setCacheMode(DeviceCoordinateCache);
}

int Piece::getId()
{
  return m_id;
}

int Piece::getGameX()
{
  return m_gamePos % 4;
}

int Piece::getGameY()
{
  return m_gamePos / 4;
}

int Piece::getGamePos()
{
  return m_gamePos;
}

void Piece::setGamePos(int gamePos)
{
  m_gamePos = gamePos;
}

void Piece::setSize(QSizeF size)
{
  prepareGeometryChange();
  m_size = size;
}

void Piece::setSplitImage(bool splitPixmap)
{
  m_splitPixmap = splitPixmap;
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
  Q_UNUSED(option);
  Q_UNUSED(widget);

  if (m_id == 0) {
    return;
  }

  if (m_splitPixmap) {
    m_svg->paint(painter, QPointF(0, 0), "piece_" + QString::number(m_id));
  } else {
    // here we assume that the svg has already been resized correctly by Fifteen::updatePixmaps()
    m_svg->paint(painter, QPointF(0, 0));
  }

  if (!m_numeral) {
    return;
  }

  int width = m_size.width();
  int height = m_size.height();

  painter->setFont(m_font);

  QFontMetrics m(m_font);
  QString text = QString::number(m_id);

  QPen pen = painter->pen();

  pen.setColor(QColor(0, 0, 0, 90));
  painter->setPen(pen);
  painter->drawText(((width / 2) - m.width(text) / 2) + 2,
                    ((height / 2) + m.ascent() / 2) + 2,
                    text);

  pen.setColor(QColor(Qt::white));
  painter->setPen(pen);
  painter->drawText((width / 2) - m.width(text) / 2,
                    (height / 2) + m.ascent() / 2,
                    text);
}

QRectF Piece::boundingRect() const
{
  return QRectF(QPointF(0, 0), m_size);
}

void Piece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  Q_UNUSED(event);
  if (m_id == 0 || event->button() != Qt::LeftButton) {
      event->ignore();
      return;
  }

  event->accept();
  emit pressed(this);
}



