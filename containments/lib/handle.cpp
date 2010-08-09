/*
 *   Copyright 2007 by Kevin Ottens <ervin@kde.org>
 *   Copyright 2009-2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "handle.h"

#include <QApplication>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QTouchEvent>

#include <kcolorscheme.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kwindowsystem.h>

#include <cmath>
#include <math.h>

#include <Plasma/Corona>
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include <Plasma/View>
#include <Plasma/FrameSvg>

#include "abstractgroup.h"
#include "groupingcontainment.h"

using namespace Plasma;

Handle::Handle(GroupingContainment *parent, Plasma::Applet *applet)
      : QGraphicsObject(applet),
        m_containment(parent),
        m_applet(applet),
        m_group(0),
        m_widget(applet),
        m_currentView(applet->view())
{
    setAcceptsHoverEvents(true);
    setAcceptTouchEvents(true);
}

Handle::Handle(GroupingContainment *parent, AbstractGroup *group)
      : QGraphicsObject(group),
        m_containment(parent),
        m_applet(0),
        m_group(group),
        m_widget(group),
        m_currentView(group->view())
{
    setAcceptsHoverEvents(true);
    setAcceptTouchEvents(true);
}

Handle::~Handle()
{
    detachWidget();
}

QGraphicsWidget *Handle::widget() const
{
    return m_widget;
}

Plasma::Applet *Handle::applet() const
{
    return m_applet;
}

AbstractGroup *Handle::group() const
{
    return m_group;
}

GroupingContainment * Handle::containment() const
{
    return m_containment;
}

QGraphicsView *Handle::currentView() const
{
    return m_currentView;
}

void Handle::detachWidget()
{
    if (!m_widget) {
        return;
    }

    m_widget->disconnect(this);

//     if (m_applet && (m_applet->geometry() != m_originalGeom || m_applet->transform() != m_originalTransform)) {
//         emit m_applet->appletTransformedByUser(); //FIXME: protected!
//     } else if (m_group && (m_group->geometry() != m_originalGeom || m_group->transform() != m_originalTransform)) {
//         emit m_group->groupTransformedByUser();
//     }

    m_applet = 0;
    m_group = 0;
    m_widget = 0;
}

bool Handle::leaveCurrentView(const QPoint &pos) const
{
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        if (widget->geometry().contains(pos)) {
            //is this widget a plasma view, a different view then our current one,
            //AND not a dashboardview?
            Plasma::View *v = qobject_cast<Plasma::View *>(widget);
            if (v && v != m_currentView && v->containment() != m_containment) {
                return true;
            }
        }
    }
    return false;
}

//pos relative to scene
void Handle::switchContainment(GroupingContainment *containment, const QPointF &pos)
{
    m_containment = containment;
    if (m_applet) {
        Applet *applet = m_applet;
        m_applet = 0; //make sure we don't try to act on the applet again
        applet->removeSceneEventFilter(this);
        setAcceptsHoverEvents(false);
//         forceDisappear(); //takes care of event filter and killing handle
        applet->disconnect(this); //make sure the applet doesn't tell us to do anything
        //applet->setZValue(m_zValue);
        containment->addApplet(applet, containment->mapFromScene(pos), false);
    } else {
        AbstractGroup *group = m_group;
        m_group = 0; //make sure we don't try to act on the applet again
        group->removeSceneEventFilter(this);
        setAcceptsHoverEvents(false);
//         forceDisappear(); //takes care of event filter and killing handle
        group->disconnect(this); //make sure the applet doesn't tell us to do anything
        //applet->setZValue(m_zValue);
        containment->addGroup(group, containment->mapFromScene(pos));
    }

    deleteLater();
}

void Handle::widgetDestroyed()
{
    m_applet = 0;
    m_group = 0;
    m_widget = 0;
}

void Handle::setHoverPos(const QPointF &hoverPos)
{
    if (!boundingRect().contains(hoverPos)) {
        emit disappearDone(this);
    }
}

#include "handle.moc"

