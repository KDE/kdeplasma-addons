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
}

FullViewWidget::~FullViewWidget()
{
}

void FullViewWidget::setImage( const QImage &image )
{
    mImage = image;
    resize( mImage.size() );
}

void FullViewWidget::adaptPosition( const QPoint &pos, int screenId )
{
    if ( !mDesktopSize.isValid() ) {
        const QDesktopWidget desktop;
        mDesktopSize = desktop.screenGeometry( screenId );
    }

    const bool contains = mDesktopSize.contains( pos, true );
    int x = ( contains ? pos.x() : mDesktopSize.left() );
    int y = ( contains ? pos.y() : mDesktopSize.top() );

    //left() and top() needed for multiple screens
    const int neededX = x - mDesktopSize.left() + width();
    const int neededY = y - mDesktopSize.top() + height();

    if ( neededX > mDesktopSize.width() ) {
        x = mDesktopSize.left();
    }
    if ( neededY > mDesktopSize.height() ) {
        y = mDesktopSize.top();
    }

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

    //vertical
    if ( ( mImage.height() > mDesktopSize.height() ) && ( event->modifiers() != Qt::AltModifier ) ) {
        futurePos += QPoint( 0, scroll );
        if ( ( scroll > 0 ) && ( futurePos.y() > mDesktopSize.top() ) ) {
            futurePos.setY( mDesktopSize.top() );
        } else if ( ( scroll < 0 ) && ( futurePos.y() + height() < mDesktopSize.bottom() ) ) {
            futurePos.setY( mDesktopSize.bottom() - height() );
        }
    //horizontal
    } else if ( mImage.width() > mDesktopSize.width() ) {
        futurePos += QPoint( scroll, 0 );
        if ( ( scroll > 0 ) && ( futurePos.x() > mDesktopSize.left() ) ) {
            futurePos.setX( mDesktopSize.left() );
        } else if ( ( scroll < 0 ) && ( futurePos.x() + width() < mDesktopSize.right() ) ) {
            futurePos.setX( mDesktopSize.right() - width() );
        }
    }

    move( futurePos );

    QWidget::wheelEvent( event );
}
