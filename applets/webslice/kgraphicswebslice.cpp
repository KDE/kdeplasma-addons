/*
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>
 *   Copyright 2009 by Richard Moore <rich@kde.org>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kgraphicswebslice.h"

#include <qdebug.h>
#include <QGraphicsSceneResizeEvent>
#include <qgraphicswebview.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qwebelement.h>
#include <qwebpage.h>
#include <qwebframe.h>
#include <qboxlayout.h>

#include <qdebug.h>

struct KGraphicsWebSlicePrivate
{
    //QGraphicsWebView *view;
    QString selector;
    QRectF sliceGeometry;
    QRectF originalGeometry;
    QString loadingText;
    qreal currentZoom;
    QTimer* resizeTimer;
    QSizeF resizeNew;
    QSizeF resizeOriginal;
    QRectF previewRect;
    bool previewMode;
    QString chosenSelector;
    QSize fullContentSize;
    QWebElementCollection elementCache;
    QHash<QString, QRect> selectorGeometry;
    QRect documentGeometry;
};

KGraphicsWebSlice::KGraphicsWebSlice( QGraphicsWidget *parent )
    : QGraphicsWebView( parent )
{
    qDebug() << "kgws ctor start";
    d = new KGraphicsWebSlicePrivate;
    d->currentZoom = 1.0;
    d->originalGeometry = QRectF();
    d->fullContentSize = QSize(1024,768);
    //this = new QGraphicsWebView( this );
    //setOpacity(.5);
    //connect( this, SIGNAL(loadFinished(bool)), this, SLOT( createSlice(bool) ) );
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(finishedLoading(bool)));

    d->resizeOriginal = QSizeF();
    d->resizeNew = QSizeF();

    qDebug() << "kgws 1";
    /*
    QWebFrame *frame = page()->mainFrame();
    qDebug() << "kgws 2";
    frame->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    frame->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
    */
    qDebug() << "kgws 3";

    d->resizeTimer = new QTimer(this);
    d->resizeTimer->setInterval(500);
    d->resizeTimer->setSingleShot(true);
    connect(d->resizeTimer, SIGNAL(timeout()), SLOT(resizeTimeout()));

    resize(300, 300);
    qDebug() << "kgws ctor end";
}

KGraphicsWebSlice::~KGraphicsWebSlice()
{
    delete d;
}

void KGraphicsWebSlice::loadSlice(const QUrl &u, const QString &selector)
{
    //show();
    //QWebFrame *frame = page()->mainFrame();
    //frame->setHtml(d->loadingText);
    //qDebug() << "loadUrl: Loading url:" << url;
    if ( d->selector == selector && url() == u) {
        return;
    }
    setElement(selector);
    if (url() != u) {
        QGraphicsWebView::load(u);
    }
    setZoomFactor(1.0);
    //qDebug() << "loading url done";
}

void KGraphicsWebSlice::setLoadingText(const QString &html)
{
    d->loadingText = html;
}

/*
QUrl KGraphicsWebSlice::url()
{
    return url();
}
*/
QString KGraphicsWebSlice::element()
{
    return d->selector;
}

void KGraphicsWebSlice::setElement(const QString &selector)
{
    d->selector = selector;
}

void KGraphicsWebSlice::setSliceGeometry(const QRectF geo)
{
    d->sliceGeometry = geo;
}

void KGraphicsWebSlice::finishedLoading(bool ok)
{
    if (!ok) {
        return;
    }
    //Q_UNUSED(ok)
    // Normalize page
    qDebug() << "loading finished" << ok << ", updating cache then slice or preview";
    QWebFrame *frame = page()->mainFrame();
    frame->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    frame->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
    page()->setPreferredContentsSize(d->fullContentSize);

    // Update geometry caches
    updateElementCache();

    // Zoom to slice or preview
    refresh();
}

void KGraphicsWebSlice::refresh()
{
    qDebug() << "--------- refresh ------------";
    // TODO: optimize for non-changes
    if (!d->previewMode) {
        showSlice(d->selector);
    } else {
        showPage();
    }
}

void KGraphicsWebSlice::updateElementCache()
{
    qDebug() << "updateElementCache()";
    d->elementCache = page()->mainFrame()->findAllElements("*");
    d->documentGeometry = page()->mainFrame()->documentElement().geometry();
    foreach(const QWebElement &el, d->elementCache) {
        if (el.attributeNames().contains("id")) {
            QString elSelector;
            elSelector = QString("#%1").arg(el.attribute("id")); // according to CSS selector syntax
            d->selectorGeometry[elSelector] = el.geometry();
        }
        // TODO: Fix other attributes
    }
}

void KGraphicsWebSlice::createSlice(bool ok) // deprecated
{
    qDebug() << "loadFinished" << ok;

    QWebFrame *frame = page()->mainFrame();
    frame->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    frame->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );

    if (ok) {
        createSlice();
        qDebug() << "createSlice() done";
    }
    else {
      qDebug() << "loadFinished called createSlice with false";
      //emit loadFinished(false);
    }
}

QWebElement KGraphicsWebSlice::findElementById(const QString &selector)
{
    foreach(const QWebElement &el, d->elementCache) {
        QString elSelector;
        if (el.attributeNames().contains("id")) {
            elSelector = QString("#%1").arg(el.attribute("id")); // according to CSS selector syntax
            if (elSelector == selector) {
                //qDebug() << "Found Element! :-)" << elSelector << el.geometry() << QRectF(el.geometry());
                return el;
            }
        }
    }
    return QWebElement();
}

void KGraphicsWebSlice::createSlice() // deprecated
{
    return;
#if 0
    qDebug() << "== == == == KGraphicsWebSlice::createSlice()";
    QRectF geo = sliceGeometry();
    QWebFrame *frame = page()->mainFrame();
    if (geo.isValid()) {
        d->originalGeometry = geo;

        qreal f = size().width() / geo.width();

        if (f > 0.1 && f < 8) {
            setZoomFactor(f);
            d->currentZoom = f;
        }

        QSizeF viewSize = geo.size();
        viewSize.scale(contentsRect().size(), Qt::KeepAspectRatio);
        resize(viewSize);
        qDebug() << "Resized to" << viewSize;

        QSizeF center = size()/2 - viewSize/2;
        qDebug() << "Pos / Size:" << geo.topLeft().toPoint() << geo.size();
        setPos(center.width(), center.height());
        frame->setScrollPosition( geo.topLeft().toPoint() );
        refresh();
        emit sizeChanged(geo.size());
        //emit loadFinished(true);
    } else {
      qDebug() << "createSlice was unable to find the geometry (fail) for " << d->selector;
      //emit loadFinished(false);
    }
#endif
}

QWebFrame* KGraphicsWebSlice::frame()
{
    return page()->mainFrame();
}

QRectF KGraphicsWebSlice::previewGeometry(const QString &selector)
{
    // Normalize slice with zoom within page
    QRectF geo = findElementById(selector).geometry();
    //qDebug() << "orig:" << geo;
    return geo;

    /*
    qDebug() << "zoom:" << zoomFactor();
    QSizeF s = geo.size() * zoomFactor();
    QPointF p = geo.topLeft();
    qDebug() << "topLeft:" << p;
    qDebug() << "p x, y" << (p.x() * zoomFactor()) << (p.y() * zoomFactor());
    p = QPointF(p.x() * zoomFactor(), p.y() * zoomFactor());
    geo = QRectF(p, s);
    qDebug() << "preview:" << geo;
    return geo;
    */
}

QRectF KGraphicsWebSlice::sliceGeometry(const QString &selector)
{
    //QWebFrame *frame = page()->mainFrame();
    QRectF geo = QRectF();
    if (!selector.isEmpty() && d->selectorGeometry.keys().contains(selector)) {
        geo = d->selectorGeometry[selector];
        qDebug() << "geometry for:" << selector << geo << zoomFactor();
        /*
        geo = findElementById(selector).geometry();
        //QWebElement element;

        QSizeF s = geo.size() / zoomFactor();
        QPointF p = geo.topLeft();
        qDebug() << "topLeft:" << p;
        qDebug() << "p x, y" << (p.x() / zoomFactor()) << (p.y() / zoomFactor());
        p = QPointF(p.x() * zoomFactor(), p.y() / zoomFactor());
        geo = QRectF(p, s);
        qDebug() << "normalized:" << geo;
        return geo;
        / *
        element = frame->findFirstElement(d->selector);
        QSizeF s;
        QPointF p;
        if (!element.isNull()) {
            //page()->setPreferredContentsSize(d->fullContentSize);
            //page()->setPreferredContentsSize(QSize(1024,768));

            geo = element.geometry();
            s = geo.size() * zoomFactor();
            p = geometry().topLeft();
            p = QPointF(p.x() * zoomFactor(), p.y() * zoomFactor());
            geo = QRectF(p, s);
            qDebug() << "-----> Zoom:" << zoomFactor() << geo;
            qDebug() << "-----> Selector:" << selector << element.attribute("id") << element.attributeNames() << element.classes();
        }
        return geo;
        */
    }
    return geo;
    if (!d->selector.isEmpty()) {
        //QWebElement element = frame->findFirstElement(d->selector);
        QWebElement element = findElementById(d->selector);
        if (!element.isNull()) {
            //page()->setPreferredContentsSize(QSize(1024,768));
            geo = element.geometry();
            //if (geo.topLeft() != QPointF(0, 0)) {
            //    qDebug() << "THIS ONE IS GOOD!?!" << geo;
            //}
        }
    } else if (!d->sliceGeometry.isEmpty()) {
        //page()->setPreferredContentsSize(QSize(1024,768));
        geo = d->sliceGeometry;
        qDebug() << "Fixed geometry" << d->sliceGeometry;
    } else {
        //page()->setPreferredContentsSize(QSize(-1,-1));
        //QWebElement element = frame->documentElement();
        geo = d->documentGeometry;
    }

    if (!geo.isValid()) {
        return QRectF();
    }

    return geo;
}

/*
void KGraphicsWebSlice::refresh()
{
    QRectF geo = sliceGeometry();
    if (!geo.isValid()) {
        return;
    }

    QSizeF viewSize = geo.size();
    viewSize.scale(size(), Qt::KeepAspectRatioByExpanding);
    viewSize = viewSize.boundedTo(size());
    resize(viewSize);
    QSizeF center = size()/2 - viewSize/2;
    setPos(center.width(), center.height());

    QWebFrame *frame = page()->mainFrame();
    frame->setScrollPosition( geo.topLeft().toPoint() );
    setPreferredSize(geo.size());
    updateGeometry();
}
*/
void KGraphicsWebSlice::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    setTiledBackingStoreFrozen(true);
    //qDebug() << "KGraphicsWebSlice::resizeEvent" << event->newSize() << "(" << event->oldSize() << ")";
    d->resizeOriginal = d->originalGeometry.size();
    d->resizeNew = event->newSize();
    qDebug() << "resize event";
    d->resizeTimer->start();
    event->accept();
}

void KGraphicsWebSlice::resizeTimeout()
{
    qDebug() << "Resizing....";
    QSizeF n = d->resizeNew;
    QSizeF o = d->resizeOriginal;
    // Prevent oopses.
    if (n.width() > 2400 || n.height() > 2400) {
        qDebug() << "giant size, what's going on???????" << o.width();
        return;
    }
    /*
    qreal f = n.width() / o.width();

    if (f > 0.1 && f < 8) {
        setZoomFactor(f);
        d->currentZoom = f;
        refresh();
        qDebug() << "Zoom  :" << n.width() << " / " <<  o.width() << " = " << f;
    }
    */
    refresh();
    setTiledBackingStoreFrozen(false);
}

void KGraphicsWebSlice::preview(const QString &selector)
{
    if (selector.isEmpty()) {
        setPreviewMode(false);
        refresh();
        //update();
        return;
    }
    setPreviewMode();
    d->previewRect = previewGeometry(selector);
    qDebug() << "Previewing ..." << selector << d->previewRect;
    update();
}

void KGraphicsWebSlice::setPreviewMode(bool on)
{
    //setZoomFactor(1);
    showPage();
    if (on && !d->previewMode) {
        qDebug() << "preview on";
        d->chosenSelector = d->selector;
        d->previewMode = on;
        refresh();
    }
    if (!on && d->previewMode) {
        d->previewMode = on;
        setZoomFactor(1.0);
        qDebug() << "setPreviewMode" << on << zoomFactor();
        setElement(d->chosenSelector);
        refresh();
    }
    //resizeTimeout();
}

void KGraphicsWebSlice::showSlice(const QString &selector)
{
    QRectF r = sliceGeometry(selector);
    qDebug() << "showSlice()" << selector << r << zoomFactor();
    if (!selector.isEmpty() && r.isValid()) {
        zoom(r);
    } else {
        zoom(d->documentGeometry);
    }
}

void KGraphicsWebSlice::zoom(const QRectF &area)
{
    //QRectF geo = sliceGeometry();
    if (!area.isValid()) {
        qDebug() << "invalid zoom area" << area;
        return;
    }
    //d->originalGeometry = geo; // ??

    qreal f = contentsRect().size().width() / qMax((qreal)1.0, area.width());
    //qreal fh = contentsRect().size().height() / qMax((qreal)1.0, area.height());
    //qreal f = qMin(fw, fh);

    qDebug() << "zoom()" << contentsRect().size() << area << f;

    //qDebug() << area.width() << qMax((qreal)1.0, contentsRect().size().width()) << f;

    // size: zoom page
    if (f > 0.1 && f < 32) { // within sane bounds?
        setZoomFactor(f);
        d->currentZoom = f;
    }

    if (area != sliceGeometry(d->selector)) {
        qDebug() << "different results.";
    }

    // Resize zoom area geometry
    //QSizeF viewSize = area.size();
    //viewSize.scale(size(), Qt::KeepAspectRatio);
    //viewSize = viewSize.boundedTo(size());

    // position: move origin according to zoom
    QPointF viewPosition = area.topLeft();
    viewPosition = QPointF(viewPosition.x() * f, (viewPosition.y() * f) - (16 * f));
    //resize(viewSize); // ??
    //qDebug() << "Resized to" << viewSize;

    //QSizeF center = size()/2 - viewSize/2;

    qDebug() << "Pos / Size:" << area.topLeft().toPoint() << area.size();
    //setPos(center.width(), center.height());
    //QWebFrame *frame = page()->mainFrame();
    page()->mainFrame()->setScrollPosition(viewPosition.toPoint());
    //refresh();
    //emit sizeChanged(geo.size());
    //emit loadFinished(true);
}

void KGraphicsWebSlice::showPage()
{
    qreal zoom = 1.0;

    QSizeF o = d->documentGeometry.size();
    QSizeF s = o;
    s.scale(contentsRect().size(), Qt::KeepAspectRatio);

    zoom = s.width() / qMax((qreal)1.0, contentsRect().size().width());

    qDebug() << "Page zoom:" << s.width()  << contentsRect().size().width() << zoom;
    setZoomFactor(zoom);
    page()->mainFrame()->setScrollPosition(QPoint(0, 0));
    update();
}

QPixmap KGraphicsWebSlice::elementPixmap()
{
    QRectF rect = sliceGeometry();
    if (!rect.isValid()) {
        return QPixmap();
    }
    QPixmap result = QPixmap( rect.size().toSize() );
    result.fill( Qt::white );

    QPainter painter( &result );
    painter.translate( -rect.x(), -rect.y() );
    QWebFrame *frame = page()->mainFrame();
    frame->render( &painter, QRegion(rect.toRect()) );

    return result;
}

void KGraphicsWebSlice::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget)
{
    QGraphicsWebView::paint(painter, option, widget);
    //painter->save();
    if (!d->previewMode) {
        return;
    }
    QColor c("orange");
    painter->setPen(c);
    c.setAlphaF(.5);
    painter->setBrush(c);

    //qDebug() << "preview" << d->previewRect << mapFromPage(d->previewRect) << contentsRect();
    //painter->drawRect(QRectF(10, 30, 20, 40));
    painter->drawRect(d->previewRect);
    qDebug() << "preview rect drawn" << d->previewRect;
    //painter->drawRect(contentsRect());

    //painter->restore();
}

QRectF KGraphicsWebSlice::mapFromPage(const QRectF &rectOnPage)
{
    QRectF rectOnWidget;

    qreal ah = size().height() / rectOnPage.height();
    qreal aw = size().width() / rectOnPage.width();

    QPointF o = QPointF(rectOnPage.x() * aw, rectOnPage.y() * ah);
    QSizeF s = QSizeF(rectOnPage.size().width() * aw, rectOnPage.height());

    return QRectF(o, s);
}
