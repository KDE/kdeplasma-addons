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

#include "kwebslice.h"
#include "kgraphicswebslice.h"

#include <qdebug.h>

struct KWebSlicePrivate
{
    KGraphicsWebSlice *slice;
    QString selector;
};

KWebSlice::KWebSlice( QWidget *parent )
    : QGraphicsView( parent )
{
    d = new KWebSlicePrivate;
    d->slice = new KGraphicsWebSlice;
    connect(d->slice, SIGNAL(sizeChanged(QSizeF)), this, SLOT(sizeChanged(QSizeF)));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsScene *qgs = new QGraphicsScene(this);
    qgs->addItem(d->slice);
    qgs->setActiveWindow(d->slice);
    setScene(qgs);

    setMinimumSize(20,20);
    //setPreferedSize(20,80);
}

KWebSlice::~KWebSlice()
{
    delete d;
}

void KWebSlice::setUrl( const QUrl &url )
{
    d->slice->setUrl( url );
}

void KWebSlice::setLoadingText(const QString &html)
{
    d->slice->setLoadingText(html);
}

void KWebSlice::setElement( const QString &selector )
{
    d->slice->setElement(selector);
}

void KWebSlice::sizeChanged(QSizeF newsize)
{
    //qDebug() << "size changed" << newsize;
    QRectF newgeometry = QRectF(QPointF(0, 0), newsize);
    setSceneRect(newgeometry);
    resize(newgeometry.toRect().size());
}

QSize KWebSlice::sizeHint () const
{
    return sceneRect().size().toSize();
}

void KWebSlice::resizeEvent ( QResizeEvent * event )
{
    QRectF newgeometry = QRectF(QPointF(0, 0), event->size());
    //qDebug() << "KWebSlice::resizeEvent" << newgeometry << "(" << event->oldSize() << ")";
    setSceneRect(newgeometry);
    d->slice->setGeometry(newgeometry);
}
