// Copyright 2008-2010 by Benoît Jacob <jacob.benoit.1@gmail.com>
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
#include "render.h"

MandelbrotRenderThread::MandelbrotRenderThread(Mandelbrot *m) : m_mandelbrot(m)
{
    connect(this, SIGNAL(tileDone(MandelbrotTile)), m_mandelbrot, SLOT(tileDone(MandelbrotTile)));
}

void MandelbrotRenderThread::run()
{
    while(true)
    {
        MandelbrotTile tile;
        if(!m_mandelbrot->tiling().next(&tile)) return;
        QRect destination = tile.destination();

        // this is on purpose slightly larger than machine epsilon for float (gave artifacts near the transition point)
        const qreal double_precision_threshold = (qreal)4.0e-07;
        const qreal rendering_resolution = m_mandelbrot->resolution() / m_mandelbrot->supersampling();

#ifdef HAVE_PATH_WITH_SSE2_EXPLICTLY_ENABLED
        if(m_mandelbrot->hasSSE2())
        {
            if(rendering_resolution > double_precision_threshold) {
                with_SSE2_explicitly_enabled_if_x86::mandelbrot_render_tile<float>(m_mandelbrot, tile);
            }
            else {
                with_SSE2_explicitly_enabled_if_x86::mandelbrot_render_tile<double>(m_mandelbrot, tile);
            }
        }
        else
#endif
        {
            if(rendering_resolution > double_precision_threshold) {
                with_arch_defaults::mandelbrot_render_tile<float>(m_mandelbrot, tile);
            }
            else {
                with_arch_defaults::mandelbrot_render_tile<double>(m_mandelbrot, tile);
            }
        }

        // abort if required
        if(m_mandelbrot->abortRenderingAsSoonAsPossible()) return;

        // tell the world we've got a shiny new tile
        emit tileDone(tile);
    }
}

#include "renderthread.moc"
