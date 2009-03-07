/***************************************************************************
 *   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
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

#include "arrowwidget.h"

#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>

#include <Plasma/Svg>

ArrowWidget::ArrowWidget( QGraphicsItem *parent, Qt::WindowFlags wFlags )
    : QGraphicsWidget( parent, wFlags ), mDirection( Plasma::Left )
{
    setCacheMode( DeviceCoordinateCache );
    setPreferredSize( 30, 30 );

    mArrow = new Plasma::Svg( this );
    mArrow->setImagePath( "widgets/arrows" );
    mArrow->setContainsMultipleImages( true );
    setDirection( mDirection );
}

ArrowWidget::~ArrowWidget()
{
}

void ArrowWidget::setDirection( Plasma::Direction direction )
{
    mDirection = direction;
    if ( mDirection == Plasma::Left ) {
        mArrowName = "left-arrow";
    } else if ( mDirection == Plasma::Right ) {
        mArrowName = "right-arrow";
    } else if ( mDirection == Plasma::Down ) {
        mArrowName = "down-arrow";
    } else if ( mDirection == Plasma::Up ) {
        mArrowName = "up-arrow";
    }
}

Plasma::Direction ArrowWidget::direction() const
{
    return mDirection;
}

void ArrowWidget::paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget* )
{
    p->setRenderHint( QPainter::Antialiasing );

    QRectF contentRect = this->rect();
    qreal arrowWidth = preferredWidth();
    qreal arrowHeight = preferredHeight();

    //there is not enough space to paint the arrows at their preferred size, so scale them down and keep their aspect ratio
    if ( ( arrowWidth > contentRect.width() ) || ( arrowHeight > contentRect.height() ) ) {
        arrowWidth = ( contentRect.height() > contentRect.width() ) ? contentRect.width() : contentRect.height();
        arrowHeight = arrowWidth;
    }

    qreal buttonLeft = contentRect.left() + ( contentRect.width() - arrowWidth ) / 2;
    qreal buttonTop = contentRect.top() + ( contentRect.height() - arrowHeight ) / 2;

    mArrow->paint( p, buttonLeft, buttonTop, arrowWidth, arrowHeight, mArrowName );
}

void ArrowWidget::mousePressEvent ( QGraphicsSceneMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton ) {
        emit clicked();
    }
}

#include "arrowwidget.moc"
