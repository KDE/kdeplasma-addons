/***************************************************************************
 *   Copyright 2007 by André Duffeck <duffeck@kde.org>                     *
 *   Copyright 2007 by Alexis Ménard <darktears31@gmail.com>               *
 *   Copyright 2007 by Marco Martin <notmart@gmail.com>                    *
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

#include "showdashboard.h"

// Qt
#include <QDBusInterface>

// KDE
#include <KWindowSystem>
#include <KIconLoader>


ShowDashboard::ShowDashboard(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{

}

void ShowDashboard::init()
{
    setBackgroundHints(NoBackground);
    m_icon = new Plasma::Icon(KIcon("user-dashboard"),QString(),this);
    connect(m_icon, SIGNAL(pressed(bool)),this, SLOT(toggleShowDashboard(bool)));

}

void ShowDashboard::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            m_icon->setText(i18n("Show Desktop"));
            setMinimumSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
        } else {
            m_icon->setText(0);
            m_icon->setInfoText(0);
            setMinimumSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Panel)));
       }
    }
    if (constraints & Plasma::SizeConstraint && m_icon) {
        resize(size());
        m_icon->resize(size());
    }

    updateGeometry();
}

Qt::Orientations ShowDashboard::expandingDirections() const
{
    return Qt::Vertical;
}

void ShowDashboard::toggleShowDashboard(bool pressed)
{
    if (!pressed) {
        return;
    }
    QDBusInterface plasmaApp( "org.kde.plasma", "/App" );
    plasmaApp.call( "toggleDashboard" );
}


#include "showdashboard.moc"
