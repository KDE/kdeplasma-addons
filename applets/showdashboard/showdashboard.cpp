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
#include <QGraphicsLinearLayout>

// KDE
#include <KIcon>
#include <plasma/widgets/icon.h>
#include <plasma/tooltipmanager.h>
#include <KIconLoader>


ShowDashboard::ShowDashboard(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    setBackgroundHints(NoBackground);
    //setAspectRatioMode(Plasma::Square);
    resize(80, 80);
}

void ShowDashboard::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    Plasma::Icon *icon = new Plasma::Icon(KIcon("dashboard-show"), QString(), this);
    registerAsDragHandle(icon);
    layout->addItem(icon);

    Plasma::ToolTipManager::self()->registerWidget(this);
    Plasma::ToolTipManager::ToolTipContent toolTipData;
    toolTipData.mainText = i18n("Show your Plasma Dashboard");
    toolTipData.image = icon->icon().pixmap(IconSize(KIconLoader::Desktop));
    Plasma::ToolTipManager::self()->setToolTipContent(this, toolTipData);
    setAspectRatioMode(Plasma::ConstrainedSquare);

    connect(icon, SIGNAL(pressed(bool)),this, SLOT(toggleShowDashboard(bool)));
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
