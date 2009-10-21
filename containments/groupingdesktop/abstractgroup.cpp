/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "abstractgroup.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneResizeEvent>

#include <kservice.h>
#include <kservicetypetrader.h>

#include <Plasma/Containment>

class AbstractGroupPrivate
{
    public:
        AbstractGroupPrivate(AbstractGroup *group)
            : q(group),
              destroying(false),
              m_mainConfig(0)
{
        }

        ~AbstractGroupPrivate()
        {
            delete m_mainConfig;
        }

        KConfigGroup *mainConfigGroup()
        {
            if (m_mainConfig) {
                return m_mainConfig;
            }

            KConfigGroup containmentGroup = q->containment()->config();
            KConfigGroup groupsConfig = KConfigGroup(&containmentGroup, "Groups");
            KConfigGroup *mainConfig = new KConfigGroup(&groupsConfig, QString::number(q->id()));

            return mainConfig;
        }

        Plasma::Applet::List applets;
        AbstractGroup *q;
        bool destroying;

    private:
        KConfigGroup *m_mainConfig;
};

AbstractGroup::AbstractGroup(QObject *parent, const QVariantList &args)
             : Plasma::Applet(parent, args),
               d(new AbstractGroupPrivate(this))
{
    setAcceptDrops(true);
    setBackgroundHints(Plasma::Applet::TranslucentBackground);
}

AbstractGroup::~AbstractGroup()
{
    foreach (Plasma::Applet *applet, assignedApplets()) {
        applet->setParentItem(containment());
    }

    delete d;
}

void AbstractGroup::init()
{
    connect(containment(), SIGNAL(appletRemoved(Plasma::Applet *)),
            this, SLOT(onAppletRemoved(Plasma::Applet *)));
}

void AbstractGroup::assignApplet(Plasma::Applet *applet, bool layoutApplets)
{
    if (!applet) {
        return;
    }

    kDebug()<<"adding applet in group"<<id();

    applet->setParentItem(this);
    d->applets << applet;

    if (layoutApplets) {
        layoutApplet(applet);
    }
}

Plasma::Applet::List AbstractGroup::assignedApplets() const
{
    return d->applets;
}

void AbstractGroup::onAppletRemoved(Plasma::Applet *applet)
{
    foreach (Plasma::Applet *ownApplet, d->applets) {
        if (applet == ownApplet) {
            d->applets.removeAll(applet);

            if (d->destroying && (d->applets.count() == 0)) {
                Plasma::Applet::destroy();
                d->destroying = false;
            }
        }
    }
}

void AbstractGroup::destroy()
{
    if (assignedApplets().count() == 0) {
        Plasma::Applet::destroy();
        return;
    }

    d->destroying = true;

    foreach (Plasma::Applet *applet, assignedApplets()) {
        applet->destroy();
    }
}

KConfigGroup AbstractGroup::config() const
{
    KConfigGroup config = KConfigGroup(d->mainConfigGroup(), "Configuration");

    return config;
}

void AbstractGroup::save(KConfigGroup &group) const
{
//     if (!group.isValid()) {
        group = *d->mainConfigGroup();
//     }

    Plasma::Applet::save(group);

    foreach (Plasma::Applet *applet, assignedApplets()) {
        KConfigGroup appletConfig = applet->config().parent();
        KConfigGroup groupConfig(&appletConfig, QString("GroupInformation"));
        groupConfig.writeEntry("Group", id());
        saveAppletLayoutInfo(applet, groupConfig);
    }
}

void AbstractGroup::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    scene()->sendEvent(parentItem(), event);
}

#include "abstractgroup.moc"