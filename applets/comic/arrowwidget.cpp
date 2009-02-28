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

static const int s_arrowWidth = 30;
static const int s_arrowHeight = 30;

ArrowWidget::ArrowWidget( QGraphicsItem *parent, Qt::WindowFlags wFlags )
    : QGraphicsWidget( parent, wFlags ), mArrowName( "left-arrow" ), mDirection( left )
{
    setCacheMode( DeviceCoordinateCache );
    setPreferredWidth( s_arrowWidth );
    mArrow = new Plasma::Svg( this );
    mArrow->setImagePath( "widgets/arrows" );
    mArrow->setContainsMultipleImages( true );
}

ArrowWidget::~ArrowWidget()
{
}

void ArrowWidget::setDirection( ArrowWidget::Direction direction )
{
    mDirection = direction;
    if ( mDirection == left ) {
        mArrowName = "left-arrow";
    } else if ( mDirection == right ) {
        mArrowName = "right-arrow";
    }
}

ArrowWidget::Direction ArrowWidget::direction() const
{
    return mDirection;
}

void ArrowWidget::paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget* )
{
    p->setRenderHint( QPainter::Antialiasing );

    qreal buttonMiddle = ( rect().height() / 2 ) + rect().top();
    mArrow->paint( p, rect().left(), buttonMiddle - s_arrowHeight / 2, s_arrowWidth, s_arrowHeight, mArrowName );
}

void ArrowWidget::mousePressEvent ( QGraphicsSceneMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton ) {
        emit clicked();
    }
}

#include "arrowwidget.moc"
