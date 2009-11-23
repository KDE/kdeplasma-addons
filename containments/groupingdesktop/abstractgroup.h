/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@kde.org>
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

#ifndef ABSTRACTGROUP_H
#define ABSTRACTGROUP_H

#include <QtGui/QGraphicsWidget>
#include <Plasma/Applet>

class KConfigGroup;

class Containment;
class AbstractGroupPrivate;

class AbstractGroup : public QGraphicsWidget
{
    Q_OBJECT
    public:
        /**
        * Constructor of the abstract class.
        **/
        AbstractGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);

        /**
        * Default destructor
        **/
        virtual ~AbstractGroup();

        /**
        * Assignes an Applet to this Group
        * @param applet the applet to be managed by this
        * @param layoutApplets if true calls layoutApplet(applet)
        **/
        void assignApplet(Plasma::Applet *applet, bool layoutApplets = true);

        /**
        * Saves the group's specific configurations for an applet.
        * This function must be reimplemented by a child class.
        * @param applet the applet which will be saved
        * @param group the config group for the configuration
        **/
        virtual void saveAppletLayoutInfo(Plasma::Applet *applet, KConfigGroup group) const = 0;

        /**
        * Restores the group's specific configurations for an applet.
        * This function must be reimplemented by a child class.
        * @param applet the applet which will be restored
        * @param group the config group for the configuration
        **/
        virtual void restoreAppletLayoutInfo(Plasma::Applet *applet, const KConfigGroup &group) = 0;

        QGraphicsView *view() const;

        /**
        * Destroyed this groups and its applet, deleting the configurations too
        **/
        void destroy();

        /**
        * Reimplemented from Plasma::Applet
        **/
        KConfigGroup config() const;

        /**
        * Reimplemented from Plasma::Applet
        **/
        void save(KConfigGroup &group) const;

        /**
        * Used to have a list of the applets managed by this group
        * @return the list of the applets
        **/
        Plasma::Applet::List assignedApplets() const;

        void setContainment(Plasma::Containment *containment);
        Plasma::Containment *containment() const;
        void setId(unsigned int id);
        unsigned int id() const;
        Plasma::ImmutabilityType immutability() const;
        virtual QString pluginName() const = 0;

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    public slots:
        void setImmutability(Plasma::ImmutabilityType immutability);

    protected:
        /**
        * Reimplemented from QGraphicsItem
        **/
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event);

        /**
        * Reimplemented from QGraphicsItem
        **/
        virtual void resizeEvent(QGraphicsSceneResizeEvent *event);

        /**
        * Lay outs an applet inside the group
        * This function must be reimplemented by a child class.
        * @param applet the applet to be layed out
        **/
        virtual void layoutApplet(Plasma::Applet *applet) = 0;

    signals:
        void groupDestroyed(AbstractGroup *group);
        void appletAddedInGroup(Plasma::Applet *applet, AbstractGroup *group);
        void appletRemovedFromGroup(Plasma::Applet *applet, AbstractGroup *group);
        void geometryChanged();
        void groupTransformedByUser();

    private:
        Q_PRIVATE_SLOT(d, void appletDestroyed(Plasma::Applet *applet))

        AbstractGroupPrivate *const d;

        friend class AbstractGroupPrivate;
        friend class GroupHandle;
};

#endif // ABSTRACTGROUP_H
