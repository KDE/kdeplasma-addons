/*
 *   Copyright 2009-2011 by Sebastian Kügler <sebas@kde.org>
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

struct KGraphicsWebSlicePrivate
{
    QString selector;
    QRectF sliceGeometry;
    QRectF originalGeometry;
    QString loadingText;
    QTimer* resizeTimer;
    QSizeF resizeNew;
    QRectF previewRect;
    bool previewMode;
    QColor previewMaskColor;
    QSize fullContentSize;
    QWebElementCollection elementCache;
    QHash<uint, QRect> selectorGeometry;
    QRect documentGeometry;
};

KGraphicsWebSlice::KGraphicsWebSlice( QGraphicsWidget *parent )
    : QGraphicsWebView( parent )
{
    d = new KGraphicsWebSlicePrivate;
    d->originalGeometry = QRectF();
    d->fullContentSize = QSize(1024,768);
    d->previewMaskColor = QColor("black");
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(finishedLoading(bool)));

    d->resizeTimer = new QTimer(this);
    d->resizeTimer->setInterval(100);
    d->resizeTimer->setSingleShot(true);
    d->resizeNew = QSizeF();
    connect(d->resizeTimer, SIGNAL(timeout()), SLOT(resizeTimeout()));

    //resize(300, 300);
}

KGraphicsWebSlice::~KGraphicsWebSlice()
{
    delete d;
}

void KGraphicsWebSlice::loadSlice(const QUrl &u, const QString &selector)
{
    if ( d->selector == selector && url() == u) {
        return;
    }
    setElement(selector);
    if (url() != u) {
        QGraphicsWebView::load(u);
    }
    setZoomFactor(1.0);
}

void KGraphicsWebSlice::setLoadingText(const QString &html)
{
    d->loadingText = html;
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
        d->selectorGeometry[qHash(el.toOuterXml())] = el.geometry();
    }
}

QRectF KGraphicsWebSlice::previewGeometry(const QString &selector) const
{
    return QRectF(page()->mainFrame()->findFirstElement(selector).geometry());
}

QRectF KGraphicsWebSlice::sliceGeometry(const QString &selector) const
{
    QWebElement el = page()->mainFrame()->findFirstElement(selector);
    return d->selectorGeometry.value(qHash(el.toOuterXml()));
}

void KGraphicsWebSlice::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    setTiledBackingStoreFrozen(true);
    d->resizeNew = event->newSize();
    d->resizeTimer->start();
    event->accept();
}

void KGraphicsWebSlice::resizeTimeout()
{
    QSizeF n = d->resizeNew;
    // Prevent oopses.
    if (n.width() > 2400 || n.height() > 2400) {
        qDebug() << "giant size, what's going on???????" << n.width();
        return;
    }
    refresh();
    setTiledBackingStoreFrozen(false);
}

void KGraphicsWebSlice::preview(const QString &selector)
{
    if (selector.isEmpty()) {
        setPreviewMode(false);
        refresh();
        return;
    }
    setPreviewMode(true);
    d->previewRect = previewGeometry(selector);
    update();
}

void KGraphicsWebSlice::setPreviewMode(bool on)
{
    showPage();
    if (on && !d->previewMode) {
        d->previewMode = on;
        refresh();
    }
    if (!on && d->previewMode) {
        d->previewMode = on;
        setZoomFactor(1.0);
        refresh();
    }
}

void KGraphicsWebSlice::showSlice(const QString &selector)
{
    QRectF r = sliceGeometry(selector);
    // qDebug() << "area=" << d->documentGeometry << ", viewportSize=" << page()->viewportSize() << ", contentsRect=" << contentsRect().size();
    if (!selector.isEmpty() && r.isValid()) {
        zoom(r);
    } else if (d->documentGeometry.width() != contentsRect().size().width()) {
        // Change zoom level only if the current one isn't appropriate
        // but reset to 1.0 first
        setZoomFactor(1.0);
        updateElementCache();
        zoom(d->documentGeometry);
    }
}

void KGraphicsWebSlice::zoom(const QRectF &area)
{
    if (!area.isValid()) {
        qDebug() << "invalid zoom area" << area;
        return;
    }

    qreal f = contentsRect().size().width() / qMax((qreal)1.0, area.width());

    // size: zoom page
    if (f > 0.1 && f < 32) { // within sane bounds?
        setZoomFactor(f);
    }

    if (area != sliceGeometry(d->selector)) {
        qDebug() << "different results.";
    }

    // position: move origin according to zoom
    QPointF viewPosition = area.topLeft();
    // We end up positioning too low, so move the new position up a little
    viewPosition = QPointF(viewPosition.x() * f, (viewPosition.y() * f) - (16 * f));
    page()->mainFrame()->setScrollPosition(viewPosition.toPoint());
}

void KGraphicsWebSlice::showPage()
{
    qreal zoom = 1.0;

    QSizeF o = d->documentGeometry.size();
    QSizeF s = o;
    s.scale(contentsRect().size(), Qt::KeepAspectRatio);

    zoom = s.width() / qMax((qreal)1.0, contentsRect().width());
    setZoomFactor(zoom);
    page()->mainFrame()->setScrollPosition(QPoint(0, 0));
    update();
}

QPixmap KGraphicsWebSlice::elementPixmap() const
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

void KGraphicsWebSlice::setPreviewMaskColor(const QColor &color)
{
    d->previewMaskColor = color;
    if (d->previewMode) {
        update();
    }
}

void KGraphicsWebSlice::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget)
{
    QGraphicsWebView::paint(painter, option, widget);
    if (!d->previewMode) {
        return;
    }
    d->previewMaskColor.setAlphaF(1.0);
    painter->setPen(d->previewMaskColor);
    d->previewMaskColor.setAlphaF(.5);
    painter->setBrush(d->previewMaskColor);

    QPainterPath path;
    path.addRect(boundingRect());
    path.addRect(d->previewRect);
    painter->drawPath(path);
    //painter->drawRect(d->previewRect);
}

#include "kgraphicswebslice.moc"

