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
#include <plasma/widgets/iconwidget.h>
#include <plasma/tooltipmanager.h>
#include <KIconLoader>
#include <KGlobalSettings>


ShowDashboard::ShowDashboard(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    setBackgroundHints(NoBackground);
    //setAspectRatioMode(Plasma::Square);
    resize(80, 80);
    setAcceptDrops(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(toggleShowDashboard()));
}

void ShowDashboard::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    Plasma::IconWidget *icon = new Plasma::IconWidget(KIcon(QLatin1String("dashboard-show")), QString(), this);
    registerAsDragHandle(icon);
    icon->setMinimumSize(16, 16);
    setMinimumSize(16, 16);
    layout->addItem(icon);

    Plasma::ToolTipManager::self()->registerWidget(this);
    Plasma::ToolTipContent toolTipData(i18n("Show the Plasma Dashboard"), QString(),
                                       icon->icon().pixmap(IconSize(KIconLoader::Desktop)));
    Plasma::ToolTipManager::self()->setContent(this, toolTipData);
    setAspectRatioMode(Plasma::ConstrainedSquare);

    connect(icon, SIGNAL(pressed(bool)),this, SLOT(toggleShowDashboard(bool)));
    connect(this, SIGNAL(activate()), this, SLOT(toggleShowDashboard()));
    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
        this, SLOT(iconSizeChanged(int)));
}

void ShowDashboard::toggleShowDashboard(bool pressed)
{
    if (!pressed) {
        return;
    }

    toggleShowDashboard();
}

void ShowDashboard::toggleShowDashboard()
{
    m_timer.stop();
    QDBusInterface plasmaApp( QLatin1String("org.kde.plasma-desktop"), QLatin1String("/App") );
    plasmaApp.call( QLatin1String("toggleDashboard") );
}

void ShowDashboard::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    m_timer.start(750);
    event->accept();
}

void ShowDashboard::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    m_timer.stop();
}

QSizeF ShowDashboard::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (which == Qt::PreferredSize) {
        int iconSize;

        switch (formFactor()) {
            case Plasma::Planar:
            case Plasma::MediaCenter:
                iconSize = IconSize(KIconLoader::Desktop);
                break;

            case Plasma::Horizontal:
            case Plasma::Vertical:
                iconSize = IconSize(KIconLoader::Panel);
                break;
        }

        return QSizeF(iconSize, iconSize);
    }

    return Plasma::Applet::sizeHint(which, constraint);
}

void ShowDashboard::iconSizeChanged(int group)
{
    if (group == KIconLoader::Desktop || group == KIconLoader::Panel) {
        updateGeometry();
    }
}

#include "showdashboard.moc"
