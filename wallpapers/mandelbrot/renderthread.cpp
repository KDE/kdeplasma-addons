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
#include "render.h"
#include <QPainter>

MandelbrotRenderThread::MandelbrotRenderThread(Mandelbrot *m, int i) : m_mandelbrot(m), m_id(i)
{
    if(m_id == 0) {
        connect(this, SIGNAL(tileDone(const MandelbrotTile&)), m_mandelbrot, SLOT(tileDone(const MandelbrotTile&)));
    }
}

void MandelbrotRenderThread::run()
{
    QImage *tileImagePtr = 0;

    // if we're the master thread, start by computing the stats
    if(m_id == 0)
    {
        m_mandelbrot->computeStats();
        // abort if required
        if(m_mandelbrot->abortRenderingAsSoonAsPossible()) return;
    }

    while(true)
    {
        // tileImagePtr is the image to which we render a tile, before scaling it and merging it to the main image.
        // Notice that tileImagePtr is not really a "local variable", it is a copy of a pointer stored in the Mandelbrot class
        // so that all threads share it. Then of course it's important that only thread 0, the master thread,
        // creates and destroys *tileImagePtr.
        tileImagePtr = m_mandelbrot->tileImagePtr();

        MandelbrotTile tile = m_mandelbrot->tile();
        QRect destination = tile.destination();
        int supersampling = m_mandelbrot->supersampling();

        // if we're the master thread, fire the other threads
        if(m_id == 0)
        {
            QSize tileImageWantedSize = destination.size() * supersampling;
            if(tileImagePtr == 0 || tileImagePtr->size() != tileImageWantedSize)
            {
                delete tileImagePtr;
                tileImagePtr = new QImage(tileImageWantedSize, MANDELBROT_QIMAGE_FORMAT);
                m_mandelbrot->setTileImagePtr(tileImagePtr);
            }

            for(int i = 1; i < m_mandelbrot->renderThreadCount(); i++) {
                m_mandelbrot->renderThread(i)->start(QThread::LowPriority);
            }
        }
        
        const qreal float_epsilon = (qreal)1.192e-07;
        const qreal rendering_resolution = m_mandelbrot->resolution() / supersampling;

#ifdef HAVE_PATH_WITH_SSE2_EXPLICTLY_ENABLED
        if(m_mandelbrot->hasSSE2())
        {
            if(rendering_resolution > float_epsilon) {
                with_SSE2_explicitly_enabled_if_x86::mandelbrot_render_tile<float>(m_mandelbrot, m_id, tileImagePtr);
            }
            else {
                with_SSE2_explicitly_enabled_if_x86::mandelbrot_render_tile<double>(m_mandelbrot, m_id, tileImagePtr);
            }
        }
        else
#endif
        {
            if(rendering_resolution > float_epsilon) {
                with_arch_defaults::mandelbrot_render_tile<float>(m_mandelbrot, m_id, tileImagePtr);
            }
            else {
                with_arch_defaults::mandelbrot_render_tile<double>(m_mandelbrot, m_id, tileImagePtr);
            }
        }

        // Now this thread is done rendering its share of the current tile

        // if we're not the master thread, we're done (only the master thread paints on the main image and continues across tiles)
        if(m_id != 0) return;

        /****** from now on, we know we're the master thread ******/

        // abort if required
        if(m_mandelbrot->abortRenderingAsSoonAsPossible()) return;

        // wait for other threads
        for(int th = 1; th < m_mandelbrot->renderThreadCount(); th++)
          m_mandelbrot->renderThread(th)->wait();

        // abort if required
        if(m_mandelbrot->abortRenderingAsSoonAsPossible()) return;

        // now all threads are done rendering the current tile. Merge that tile into the main image.
        if(supersampling>1) {
            // need smooth scaling
            QPainter(m_mandelbrot->image())
              .drawImage(destination, tileImagePtr->scaled(destination.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            // no need for any scaling
            QPainter(m_mandelbrot->image()).drawImage(destination, *tileImagePtr);
        }

        // tell the world we've got a shiny new tile
        emit tileDone(tile);

        // abort if required
        if(m_mandelbrot->abortRenderingAsSoonAsPossible()) return;
        
        // proceed to the next tile, or exit -- rendering is then complete
        if(!m_mandelbrot->tile().next()) {
            m_mandelbrot->setImageIsReady(true);
            delete tileImagePtr;
            m_mandelbrot->setTileImagePtr(0);
            return;
        }
    }
}

#include "renderthread.moc"
