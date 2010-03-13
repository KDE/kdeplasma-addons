// Copyright 2008-2009 by Benoît Jacob <jacob.benoit.1@gmail.com>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License or (at your option) version 3 or any later version
// accepted by the membership of KDE e.V. (or its successor approved
// by the membership of KDE e.V.), which shall act as a proxy 
// defined in Section 14 of version 3 of the license.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef TILE_HEADER
#define TILE_HEADER

#include <QPoint>
#include <QRect>
#include <QMutex>

#include "global.h"

/** This class represents an abstract tile, that is, just an element of a 8x8 checkerboard.
  * This class is completely unaware of the pixel and complex coordinates of the tile,
  * it only stores a pointer to a Mandelbrot wallpaper
  * and has a method destination() computing the QRect in pixels where the tile belongs.
  */
class MandelbrotTile
{
        int m_x, m_y;
        Mandelbrot *m_mandelbrot;

    public:
        /** Initializes the tile at given (x,y) location in the checkerboard*/
        void set(Mandelbrot *mandelbrot, int x, int y) { m_mandelbrot = mandelbrot; m_x = x; m_y = y; }
        MandelbrotTile(Mandelbrot *mandelbrot) : m_mandelbrot(mandelbrot) {}
        MandelbrotTile() {} // needed by qRegisterMetaType
        ~MandelbrotTile() {}
        /** \returns the x-coordinate of the tile in the checkerboard */
        int x() const { return m_x; }
        /** \returns the y-coordinate of the tile in the checkerboard */
        int y() const { return m_y; }
        /** \returns the QRect in pixels where the tile belongs in the destination image */
        QRect destination() const;
        /** \returns the complex coordinate of the top-left corner of the tile */
        QPointF affix() const;
};

Q_DECLARE_METATYPE(MandelbrotTile)

/** This class remembers, in the 8x8 checkerboard, which tiles are already rendered, and uses it in the next()
  * method to determine which tile to render next.
  */
class MandelbrotTiling
{
        Mandelbrot *m_mandelbrot;
        int m_number;
        int m_board[TILING_SIZE][TILING_SIZE];
        QPoint m_renderFirst;
        QMutex m_mutex;

    public:
        /** Initializes the tile at given (x,y) location in the checkerboard*/
        MandelbrotTiling(Mandelbrot* m) : m_mandelbrot(m) {}
        /** finds the next not-yet-rendered tile to render. \returns true if there
          * is indeed such a next tile to render; false if the rendering is complete. */
        bool next(MandelbrotTile *result);
        /** Resets the checkerboard. \param renderFirst the pixel coordinate of the
          * pixel the user is most interested in. Tiles will be prioritized according to distance
          * from that pixel. */
        void start(const QPointF& renderFirst);
};

#endif
