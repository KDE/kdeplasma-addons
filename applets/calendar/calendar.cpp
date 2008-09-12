/***************************************************************************
 *   Copyright 2007 by Marco Martin <notmart@gmail.com>                    *
 *   Copyright 2008 by Montel Laurent <montel@kde.org>                     *
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

#include "calendar.h"
#include <KDatePicker>
#include <KIcon>
#include <plasma/widgets/icon.h>

Calendar::Calendar(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args), m_datePicker( 0 )
{
    setPopupIcon("view-pim-calendar");
}

Calendar::~Calendar()
{
    delete m_datePicker;
}

QWidget *Calendar::widget()
{
    if ( !m_datePicker )
    {
        m_datePicker =  new KDatePicker();
        m_datePicker->setMinimumSize( 280, 280 );
    }
    return m_datePicker;
}

#include "calendar.moc"

