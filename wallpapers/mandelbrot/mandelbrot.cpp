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
#include <iostream>

#include <cmath>
#include <QGraphicsSceneMouseEvent>
#include <kfiledialog.h>
#include <QByteArray>
#include <QBuffer>
#include <kio/job.h>
//#include <KDebug>

const qreal MOUSE_MMB_SPEED = (qreal)10.;
const qreal MOUSE_WHEEL_SPEED = (qreal)0.002;

K_EXPORT_PLASMA_WALLPAPER(mandelbrot, Mandelbrot)

Mandelbrot::Mandelbrot(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args), m_image(0), m_tiling(this),
      m_exportImageAction(i18n("Export Mandelbrot image..."), this),
      m_exportConfigAction(i18n("Export Mandelbrot parameters..."), this),
      m_importConfigAction(i18n("Import Mandelbrot parameters..."), this),
      m_abortRenderingAsSoonAsPossible(false),
      m_imageIsReady(false),
      m_firstInit(true)
{
    setPreviewDuringConfiguration(true);
    qRegisterMetaType<MandelbrotTile>();
    m_hasSSE2 = system_has_SSE2();
    m_renderThreadCount = QThread::idealThreadCount();
    m_renderThreads = new MandelbrotRenderThread*[m_renderThreadCount];
    for(int th = 0; th < m_renderThreadCount; th++) m_renderThreads[th] = new MandelbrotRenderThread(this);
    setUsingRenderingCache(true);

    QList<QAction*> actionsList;
    actionsList.append(&m_exportImageAction);
    actionsList.append(&m_exportConfigAction);
    actionsList.append(&m_importConfigAction);
    setContextualActions(actionsList);

    connect(this, SIGNAL(renderHintsChanged()), this, SLOT(checkRenderHints()));
    connect(&m_exportImageAction, SIGNAL(triggered()), this, SLOT(exportImage()));
    connect(&m_exportConfigAction, SIGNAL(triggered()), this, SLOT(exportConfig()));
    connect(&m_importConfigAction, SIGNAL(triggered()), this, SLOT(importConfig()));
}

Mandelbrot::~Mandelbrot()
{
    abortRendering();
    // the user may have moved the viewpoint, so we must save the config on exit (and that takes care of the cache, too)
    emit(configNeedsSaving());
    for(int th = 0; th < m_renderThreadCount; th++) delete m_renderThreads[th];
    delete[] m_renderThreads;
    delete m_image;
}

void Mandelbrot::updateCache()
{
    // Don't let the mini-previewer in Desktop Settings affect the cache.
    if(isPreviewing()) return;

    QString k = key();

    // if the view parameters changed since we loaded from cache, or if we couldn't load from cache
    if(k != m_cacheKey)
    {
        // remove old image from cache
        //kDebug() << "remove " << m_cacheKey;
        insertIntoCache(m_cacheKey, QImage());

        // if the image is ready to be cached, cache it
        if(m_imageIsReady) {
            //kDebug() << "caching " << k << " replacing " << m_cacheKey;
            insertIntoCache(k, *m_image);
            m_cacheKey = k;
        }
    }
    //else kDebug() << k << " is already cached";
}

void Mandelbrot::paint(QPainter *painter, const QRectF& exposedRect)
{
    painter->drawImage(exposedRect, *m_image, exposedRect.translated(-boundingRect().topLeft()));
}

void Mandelbrot::save(KConfigGroup &config)
{
    if (!isPreviewing())
    {
        config.writeEntry(MANDELBROT_CENTER_KEY, m_center);
        config.writeEntry(MANDELBROT_ZOOM_KEY, m_zoom);
    }

    config.writeEntry(MANDELBROT_COLOR1_KEY, m_color1);
    config.writeEntry(MANDELBROT_COLOR2_KEY, m_color2);
    config.writeEntry(MANDELBROT_COLOR3_KEY, m_color3);
    config.writeEntry(MANDELBROT_QUALITY_KEY, m_quality);
    config.writeEntry(MANDELBROT_LOCK_KEY, int(m_lock));
    // and this is where we update the cache. It's important to update the cache at the same time we save the config.
    // Otherwise we could easily have stale cached images, or missing ones, for example in case of a crash.
    updateCache();
}

void Mandelbrot::readConfig(const KConfigGroup &config, int options)
{
    QString old_key = key();

    if(options & ReadViewpoint)
    {
        m_center = config.readEntry(MANDELBROT_CENTER_KEY, QPointF(qreal(-0.25),qreal(0)));
        m_zoom = config.readEntry(MANDELBROT_ZOOM_KEY, qreal(4));
    }

    m_color1 = config.readEntry(MANDELBROT_COLOR1_KEY, QColor(0,0,0));
    m_color2 = config.readEntry(MANDELBROT_COLOR2_KEY, QColor(255,255,255));
    m_color3 = config.readEntry(MANDELBROT_COLOR3_KEY, QColor(0,0,255));
    m_quality = qBound(0, config.readEntry(MANDELBROT_QUALITY_KEY, 1), 4);

    if(options & ReadLockStatus)
    {
      m_lock = Qt::CheckState(config.readEntry(MANDELBROT_LOCK_KEY, int(Qt::Unchecked)));
    }

    if(key() != old_key)
    {
        abortRendering();

        if(!m_image || m_image->size() != boundingRect().size())
        {
            delete m_image;
            m_image = new QImage(width(), height(), MANDELBROT_QIMAGE_FORMAT);
            QPainter(m_image).fillRect(m_image->rect(), Qt::black);
        }

        loadFromCacheOrStartRendering();
    }
}

void Mandelbrot::init(const KConfigGroup &config)
{
    readConfig(config, (m_firstInit?ReadViewpoint:0) | ReadLockStatus);
    m_firstInit = false;
}

QWidget* Mandelbrot::createConfigurationInterface(QWidget* parent)
{
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);

    m_ui.m_color1->setColor(m_color1);
    m_ui.m_color2->setColor(m_color2);
    m_ui.m_color3->setColor(m_color3);
    m_ui.m_quality->setCurrentIndex(m_quality);
    m_ui.m_lock->setCheckState(m_lock);
    connect(m_ui.m_color1, SIGNAL(changed(QColor)), this, SLOT(setColor1(QColor)));
    connect(m_ui.m_color2, SIGNAL(changed(QColor)), this, SLOT(setColor2(QColor)));
    connect(m_ui.m_color3, SIGNAL(changed(QColor)), this, SLOT(setColor3(QColor)));
    connect(m_ui.m_quality, SIGNAL(activated(int)), this, SLOT(setQuality(int)));
    connect(m_ui.m_lock, SIGNAL(stateChanged(int)), this, SLOT(setLock(int)));

    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));

    return widget;
}

void Mandelbrot::setColor1(const QColor& color1)
{
    abortRendering();
    m_color1 = color1;
    startRendering();
    emit settingsChanged(true);
}

void Mandelbrot::setColor2(const QColor& color2)
{
    abortRendering();
    m_color2 = color2;
    startRendering();
    emit settingsChanged(true);
}

void Mandelbrot::setColor3(const QColor& color3)
{
    abortRendering();
    m_color3 = color3;
    startRendering();
    emit settingsChanged(true);
}

void Mandelbrot::setQuality(int quality)
{
    abortRendering();
    m_quality = quality;
    startRendering();
    emit settingsChanged(true);
}

void Mandelbrot::setLock(int lock)
{
    m_lock = Qt::CheckState(lock);
    emit settingsChanged(true);
}

void Mandelbrot::checkRenderHints()
{
    if (m_image && m_image->size() != boundingRect().size()) {
        abortRendering();
        delete m_image;
        m_image = new QImage(width(), height(), MANDELBROT_QIMAGE_FORMAT);
        QPainter(m_image).fillRect(m_image->rect(), Qt::black);
        loadFromCacheOrStartRendering();
    }
}

void Mandelbrot::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
    if(m_lock) return;
    m_mousePressPos = m_mouseLastMovePos = event->pos();
    m_mousePressedButtons = event->buttons();
    if(event->buttons() & (Qt::LeftButton|Qt::MidButton)) event->accept();
}

void Mandelbrot::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
    if(m_lock) return;
    if(m_mousePressedButtons & (Qt::LeftButton|Qt::MidButton)) event->accept();
}

void Mandelbrot::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
    if(m_lock) return;
    if(event->buttons() & (Qt::LeftButton|Qt::MidButton)) event->accept();

    QPointF delta = event->pos() - m_mouseLastMovePos;
    m_mouseLastMovePos = event->pos();

    if(event->buttons() & Qt::MidButton)
    {
        zoomView(m_mousePressPos, std::exp(MOUSE_MMB_SPEED * qreal(delta.y()) / height()));
    }

    if(event->buttons() & Qt::LeftButton)
    {
        translateView(delta);
    }
}

void Mandelbrot::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    event->ignore();
    if(m_lock) return;
    event->accept();
    zoomView(event->pos(), std::exp(-MOUSE_WHEEL_SPEED * event->delta()));
}

void Mandelbrot::abortRendering()
{
    m_abortRenderingAsSoonAsPossible = true;
    for(int i = 0; i < m_renderThreadCount; i++) m_renderThreads[i]->wait();
    m_abortRenderingAsSoonAsPossible = false;
}

void Mandelbrot::loadFromCacheOrStartRendering()
{
    QString k = key();
    if(findInCache(k, *m_image))
    {
        if(m_image->size() == boundingRect().size()) {
            //kDebug() << "image " << k << " found in cache and has the wanted size";
            m_image->convertToFormat(MANDELBROT_QIMAGE_FORMAT);
            m_cacheKey = k;
            m_imageIsReady = true;
            update(m_image->rect());
        }
        else {
            //kDebug() << "image " << k << " found in cache but hasn't the wanted size. Removing it, and re-rendering.";
            insertIntoCache(k, QImage());
            startRendering();
        }
    }
    else
    {
        //kDebug() << "image " << k << " not found in cache";
        startRendering();
    }
}

void Mandelbrot::startRendering(const QPointF& renderFirst)
{
    abortRendering();
    if(m_image->size() != boundingRect().size()) {
        delete m_image;
        m_image = new QImage(width(), height(), MANDELBROT_QIMAGE_FORMAT);
        QPainter(m_image).fillRect(m_image->rect(), Qt::black);
    }
    m_imageIsReady = false;
    m_tilesFinishedRendering = 0;
    m_tiling.start(renderFirst);
    computeStats();
    // abort if required
    if(abortRenderingAsSoonAsPossible()) return;
    for(int i = 0; i < renderThreadCount(); i++) {
        renderThread(i)->start(QThread::LowestPriority);
    }
}

void Mandelbrot::translateView(const QPointF& _delta)
{
    abortRendering();

    // Compute the new complex coordinate of the viewpoint
    m_center -= resolution() * _delta;

    // now translate part the old image and merge it immediately into m_image to give the user a sense of speed
    QPoint delta((int)_delta.x(), (int)_delta.y());
    int srcx = delta.x()>0 ? 0 : -delta.x(),
        srcy = delta.y()>0 ? 0 : -delta.y(),
        dstx = delta.x()>0 ? delta.x() : 0,
        dsty = delta.y()>0 ? delta.y() : 0,
        w = m_image->width() - qAbs(delta.x()),
        h = m_image->height() - qAbs(delta.y());
    QImage part = m_image->copy(srcx,srcy,w,h);
    m_image->fill(0);
    QPainter(m_image).drawImage(QPointF(dstx,dsty), part);
    update(m_image->rect());

    // compute which pixel to render first, so we start with the tiles the user is most interested in
    qreal renderfirstx, renderfirsty;
    if(delta.y()!=0 && qAbs(_delta.x()/_delta.y())<qAbs(qreal(width())/height()))
    {
      if(delta.y()>0)
      {
        renderfirstx = width()/2 - _delta.x() * height() / (2 * _delta.y());
        renderfirsty = 0.;
      }
      else
      {
        renderfirstx = width()/2 + _delta.x() * height() / (2 * _delta.y());
        renderfirsty = height();
      }
    }
    else
    {
      if(delta.x()>0)
      {
        renderfirsty = height()/2 - _delta.y() * width() / (2 * _delta.x());
        renderfirstx = 0.;
      }
      else
      {
        renderfirsty = height()/2 + _delta.y() * width() / (2 * _delta.x());
        renderfirstx = width();
      }
    }
    startRendering(QPointF(renderfirstx, renderfirsty));
}

void Mandelbrot::zoomView(const QPointF& at, qreal zoomFactor)
{
    abortRendering();

    // Compute the new complex coordinate of the viewpoint
    qreal oldResolution = resolution();
    if(zoomFactor < 0.5) zoomFactor = 0.5;
    if(zoomFactor > 2) zoomFactor = 2;
    qreal newzoom = m_zoom * zoomFactor;
    if(newzoom>10.) newzoom = 10.;
    zoomFactor = newzoom/m_zoom;
    m_zoom = newzoom;
    qreal newResolution = resolution();
    m_center += (oldResolution - newResolution) * (at - QPointF(width()/2, height()/2));

    // now scale part the old image and merge it immediately into m_image to give the user a sense of speed
    qreal srcwidth = width() * qMin(zoomFactor,(qreal)(1.0));
    qreal srcheight = height() * qMin(zoomFactor,(qreal)(1.0));
    qreal srcleft = at.x() - srcwidth/2.;
    qreal srcright = at.x() + srcwidth/2.;
    qreal srctop = at.y() - srcheight/2.;
    qreal srcbottom = at.y() + srcheight/2.;
    qreal srcleft_c = CLAMP(srcleft, (qreal)(0.0), (qreal)width());
    qreal srcright_c = CLAMP(srcright, (qreal)(0.0), (qreal)width());
    qreal srctop_c = CLAMP(srctop, (qreal)(0.0), (qreal)height());
    qreal srcbottom_c = CLAMP(srcbottom, (qreal)(0.0), (qreal)height());
    qreal srcwidth_c = srcright_c - srcleft_c;
    qreal srcheight_c = srcbottom_c - srctop_c;
    qreal dstwidth = width() * qMin(1./zoomFactor,1.);
    qreal dstheight = height() * qMin(1./zoomFactor,1.);
    qreal dstleft = at.x() - dstwidth/2.;
    qreal dsttop = at.y() - dstheight/2.;
    QImage part(srcwidth, srcheight, m_image->format());
    if(zoomFactor>1.) part.fill(0);
    QPainter(&part).drawImage(QPointF(srcleft_c-srcleft, srctop_c-srctop), *m_image, QRectF(srcleft_c,srctop_c,srcwidth_c,srcheight_c));
    if(zoomFactor>1.) m_image->fill(0);
    QImage scaled = part.scaled(dstwidth, dstheight);
    QPainter(m_image)
      .drawImage(QPointF(dstleft, dsttop),
                scaled);
    update(m_image->rect());

    startRendering(at);
}

int Mandelbrot::maxIter() const
{
    int max_iter_factor;
    if(quality() == 0) max_iter_factor=100;
    else if(quality() == 1) max_iter_factor=250;
    else max_iter_factor=1000;
    return int(max_iter_factor * -std::log(resolution()));
}

int Mandelbrot::supersampling() const
{
    if(quality() <= 2) return 1;
    else if(quality() == 3) return 2;
    else return 4;
}

QString Mandelbrot::key() const
{
    // number of guaranteed significant digits in a qreal
    // beware: a too high number of digits will result in inconsistent keys and stale/not-found cached images,
    // while a too low number of digits will result in loading mismatching cached images (actually the latter is
    // almost impossible to avoid when the view is very close to machine precision limits, but that is probably
    // not a big problem).
    const int digits = (sizeof(qreal) >= 8) ? 15 : 6;
    // QString::number doesn't honor any locale setting, that's good for us. The Qt documentation doesn't say
    // whether that's also the case of arg() so let's play safe and use QString::number.
    // notice how we separate the numbers by a space. That prevents "12 3" giving the same key as "1 23".
    QString s = QString(QLatin1String( "%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15" ))
              .arg(QString::number(m_center.x(), 'g', digits))
              .arg(QString::number(m_center.y(), 'g', digits))
              .arg(QString::number(m_zoom, 'g', digits))
              .arg(m_quality).arg(width()).arg(height())
              .arg(m_color1.red()).arg(m_color1.green()).arg(m_color1.blue())
              .arg(m_color2.red()).arg(m_color2.green()).arg(m_color2.blue())
              .arg(m_color3.red()).arg(m_color3.green()).arg(m_color3.blue());
    return QString(QLatin1String( "mandelbrot-%1" )).arg(qHash(s));
}

void Mandelbrot::computeStats()
{
    if (height() < 1 || width() < 1) {
        return;
    }

    // run a little simulation on 15x15 samples to estimate roughly the minimum iteration count before divergence.
    // This number, m_min_iter_divergence, will be used to tune gradients.
    int max_iter = maxIter();
    m_min_iter_divergence = max_iter;
    for(int i = -7; i < 8; i++) for(int j = -7; j < 8; j++)
    {
      if(m_abortRenderingAsSoonAsPossible) return;
      qreal x_start = center().x() + i * zoom() / 8;
      qreal y_start = center().y() + i * (zoom() * height()/width()) / 8;
      qreal x = x_start;
      qreal y = y_start;
      int iter = 0;
      bool diverged = false;
      do {
          qreal tmp = (x*x - y*y + x_start);
          y = (2*x*y + y_start);
          x = tmp;
          if((x*x + y*y) > 4) diverged = true;
          else iter++;
      } while(iter < max_iter && !diverged);
      if(iter < m_min_iter_divergence) m_min_iter_divergence = iter;
    }
    if(m_min_iter_divergence < 1) m_min_iter_divergence = 1;
}

void Mandelbrot::tileDone(const MandelbrotTile& t)
{
    emit update(QRectF(t.destination()).translated(boundingRect().topLeft()));
    m_tilesFinishedRendering++;
    if(m_tilesFinishedRendering >= TILING_SIZE*TILING_SIZE) m_imageIsReady = true;
}

void Mandelbrot::exportImage()
{
    KUrl url = KFileDialog::getSaveUrl(
                   KUrl(),
                   QLatin1String( "*.png|" )+i18n( "PNG images" ),
                   0,
                   QString(),
                   KFileDialog::ConfirmOverwrite
                 );
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    m_image->save(&buffer, "PNG");
    KIO::file_delete(url);
    KIO::storedPut(ba, url, -1);
}

void Mandelbrot::exportConfig()
{
    QString file = KFileDialog::getSaveFileName(
                     KUrl(),
                     QLatin1String( "*.txt|" )+i18n( "Text files" ),
                     0,
                     QString(),
                     KFileDialog::ConfirmOverwrite
                   );
    KConfig config(file, KConfig::SimpleConfig);
    KConfigGroup configgroup(&config, "Mandelbrot");
    save(configgroup);
    configgroup.config()->sync();
}

void Mandelbrot::importConfig()
{
    QString file = KFileDialog::getOpenFileName(
                     KUrl(),
                     QLatin1String( "*.txt|" )+i18n( "Text files" ),
                     0,
                     QString()
                   );
    if(file.isEmpty()) return;
    KConfig config(file, KConfig::SimpleConfig);
    KConfigGroup configgroup(&config, "Mandelbrot");
    readConfig(configgroup, ReadViewpoint); // reading colors and quality level is implicit
}                                           // but we dont want to read the lock status

#include "mandelbrot.moc"
