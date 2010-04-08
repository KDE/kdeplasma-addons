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


#ifndef MANDELBROT_HEADER
#define MANDELBROT_HEADER

#include <QColor>
#include <QRect>
#include <QPainter>
#include <plasma/wallpaper.h>

#include "ui_config.h"

#include "global.h"
#include "tile.h"
#include "renderthread.h"

/** A Plasma Wallpaper that displays and allows to navigate through the Mandelbrot set
  *
  * The rendering is done in a separate MandelbrotRenderThread class. However the present Mandelbrot
  * class serves as a central repository of data that is shared between all threads. Whence all the
  * byzantine data members here. Definitely not very clean, but works, and this is just application code, right?
  */
class Mandelbrot : public Plasma::Wallpaper
{
    Q_OBJECT

#ifdef MANDELBROT_KEEP_KDE44_COMPATIBILITY
        inline bool isPreviewing() { return width()*height()<100000; }
#endif
        
    public:
        Mandelbrot(QObject* parent, const QVariantList& args);
        ~Mandelbrot();

        /** saves the config and updates the image cache. called by plasma. \see Plasma::Wallpaper*/
        virtual void save(KConfigGroup &config);
        /** \see Plasma::Wallpaper */
        virtual void paint(QPainter* painter, const QRectF& exposedRect);
        /** \see Plasma::Wallpaper */
        virtual QWidget* createConfigurationInterface(QWidget* parent);
        /**  \returns the width of the wallpaper in pixels */
        int width() const { return (int)boundingRect().width(); }
        /**  \returns the height of the wallpaper in pixels */
        int height() const { return (int)boundingRect().height(); }
        /** \returns the complex coordinate of the center of the viewpoint \see m_center */
        const QPointF& center() const { return m_center; }
        /** \returns the half-width of the view in the complex plane \see m_zoom */
        const qreal& zoom() const { return m_zoom; }
        /** \returns the image of the wallpaper \see m_image */
        QImage *image() { return m_image; }
        /** \returns the first color of the gradient, a.k.a. the inside color \see m_color1 */
        const QColor& color1() const { return m_color1; }
        /** \returns the second color of the gradient, a.k.a. the frontier color \see m_color2 */
        const QColor& color2() const { return m_color2; }
        /** \returns the third color of the gradient, a.k.a. the outside color \see m_color3 */
        const QColor& color3() const { return m_color3; }
        /** \returns the quality level as set by the KComboBox in the UI. \see m_quality */
        int quality() const { return m_quality; }
        /** \returns true is the view is locked (as set in the UI in the corresponding checkbox) \see m_lock */
        bool lock() const { return m_lock; }

        /** \returns a reference to the current tiling state being rendered \see m_tiling*/
        MandelbrotTiling& tiling() { return m_tiling; }
        /** \returns the number of rendering threads \see m_renderThreadCount */
        int renderThreadCount() const { return m_renderThreadCount; }
        /** \returns a pointer to the i-th rendering thread \see m_renderThreads*/
        MandelbrotRenderThread *renderThread(int i) { return m_renderThreads[i]; }
        /** \returns the distance in the complex plane between two adjacent pixels. So a smaller value of resolution() means a
          * closer zoom. \see zoom(), m_zoom
          */
        qreal resolution() const { return 2*zoom()/width(); }
        /** \returns a statistical estimation of the minimum count of iterations before divergence, as estimated from a few samples.
          * \note this functions only returns the precomputed result. You need to ask explicitly for it to be computed beforehand,
          * by calling computeStats() */
        int min_iter_divergence() const { return m_min_iter_divergence; }
        /** \returns true if the host CPU has SSE2 instructions */
        bool hasSSE2() const { return m_hasSSE2; }
        /** \returns true if rendering should be aborted as soon as possible */
        bool abortRenderingAsSoonAsPossible() const { return m_abortRenderingAsSoonAsPossible; }
        /** \returns the maximum number of iterations to try before declaring non-divergence */
        int maxIter() const;
        /** \returns 1 for no supersampling, 2 for 4x (2x2) supersampling, 4 for 16x (4x4) supersampling, etc... */
        int supersampling() const;
        /** \returns a key based on a hash of all the parameters that influence rendering. So if keys agree, the resulting images
          * can safely be assumed to also agree. */
        QString key() const;
        /** computes the stats, that is currently m_min_iter_before_divergence. \see min_iter_divergence() */ 
        void computeStats();
        /** sets the value of  m_imageIsReady. \see m_imageIsReady */
        void setImageIsReady(bool b) { m_imageIsReady = b; }

    signals:
        /** Signals that the configuration has changed */
        void settingsChanged(bool);

    public slots:
        /** To be called whenever the given tile is done rendering */
        void tileDone(const MandelbrotTile& t);

        void exportImage();
        void exportConfig();
        void importConfig();

    protected:
        virtual void init(const KConfigGroup &config);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

        /** (re-)starts rendering.
          * \param renderFirst pixel in the image that the user is most interested in. The nearest tiles will be rendered first.
          */
        void startRendering(const QPointF& renderFirst);
        /** convenience overloaded function, starts from the middle of the image. For restarting, call abortRendering() beforehand. */
        void startRendering() { startRendering(QPointF(width()/2,height()/2)); }
        /** self-explanatory. For restarting, call abortRendering() beforehand. */
        void loadFromCacheOrStartRendering();
        /** self-explanatory */
        void abortRendering();

        /** \param delta the translation, in pixels */
        void translateView(const QPointF& delta);
        /** \param at the pixel to zoom to/from
          * \param zoomFactor smaller than 1 for zoom in, bigger than 1 for zoom out */
        void zoomView(const QPointF& at, qreal zoomFactor);

        /** removes the current cache entry if it is obsolete, and inserts the new image into cache if it is ready */
        void updateCache();

        enum { ReadViewpoint=0x1, ReadLockStatus=0x2 };
        void readConfig(const KConfigGroup &config, int options);

    protected slots:
        void setColor1(const QColor& color1);
        void setColor2(const QColor& color2);
        void setColor3(const QColor& color3);
        void setQuality(int quality);
        void setLock(int lock);
        void checkRenderHints();

    private:
        Ui::Config m_ui;
        QColor m_color1, m_color2, m_color3;
        int m_quality;
        Qt::CheckState m_lock;
        QImage *m_image;
        MandelbrotTiling m_tiling;
        QPointF m_center;
        qreal m_zoom;
        MandelbrotRenderThread **m_renderThreads;
        int m_renderThreadCount;
        QPointF m_mousePressPos, m_mouseLastMovePos;
        Qt::MouseButtons m_mousePressedButtons;
        int m_min_iter_divergence;
        QString m_cacheKey;
        int m_tilesFinishedRendering;
        QAction m_exportImageAction;
        QAction m_exportConfigAction;
        QAction m_importConfigAction;
        bool m_abortRenderingAsSoonAsPossible : 1;
        bool m_hasSSE2 : 1;
        bool m_imageIsReady : 1;
        bool m_firstInit : 1;
};

#endif
