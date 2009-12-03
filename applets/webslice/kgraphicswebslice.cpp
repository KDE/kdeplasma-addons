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
#include <qlabel.h>
#include <qgraphicswebview.h>
#include <qwebelement.h>
#include <qwebpage.h>
#include <qwebframe.h>
#include <qboxlayout.h>

struct KGraphicsWebSlicePrivate
{
    QGraphicsWebView *view;
    QString selector;
    QRectF sliceGeometry;
    QRectF originalGeometry;
    QString loadingText;
    qreal currentZoom;
};

KGraphicsWebSlice::KGraphicsWebSlice( QGraphicsWidget *parent )
    : QGraphicsWidget( parent )
{
    d = new KGraphicsWebSlicePrivate;
    d->currentZoom = 1.0;
    d->originalGeometry = QRectF();
    d->view = new QGraphicsWebView( this );
    connect( d->view, SIGNAL( loadFinished(bool) ), this, SLOT( createSlice(bool) ) );

    QWebFrame *frame = d->view->page()->mainFrame();
    frame->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    frame->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
}

KGraphicsWebSlice::~KGraphicsWebSlice()
{
    delete d;
}

void KGraphicsWebSlice::setUrl( const QUrl &url )
{
    QWebFrame *frame = d->view->page()->mainFrame();
    frame->setHtml(d->loadingText);
    d->view->load( url );
}

void KGraphicsWebSlice::setLoadingText(const QString &html)
{
    d->loadingText = html;
}

QUrl KGraphicsWebSlice::url()
{
    return d->view->url();
}

QString KGraphicsWebSlice::element()
{
    return d->selector;
}

void KGraphicsWebSlice::setElement( const QString &selector )
{
    d->selector = selector;
}

void KGraphicsWebSlice::setSliceGeometry( const QRectF geo )
{
    d->sliceGeometry = geo;
}

void KGraphicsWebSlice::createSlice(bool ok)
{
    if (ok) {
        createSlice();
    }
}

void KGraphicsWebSlice::createSlice()
{
    //qDebug() << "KGraphicsWebSlice::createSlice()";
    QRectF geo = sliceGeometry();
    QWebFrame *frame = d->view->page()->mainFrame();
    if (geo.isValid()) {
        d->originalGeometry = geo;
        d->view->resize( geo.size() );
        frame->setScrollPosition( geo.topLeft().toPoint() );
        refresh();
        emit sizeChanged(geo.size());
        emit loadFinished();
    }
}

QRectF KGraphicsWebSlice::sliceGeometry()
{
    QWebFrame *frame = d->view->page()->mainFrame();
    QRectF geo = QRectF();
    if (!d->selector.isEmpty()) {
       QWebElement element = frame->findFirstElement( d->selector );
        if ( element.isNull() ) {
            return QRectF();
        }
        geo = element.geometry();
    } else if (d->sliceGeometry.isValid()) {
        geo = d->sliceGeometry;
    }
    if (!geo.isValid()) {
        return QRectF();
    }
    return geo;
}

void KGraphicsWebSlice::refresh()
{
    QRectF geo = sliceGeometry();
    if (!geo.isValid()) {
        return;
    }
    d->view->resize( geo.size() );
    QWebFrame *frame = d->view->page()->mainFrame();
    frame->setScrollPosition( geo.topLeft().toPoint() );
    setPreferredSize(geo.size());
    updateGeometry();
}

void KGraphicsWebSlice::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    //qDebug() << "KGraphicsWebSlice::resizeEvent" << event->newSize() << "(" << event->oldSize() << ")";
    QSizeF o = d->originalGeometry.size();
    QSizeF n = event->newSize();

    // Prevent oopses.
    if (n.width() > 2400 || n.height() > 2400) {
        qDebug() << "giant size, what's going on???????" << o.width();
        return;
    }
    qreal f = n.width() / o.width();
    if (f > 0.1 && f < 8) {
        d->view->setZoomFactor(f);
        refresh();
        qDebug() << "Zoom  :" << n.width() << " / " <<  o.width() << " = " << f;
    }
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
    QWebFrame *frame = d->view->page()->mainFrame();
    frame->render( &painter, QRegion(rect.toRect()) );

    return result;
}
