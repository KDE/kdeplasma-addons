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

#include <QFont>
#include <QGraphicsWidget>
#include <QGraphicsPixmapItem>

#include <Plasma/Svg>

class Fifteen;

class Piece : public QGraphicsWidget
{
  Q_OBJECT
  public:
    Piece(int id, Fifteen * parent, Plasma::Svg *svg);
    int id() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void showNumeral(bool show);
    void setPartialPixmap(QPixmap pixmap);
    void setSplitImage(bool splitPixmap);
    void setFont(const QFont &font);

  private:
    int m_id;
    bool m_numeral;
    bool m_splitPixmap;
    QGraphicsPixmapItem m_partialPixmap;
    QFont m_font;
    Plasma::Svg *m_svg;
    QGraphicsRectItem *m_bg;
    Fifteen *m_fifteen;

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

  signals:
    void pressed(Piece *item);
};

#endif
