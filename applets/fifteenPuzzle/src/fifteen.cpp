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

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetrics>
#include <QTime>

#include <KDebug>
#include <KGlobalSettings>

#include <Plasma/Animation>
#include <Plasma/Animator>

Fifteen::Fifteen(QGraphicsItem* parent, int size)
    : QGraphicsWidget(parent),
      m_size(0), // this will immediately get overwritten by setSize(size) below, but needs an initial value
      m_pixmap(0)
{
  m_pieces.resize(size*size);
  m_splitPixmap = false;
  m_numerals = true;
  m_solved = true;
  m_svg = new Plasma::Svg();
  setSize(size);
  setCacheMode(DeviceCoordinateCache);
}

Fifteen::~Fifteen()
{
  qDeleteAll(m_pieces);
  delete m_svg;
}


void Fifteen::setSize(int size)
{
    if (m_size == size) {
        return;
    }
    m_size = qMax(size, 1);
    startBoard();
    setPreferredSize(48 * size, 48 * size);
    setMinimumSize(24 * size, 24 * size);
}

void Fifteen::setColor(const QColor& c)
{
    m_color = c;
    updatePieces();
}

int Fifteen::size() const
{
  return m_size;
}

const QColor& Fifteen:: color() const
{
  return m_color;
}

void Fifteen::setPixmap(QPixmap *pixmap)
{
  m_pixmap = pixmap;
  if (m_pixmap) {
    updatePieces();
    updatePixmaps();
  }
}

void Fifteen::updatePixmaps()
{
    if (!m_pixmap) {
        return;
    }
    QSize size = m_pieces[0]->size().toSize() * m_size;
    QPixmap copyPixmap = m_pixmap->scaled(size);
 
    for (int i = 0; i < m_size * m_size; i++) {
        if (!m_pieces[i]) continue;
        QRect rect = m_pieces[i]->boundingRect().toRect();
        // use the index of this piece in the solved puzzle to decide which pixmap
        // tile to use; the index in the array (i) is just its current position
        int index = m_pieces[i]->id() - 1;
        int posX = (index % m_size) * rect.width();
        int posY = (index / m_size) * rect.height();
      
        m_pieces[i]->setPartialPixmap(copyPixmap.copy(posX, posY, rect.width(), rect.height()));
    }
}

void Fifteen::startBoard()
{
  // abort the old puzzle if necessary (this resets the puzzle's timer)
  if (!m_solved) {
      emit aborted();
  }
  qDeleteAll(m_pieces);
  m_pieces.fill(NULL);
  int d = m_size * m_size;
  m_pieces.resize(d);
  for (int i = 0; i < d; ++i) {
      m_pieces[i] = new Piece(i+1, this, m_svg);
      if (i == d - 1) {
          m_blank = m_pieces[i];
      } else {
          connect(m_pieces[i], SIGNAL(pressed(Piece*)), this, SLOT(piecePressed(Piece*)));
      }
  }
  m_solved = true;
  updatePieces();
  updatePixmaps();
}

void Fifteen::shuffle()
{
    // shuffle the array of pieces
    qsrand(QTime::currentTime().msec());
    for (int i = m_size * m_size - 1; i > 0; i--) {
        // choose a random number such that 0 <= rand <= i
        int rand = qrand() % (i + 1);
        qSwap(m_pieces[i], m_pieces[rand]);
    }

    // make sure the new board is solveable
    
    // count the number of inversions
    // an inversion is a pair of tiles at positions a, b where
    // a < b but value(a) > value(b)

    // also count the number of lines the blank tile is from the bottom
    int inversions = 0;
    int blankRow = -1;
    for (int i = 0; i < m_size * m_size; i++) {
        if (m_pieces[i] == m_blank) {
            blankRow = i / m_size;
            continue;
        }
        for (int j = 0; j < i; j++) {
            if (m_pieces[j] == m_blank) {
                continue;
            }
            if (m_pieces[i]->id() < m_pieces[j]->id()) {
                inversions++;
            }
        }
    }

    if (blankRow == -1) {
        kDebug() << "Unable to find row of blank tile";
    }

    // we have a solveable board if:
    // size is odd:  there are an even number of inversions
    // size is even: the number of inversions is odd if and only if
    //               the blank tile is on an odd row from the bottom
    bool solveable = (m_size % 2 == 1 && inversions % 2 == 0) ||
                     (m_size % 2 == 0 && (inversions % 2 == 0) == ((m_size - blankRow) % 2 == 1));
    if (!solveable) {
        // make the grid solveable by swapping two adjacent pieces around
        int pieceA = 0;
        int pieceB = 1;
        if (m_pieces[pieceA] == m_blank) {
            pieceA = m_size + 1;
        } else if (m_pieces[pieceB] == m_blank) {
            pieceB = m_size;
        }
        qSwap(m_pieces[pieceA], m_pieces[pieceB]);
    }

    // move the pieces to their new locations
    for (int i = 0; i < m_size * m_size; i++) {
        if (m_pieces[i] == m_blank) {
            // don't animate the blank piece
            int width = contentsRect().width() / m_size;
            int height = contentsRect().height() / m_size;
            m_pieces[i]->setPos((i % m_size) * width, (i / m_size) * height);
        } else {
            movePiece(m_pieces[i], i % m_size, i / m_size);
        }
    }

    // assume the shuffle didn't result in a sorted board
    // in the unlikely event that it did, the user just has to move a tile
    // and put it back again to solve it
    m_solved = false;
    toggleBlank(false);
    emit started();
}

void Fifteen::resizeEvent(QGraphicsSceneResizeEvent *event)
{
  Q_UNUSED(event);

  updatePieces();
  updatePixmaps();
}

void Fifteen::updateFont()
{
  int width = contentsRect().width() / m_size;
  int height = contentsRect().height() / m_size;

  QString test = "99";
  int smallest = KGlobalSettings::smallestReadableFont().pixelSize();
  int fontSize = height / 3;
  QFont f = font();
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
}

void Fifteen::setShowNumerals(bool show)
{
  m_numerals = show;
  updatePieces();
}

void Fifteen::setSvg(const QString &path, bool identicalPieces)
{
  m_svg->setImagePath(path);
  m_numerals = m_numerals || identicalPieces;
  updatePieces();
}

void Fifteen::updatePieces()
{
    updateFont();

    QSizeF size = contentsRect().size();
    int width = size.width() / m_size;
    int height = size.height() / m_size;

    for (int i = 0; i < m_size * m_size; ++i) {
        m_pieces[i]->showNumeral(m_numerals);
        m_pieces[i]->setSplitImage(m_pixmap != NULL);
        m_pieces[i]->resize(QSizeF(width, height));
        m_pieces[i]->setPos((i % m_size) * width, (i / m_size) * height);
        m_pieces[i]->setFont(m_font);
        m_pieces[i]->update();
    }
  
    if (!m_pixmap) {
        m_svg->resize(width, height);
    }
}

void Fifteen::checkSolved()
{
    bool sorted = true;
    for (int i = 0; i < m_size * m_size; i++) {
        if (m_pieces[i]->id() != i + 1) {
            sorted = false;
            break;
        }
    }

    if (m_solved && !sorted) {
        // the board has already been solved, so the user is playing with tiles on a solved board
        toggleBlank(false);
    } else if (m_solved && sorted) {
        // the user has finished playing with tiles on a solved board, and it is solved again
        toggleBlank(true);
    } else if (!m_solved && sorted) {
        emit solved();
        m_solved = true;
        toggleBlank(true);
    }
}

void Fifteen::piecePressed(Piece *item)
{
    int itemX = -1, itemY = -1, blankX = -1, blankY = -1;
    for (int i = 0; i < m_size * m_size; i++) {
        if (item == m_pieces[i]) {
            itemX = i % m_size;
            itemY = i / m_size;
        }
        if (m_blank == m_pieces[i]) {
            blankX = i % m_size;
            blankY = i / m_size;
        }
    }

    if (itemX == -1 || itemY == -1 || blankX == -1 || blankY == -1) {
        kDebug() << "Missing piece!";
        return;
    }

    if (blankX == itemX && blankY != itemY) {
        while (blankY < itemY) {
            swapPieceWithBlank(itemX, blankY + 1, blankX, blankY);
            blankY++;
        }
        while (blankY > itemY) {
            swapPieceWithBlank(itemX, blankY - 1, blankX, blankY);
            blankY--;
        }
    } else if (blankY == itemY && blankX != itemX) {
        while (blankX < itemX) {
            swapPieceWithBlank(blankX + 1, itemY, blankX, blankY);
            blankX++;
        }
        while (blankX > itemX) {
            swapPieceWithBlank(blankX - 1, itemY, blankX, blankY);
            blankX--;
        }
    }
    
    checkSolved();
}

void Fifteen::swapPieceWithBlank(int pieceX, int pieceY, int blankX, int blankY)
{
    Piece *piece = m_pieces[(pieceY * m_size) + pieceX];

    int width = contentsRect().width() / m_size;
    int height = contentsRect().height() / m_size;
    QPointF pos = QPointF(pieceX * width, pieceY * height);

    // swap widget positions
    movePiece(piece, blankX, blankY);
    m_blank->setPos(pos);

    // swap game positions
    qSwap(m_pieces[(pieceY * m_size) + pieceX],
          m_pieces[(blankY * m_size) + blankX]);
}

void Fifteen::movePiece(Piece *piece, int newX, int newY)
{
    int width = contentsRect().width() / m_size;
    int height = contentsRect().height() / m_size;
    QPointF pos = QPointF(newX * width, newY * height);

    // stop and delete any existing animation
    Plasma::Animation *animation = m_animations.value(piece).data();
    if (animation) {
        if (animation->state() == QAbstractAnimation::Running) {
            animation->stop();
        }
        delete animation;
        animation = NULL;
    }
    animation = Plasma::Animator::create(Plasma::Animator::SlideAnimation, this);
    animation->setTargetWidget(piece);
    animation->setProperty("easingCurve", QEasingCurve::InOutQuad);
    animation->setProperty("movementDirection", Plasma::Animation::MoveAny);
    animation->setProperty("distancePointF", pos - piece->pos());
    m_animations[piece] = animation;
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Fifteen::toggleBlank(bool show)
{
    if (show) {
        if (!m_blank->isVisible()) {
            Plasma::Animation *animation = Plasma::Animator::create(Plasma::Animator::FadeAnimation, this);
            animation->setProperty("startOpacity", 0.0);
            animation->setProperty("targetOpacity", 1.0);
            animation->setTargetWidget(m_blank);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
            m_blank->show();
        }
    } else {
        m_blank->hide();
    }
}

