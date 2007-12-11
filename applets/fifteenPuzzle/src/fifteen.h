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

#ifndef FIFTEEN_H
#define FIFTEEN_H

#include <QGraphicsRectItem>
#include <QObject>

#include "piece.h"

class Fifteen : public QObject, public QGraphicsRectItem
{
Q_OBJECT

public:
	Fifteen(QGraphicsItem *parent = 0 );
  
public slots:
  void piecePressed(QGraphicsItem *item);
  void setSplitPixmap(QString path);
  void setIdentical();
  void setNumerals(bool show);
  void shuffle();

private:
  void drawPieces();
  bool isAdjacent(QGraphicsItem *a, QGraphicsItem *b);
  void updatePixmaps();
  void clearPieces();
  void updateNumerals();
  bool isSolvable();

  QVector<Piece *> m_pieces;
  QVector<QPixmap> m_pixmaps;
  QGraphicsItem *m_blank;
  bool m_splitPixmap;
  QPixmap m_pixmap;
  bool m_numerals;
};
#endif
