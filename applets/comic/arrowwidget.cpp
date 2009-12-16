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

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>

#include <Plasma/Svg>

const int Arrow::HEIGHT = 28;
const int Arrow::WIDTH = 28;

Arrow::Arrow( QGraphicsItem *parent, Qt::WindowFlags wFlags )
  : QGraphicsWidget( parent, wFlags ),
    mDirection( Plasma::Left )
{
    setCacheMode( DeviceCoordinateCache );

    mArrow = new Plasma::Svg( this );
    mArrow->setImagePath( "widgets/arrows" );
    mArrow->setContainsMultipleImages( true );
    setDirection( mDirection );
}

Arrow::~Arrow()
{
}

QSizeF Arrow::sizeHint( Qt::SizeHint which, const QSizeF &constraint ) const
{
    Q_UNUSED( which )
    Q_UNUSED( constraint )

    return QSizeF( WIDTH, HEIGHT );
}

Plasma::Direction Arrow::direction() const
{
    return mDirection;
}

void Arrow::setDirection(Plasma::Direction direction)
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

void Arrow::paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget* )
{
    p->setRenderHint( QPainter::Antialiasing );

    mArrow->paint( p, 0, 0, WIDTH, HEIGHT, mArrowName );
}

ArrowWidget::ArrowWidget( QGraphicsItem *parent, Qt::WindowFlags wFlags )
    : QGraphicsWidget( parent, wFlags )
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout( Qt::Vertical );
    layout->setContentsMargins( 1, 1, 1, 1 );
    layout->addStretch();
    mArrow = new Arrow( this );
    layout->addItem( mArrow );
    layout->addStretch();

    setLayout( layout );
}

ArrowWidget::~ArrowWidget()
{
}

void ArrowWidget::setDirection( Plasma::Direction direction )
{
    mArrow->setDirection( direction );
}

Plasma::Direction ArrowWidget::direction() const
{
    return mArrow->direction();
}

void ArrowWidget::mousePressEvent ( QGraphicsSceneMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton ) {
        emit clicked();
    }
}

QSizeF ArrowWidget::sizeHint( Qt::SizeHint which, const QSizeF &constraint ) const
{
    if ( !isVisible() ) {
        return QSizeF( 0, 0 );
    }

    return QGraphicsWidget::sizeHint( which, constraint );
}

void ArrowWidget::hideEvent( QHideEvent *event )
{
    updateGeometry();
    QGraphicsWidget::hideEvent( event );
}

void ArrowWidget::showEvent(QShowEvent* event)
{
    updateGeometry();
    QGraphicsWidget::showEvent( event );
}

#include "arrowwidget.moc"
