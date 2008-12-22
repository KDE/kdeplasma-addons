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

#include <QGraphicsWidget>

#include <Plasma/Svg>

#include "piece.h"

class Fifteen : public QGraphicsWidget
{
  Q_OBJECT
  public:
    Fifteen(QGraphicsItem *parent = 0);
    ~Fifteen();
    void updatePieces();

  public slots:
    void piecePressed(Piece *item);
    void setImage(const QString &path, bool identicalPieces);
    void setShowNumerals(bool show);
    void shuffle();

  protected:
    void resizeEvent(QGraphicsSceneResizeEvent * event);

  private:
    void drawPieces();
    Piece* itemAt(int gameX, int gameY);
    void swapPieceWithBlank(Piece *item);
    void clearPieces();
    bool isSolvable();

    QVector<Piece *> m_pieces;
    Piece *m_blank;
    Plasma::Svg *m_svg;
    QFont m_font;
    bool m_splitPixmap;
    bool m_numerals;
};

#endif
