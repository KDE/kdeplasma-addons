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

void FullViewWidget::adaptPosition( const QPoint &pos )
{
    const QDesktopWidget desktop;
    const QRect desktopSize = desktop.availableGeometry( this );

    int x = pos.x();
    int y = pos.y();
    if ( x + width() > desktopSize.width() )
        x = desktopSize.width() - width();
    if ( y + height() > desktopSize.height() )
        y = desktopSize.height() - height();

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
