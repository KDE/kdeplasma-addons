/***************************************************************************
 *   Copyright (C) 2008 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "fullviewwidget.h"

#include <QtGui/QDesktopWidget>
#include <QtGui/QPainter>
#include <QWheelEvent>

FullViewWidget::FullViewWidget()
    : QWidget( 0, Qt::Popup )
{
    const QDesktopWidget desktop;
    mDesktopSize = desktop.availableGeometry( this );
}

FullViewWidget::~FullViewWidget()
{
}

void FullViewWidget::setImage( const QImage &image )
{
    mImage = image;
    resize( mImage.size() );
}

void FullViewWidget::adaptPosition( const QPoint &pos )
{
    int x = pos.x();
    int y = pos.y();
    if ( x + width() > mDesktopSize.width() )
        x = mDesktopSize.left();
    if ( y + height() > mDesktopSize.height() )
        y = mDesktopSize.top();

    move( x, y );
}

void FullViewWidget::paintEvent( QPaintEvent* )
{
    QPainter p( this );
    p.drawImage( 0, 0, mImage );
}

void FullViewWidget::mousePressEvent( QMouseEvent* )
{
    hide();
}

void FullViewWidget::wheelEvent( QWheelEvent *event )
{
    const int numDegrees = event->delta() / 8;
    const int numSteps = numDegrees / 15;
    const int scroll = numSteps * 30;

    QPoint futurePos = this->pos();
    const int bufferSpace = 70;
    //horizontal
    if ( event->modifiers() == Qt::AltModifier ) {
        futurePos += QPoint( scroll, 0 );
        if ( ( bufferSpace >= futurePos.x() + width() - mDesktopSize.left() ) ||
             ( bufferSpace >= mDesktopSize.right() - futurePos.x() ) ) {
            return;
        }
    //vertical
    } else {
        futurePos += QPoint( 0, scroll );
        if ( ( 70 >= futurePos.y() + height() - mDesktopSize.top() ) ||
            ( 70 >= mDesktopSize.bottom() - futurePos.y() ) ) {
            return;
        }
    }

    move( futurePos );

    QWidget::wheelEvent( event );
}
