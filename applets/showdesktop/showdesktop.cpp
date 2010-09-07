/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "showdesktop.h"
#include <plasma/widgets/iconwidget.h>
#include <plasma/containment.h>
#include <plasma/tooltipmanager.h>
#include <KGlobalSettings>
#include <KIcon>
#include <kwindowsystem.h>
#include <netwm.h>
#include <KIconLoader>
#include <QX11Info>
#include <QTimer>
#include <QGraphicsLinearLayout>

ShowDesktop::ShowDesktop(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_wm2ShowingDesktop(false)
#ifndef MINIMIZE_ONLY
      , m_down(false), m_goingDown(false)
#endif
{
    setAspectRatioMode(Plasma::ConstrainedSquare);
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize * 2, iconSize * 2);
    setAcceptDrops(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(minimizeAll()));
}

ShowDesktop::~ShowDesktop()
{
}

void ShowDesktop::init()
{
    connect(this, SIGNAL(activate()), this, SLOT(minimizeAll()));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    Plasma::IconWidget *icon = new Plasma::IconWidget(KIcon("user-desktop"), QString(), this);
    layout->addItem(icon);
    registerAsDragHandle(icon);
    connect(icon, SIGNAL(clicked()), this, SLOT(minimizeAll()));

    Plasma::ToolTipContent toolTipData(i18n("Show the Desktop"),
                                       i18n("Minimize all open windows and show the Desktop"),
                                       icon->icon().pixmap(IconSize(KIconLoader::Desktop)));
    Plasma::ToolTipManager::self()->setContent(this, toolTipData);

    NETRootInfo info(QX11Info::display(), NET::Supported);
    m_wm2ShowingDesktop = info.isSupported(NET::WM2ShowingDesktop);

#ifndef MINIMIZE_ONLY
    if (m_wm2ShowingDesktop) {
        connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(reset()));
    }
#endif

    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
        this, SLOT(iconSizeChanged(int)));
}

void ShowDesktop::minimizeAll()
{
    m_timer.stop();
    if (m_wm2ShowingDesktop) {
        NETRootInfo info(QX11Info::display(), 0);
#ifndef MINIMIZE_ONLY
        m_down = !m_down;
        m_goingDown = m_down;
        info.setShowingDesktop(m_down);
        // NETRootInfo::showingDesktop() returns always false
        QTimer::singleShot(500, this, SLOT(delay()));
#else
        info.setShowingDesktop(true);
#endif
    }
}

void ShowDesktop::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    m_timer.start(750);
    event->accept();
}

void ShowDesktop::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    m_timer.stop();
}
#ifndef MINIMIZE_ONLY

void ShowDesktop::delay()
{
    m_goingDown = false;
}

void ShowDesktop::reset()
{
    if (!m_goingDown) {
        m_down = false;
    }
}

QSizeF ShowDesktop::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
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

void ShowDesktop::iconSizeChanged(int group)
{
    if (group == KIconLoader::Desktop || group == KIconLoader::Panel) {
        updateGeometry();
    }
}

#endif

#include "showdesktop.moc"
