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
#include <QGraphicsItem>
#include <QObject>

#include <Plasma/Svg>

class Piece : public QObject, public QGraphicsItem
{
  Q_OBJECT

  public:
    Piece(int id, QGraphicsItem * parent, Plasma::Svg *svg, int gamePos);
    int getId();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;
    void showNumeral(bool show);
    int getGameX();
    int getGameY();
    int getGamePos();
    void setGamePos(int gamePos);
    void setSize(QSizeF size);
    void setSplitImage(bool splitPixmap);
    void setFont(const QFont &font);

  private:
    int m_id;
    bool m_numeral;
    int m_gamePos;
    bool m_splitPixmap;
    QSizeF m_size;
    QFont m_font;
    Plasma::Svg *m_svg;

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

  signals:
    void pressed(Piece *item);
};

#endif
