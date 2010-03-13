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

#include <QMutexLocker>

#include "mandelbrot.h"

bool MandelbrotTiling::next(MandelbrotTile *result)
{
    QMutexLocker locker(&m_mutex);
    int m_square_horiz_dist[TILING_SIZE], m_square_vert_dist[TILING_SIZE];
    for(int i = 0; i < TILING_SIZE; i++) {
        int x = m_renderFirst.x() - int((1/(2*double(TILING_SIZE)) + i/double(TILING_SIZE)) * m_mandelbrot->width());
        m_square_horiz_dist[i] = x*x;
        int y = m_renderFirst.y() - int((1/(2*double(TILING_SIZE)) + i/double(TILING_SIZE)) * m_mandelbrot->height());
        m_square_vert_dist[i] = y*y;
    }

    int min_square_distance = INT_MAX;
    int result_x = 0, result_y = 0;
    for(int i = 0; i < TILING_SIZE; i++) for(int j = 0; j < TILING_SIZE; j++) {
        if(m_board[i][j] == 0) {
            int square_distance = m_square_horiz_dist[i] + m_square_vert_dist[j];
            if(square_distance<min_square_distance) {
                result_x = i;
                result_y = j;
                min_square_distance = square_distance;
            }
        }
    }

    result->set(m_mandelbrot, result_x, result_y);
    m_board[result->x()][result->y()] = 1;
    m_number++;

    return m_number <= TILING_SIZE*TILING_SIZE;
}

void MandelbrotTiling::start(const QPointF& renderFirst)
{
    m_number = 0;
    m_renderFirst = QPoint((int)renderFirst.x(), (int)renderFirst.y());
    for(int i = 0; i < TILING_SIZE; i++) for(int j = 0; j < TILING_SIZE; j++) m_board[i][j] = 0;
}

QPointF MandelbrotTile::affix() const
{
    return QPointF(m_mandelbrot->center().x() + (-1+2*qreal(m_x)/TILING_SIZE) * m_mandelbrot->zoom(),
                   m_mandelbrot->center().y() + (-1+2*qreal(m_y)/TILING_SIZE) * m_mandelbrot->zoom()
                                                * m_mandelbrot->height() / m_mandelbrot->width());
}

QRect MandelbrotTile::destination() const
{
    int left = m_x*m_mandelbrot->width()/TILING_SIZE;
    int top = m_y*m_mandelbrot->height()/TILING_SIZE;
    int next_left = (m_x+1)*m_mandelbrot->width()/TILING_SIZE;
    int next_top = (m_y+1)*m_mandelbrot->height()/TILING_SIZE;
    return QRect(left, top, next_left-left, next_top-top);
}
