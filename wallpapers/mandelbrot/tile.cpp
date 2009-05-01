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


#include "mandelbrot.h"

bool MandelbrotTile::next()
{
    int m_square_horiz_dist[8], m_square_vert_dist[8];
    for(int i = 0; i < 8; i++) {
        int x = m_renderFirst.x() - int((1./16. + i/8.) * m_mandelbrot->width());
        m_square_horiz_dist[i] = x*x;
        int y = m_renderFirst.y() - int((1./16. + i/8.) * m_mandelbrot->height());
        m_square_vert_dist[i] = y*y;
    }

    int min_square_distance = INT_MAX;
    for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++) {
        if(m_board[i][j] == 0) {
            int square_distance = m_square_horiz_dist[i] + m_square_vert_dist[j];
            if(square_distance<min_square_distance) {
                m_x = i;
                m_y = j;
                min_square_distance = square_distance;
            }
        }
    }

    m_board[m_x][m_y] = 1;
    m_number++;

    return m_number < 64;
}

void MandelbrotTile::start(const QPointF& renderFirst)
{
    m_number = 0;
    m_renderFirst = QPoint((int)renderFirst.x(), (int)renderFirst.y());
    for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++) m_board[i][j] = 0;
    m_x = 8 * renderFirst.x() / m_mandelbrot->width();
    m_y = 8 * renderFirst.y() / m_mandelbrot->height();
    if(m_x < 0) m_x = 0;
    if(m_y < 0) m_y = 0;
    if(m_x > 7) m_x = 7;
    if(m_y > 7) m_y = 7;
    m_board[m_x][m_y] = 1;
}

QPointF MandelbrotTile::affix() const
{
    return QPointF(m_mandelbrot->center().x() + (-1+qreal(m_x)/4) * m_mandelbrot->zoom(),
                   m_mandelbrot->center().y() + (-1+qreal(m_y)/4) * m_mandelbrot->zoom()
                                                * m_mandelbrot->height() / m_mandelbrot->width());
}

QRect MandelbrotTile::destination() const
{
    int left = m_x*m_mandelbrot->width()/8;
    int top = m_y*m_mandelbrot->height()/8;
    int next_left = (m_x+1)*m_mandelbrot->width()/8;
    int next_top = (m_y+1)*m_mandelbrot->height()/8;
    return QRect(left, top, next_left-left, next_top-top);
}
