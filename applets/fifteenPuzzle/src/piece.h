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

#ifndef PIECE_H
#define PIECE_H

#include <QGraphicsItem>
#include <QObject>

class Piece : public QObject, public QGraphicsPixmapItem
{  
Q_OBJECT

public:
	Piece(int size, int id, QGraphicsItem * parent);
  int getId();
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  void showNumeral(bool show);

private:
  int m_id;
  int m_size;
  bool m_numeral;
 
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);  

signals:
  void pressed(QGraphicsItem *item);
};
#endif
