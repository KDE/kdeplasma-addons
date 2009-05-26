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

#include "fifteen.h"
#include "piece.h"

#include <ctime>
#include <QGraphicsScene>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include <KDebug>
#include <KGlobalSettings>

#include "plasma/animator.h"

Fifteen::Fifteen(QGraphicsItem *parent)
    : QGraphicsWidget(parent)
{
  m_pieces.resize(16);
  m_splitPixmap = false;
  m_numerals = true;

  m_svg = new Plasma::Svg();
  shuffle();
  setCacheMode(DeviceCoordinateCache);
}

Fifteen::~Fifteen()
{
  qDeleteAll(m_pieces);
  delete m_svg;
}

void Fifteen::clearPieces()
{
  for (int i = 0; i < 16; ++i)
    delete m_pieces[i];
}

void Fifteen::shuffle()
{
  qsrand(time(0));
  qDeleteAll(m_pieces);
  m_pieces.fill(NULL);
  int numPiecesLeft = 16;
  for (int i = 0; i < 16; ++i) {
    int randIndex = qrand() % numPiecesLeft;
    int rand = 0;
    --numPiecesLeft;

    // make sure we have an empty piece
    if (randIndex == 0 && m_pieces.at(0)) {
        ++randIndex;
    }

    for (int j = 0; j < randIndex; ++j) {
        ++rand;
        while (m_pieces.at(rand)) {
            ++rand;
        }
    }
    //kDebug() << "rand" << randIndex << rand;

    m_pieces[rand] = new Piece(i, this, m_svg, rand);
    QObject::connect(m_pieces[rand], SIGNAL(pressed(Piece*)), this, SLOT(piecePressed(Piece*)));

    if (i == 0) {
      m_blank = m_pieces[rand];
    }
  }

  if (!isSolvable()) {
    // this is where I would rip out two pieces and swap them around, if it were a plastic puzzle
    int a = 1;
    int b = 2;
    if (m_pieces[a] == m_blank) { // you can't pull out a hole!
      a = 3;
    } else if (m_pieces[b] == m_blank) {
      b = 0;
    }
    qSwap(m_pieces[a], m_pieces[b]);

    // also swap the gamePos of the pieces
    int aPos = m_pieces[a]->getGamePos();
    m_pieces[a]->setGamePos(m_pieces[b]->getGamePos());
    m_pieces[b]->setGamePos(aPos);
  }

  updatePieces();
}

void Fifteen::resizeEvent(QGraphicsSceneResizeEvent *event)
{
  Q_UNUSED(event);

  QSizeF size = contentsRect().size();
  int width = size.width() / 4;
  int height = size.height() / 4;

  QString test = "99";
  QFont f = font();
  int smallest = KGlobalSettings::smallestReadableFont().pixelSize();
  int fontSize = 14;
  f.setBold(true);
  f.setPixelSize(fontSize);
  
  QFontMetrics fm(f);
  QRect rect = fm.boundingRect(test);
  while (rect.width() > width - 2 || rect.height() > height - 2) {
    --fontSize;
    f.setPixelSize(fontSize);

    if (fontSize <= smallest) {
        f = KGlobalSettings::smallestReadableFont();
        break;
    }

    fm = QFontMetrics(f);
    rect = fm.boundingRect(test);
  }

  m_font = f;

  updatePieces();
}

bool Fifteen::isSolvable()
{
  int fields[16];
  bool odd_even_solvable=0;
  for (int i = 0;  i < 16; ++i) {
    fields[i] = m_pieces[i]->getId();
    if (fields[i] == 0) {
      fields[i] = 16;
      switch (i) {
        case 0: case  2: case  5: case  7:
        case 8: case 10: case 13: case 15: odd_even_solvable = 1; break;
        case 1: case  3: case  4: case  6:
        case 9: case 11: case 12: case 14: odd_even_solvable = 0; break;
      }
    }
  }

  bool odd_even_permutations = 1;
  for (int i = 0; i < 16; ++i) {
    int field = fields[i];
    while (field != i + 1) {
      int temp_field = fields[field - 1];
      fields[field - 1] = field;
      field = temp_field;
      odd_even_permutations = !odd_even_permutations;
    }
  }

  return odd_even_solvable == odd_even_permutations;
}

void Fifteen::setShowNumerals(bool show)
{
  m_numerals = show;
  updatePieces();
}

void Fifteen::setImage(const QString &path, bool identicalPieces)
{
  m_svg->setImagePath(path);
  m_splitPixmap = !identicalPieces && m_svg->hasElement("piece_0");
  m_numerals = m_numerals || identicalPieces;
  updatePieces();
}

void Fifteen::updatePieces()
{
  QSizeF size = contentsRect().size();
  int width = size.width() / 4;
  int height = size.height() / 4;

  if (m_splitPixmap) {
    m_svg->resize(size);
  } else {
    m_svg->resize(width, height);
  }

  for (int i = 0; i < 16; ++i) {
    m_pieces[i]->showNumeral(m_numerals);
    m_pieces[i]->setSplitImage(m_splitPixmap);
    m_pieces[i]->setSize(QSizeF(width, height));
    m_pieces[i]->setPos(m_pieces[i]->getGameX() * width, m_pieces[i]->getGameY() * height);
    m_pieces[i]->setFont(m_font);
  }
}

void Fifteen::piecePressed(Piece *item)
{
  int ix = item->getGameX();
  int iy = item->getGameY();
  int bx = m_blank->getGameX();
  int by = m_blank->getGameY();

  if (ix == bx && iy != by) {
    if (iy > by) {
      for (; by < iy; by++) {
        // swap the piece at ix,by+1 with blank
        swapPieceWithBlank(itemAt(ix, by + 1));
      }
    }
    else if (iy < by) {
      for (; by > iy; by--) {
        // swap the piece at ix,by-1 with blank
        swapPieceWithBlank(itemAt(ix, by - 1));
      }
    }
  }
  else if (iy == by && ix != bx) {
    if (ix > bx) {
      for (; bx < ix; bx++) {
        // swap the piece at bx+1,iy with blank
        swapPieceWithBlank(itemAt(bx + 1, iy));
      }
    }
    else if (ix < bx) {
      for (; bx > ix; bx--) {
        // swap the piece at bx-1,iy with blank
        swapPieceWithBlank(itemAt(bx - 1, iy));
      }
    }
  }
}

Piece* Fifteen::itemAt(int gameX, int gameY)
{
  int gamePos = (gameY * 4) + gameX;
  for (int i = 0; i < 16; i++) {
    if (m_pieces[i]->getGamePos() == gamePos) {
      return m_pieces[i];
    }
  }
  return NULL;
}

void Fifteen::swapPieceWithBlank(Piece *item)
{
  int width = contentsRect().size().width() / 4;
  int height = contentsRect().size().height() / 4;

  // swap widget positions
  QPointF pos = QPointF(item->getGameX() * width, item->getGameY() * height);
  Plasma::Animator::self()->moveItem(item, Plasma::Animator::FastSlideInMovement, m_blank->pos().toPoint());
  m_blank->setPos(pos);

  // swap game positions
  int blankPos = m_blank->getGamePos();
  m_blank->setGamePos(item->getGamePos());
  item->setGamePos(blankPos);
}

