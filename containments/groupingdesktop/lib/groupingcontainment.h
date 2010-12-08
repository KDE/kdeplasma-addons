/*
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

#ifndef GROUPINGCONTAINMENT_H
#define GROUPINGCONTAINMENT_H

#include <Plasma/Containment>

#include "groupingcontainment_export.h"

class AbstractGroup;
class GroupingContainmentPrivate;

/**
 * @class GroupingContainment
 *
 * @short The base Containment class
 *
 * GroupingContainment manages all the Groups and must be subclassed to create a new
 * Containment with grouping support.
 */
class PLASMA_GROUPINGCONTAINMENT_EXPORT GroupingContainment: public Plasma::Containment
{
    Q_OBJECT
    Q_PROPERTY(AbstractGroup *mainGroup READ mainGroup WRITE setMainGroup)

    public:
        /**
         * Default constructor.
         **/
        GroupingContainment(QObject *parent, const QVariantList &args);

        /**
         * Default destructor.
         **/
        virtual ~GroupingContainment();

        /**
         * Reimplemented from Plasma::Applet
         **/
        void init();

        /**
         * Reimplemented from Plasma::Applet
         **/
        QList<QAction *> contextualActions();

        /**
         * Creates a new Group and it adds it to this Containment.
         *
         * @param plugin the name of the type of Group
         * @param pos the position where it should be created
         * @param id the identifier for the Group. Passing 0 will set a unique one
         *
         * @return the newly created Group
         *
         * @see addGroup
         * @see groups
         **/
        AbstractGroup *addGroup(const QString &plugin, const QPointF &pos = QPointF(0, 0), int id = 0);

        /**
         * Adds a new Group to this Containment.
         *
         * @param group the Group to be added
         * @param pos the position where it should be added
         *
         * @see addGroup
         * @see groups
         **/
        void addGroup(AbstractGroup *group, const QPointF &pos);

        /**
         * Returns a list of all the Groups contained in this Containment
         *
         * @see addGroup
         **/
        QList<AbstractGroup *> groups() const;

        /**
         * Call this function when an Applet or a Group is being moved by the user.
         * It will be monitored to see if it is placed in a Group.
         * Typically this method will be called by an handle.
         *
         * @param widget the Applet or Group to be monitored
         **/
        void setMovingWidget(QGraphicsWidget *widget);

        /**
         * Returns the widget that is currently being moved by the user, or 0
         * if there is not any.
         **/
        QGraphicsWidget *movingWidget() const;

        /**
         * Raises a widget above all the other Applets or Groups.
         **/
        void raise(QGraphicsWidget *widget);

    protected Q_SLOTS:
        /**
         * Lays out the Main Group.
         * The default implementation makes it as big as the Containment.
         * You may want to reimplement this in subclasses if you want other behaviours.
         **/
        virtual void layoutMainGroup();

    protected:
        /**
         * Reimplemented from Plasma::Applet
         **/
        virtual void constraintsEvent(Plasma::Constraints constraints);

        /**
         * Reimplemented from QObject
         **/
        virtual bool eventFilter(QObject *obj, QEvent *event);

        /**
         * Reimplemented from QGraphicsItem
         **/
        virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

        /**
         * Reimplemented from QGraphicsItem
         **/
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

        /**
         * Reimplemented from QGraphicsItem
         **/
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * Reimplemented from Plasma::Containment
         **/
        virtual void saveContents(KConfigGroup &group) const;

        /**
         * Reimplemented from Plasma::Containment
         **/
        virtual void restoreContents(KConfigGroup &group);

        /**
         * Tells the Containment to use a Main Group of the given type.
         * It will overlap to the Containment, so it will be the root of
         * the Groups hierarchy.
         *
         * @warning every sub class of this MUST set a Main Group, or it will
         * encounter breackages. GroupingContainment sets by default a FloatingGroup.
         *
         * @param type the type of the wanted Main Group
         *
         * @see setMainGroup
         * @see mainGroup
         * @see AbstractGroup::isMainGroup
         **/
        void useMainGroup(const QString &type);

        /**
         * Sets a Main Group for this Containment.
         * Normally you will use useMainGroup instead of this.
         *
         * @param group the group to be set Main Group
         *
         * @see useMainGroup
         * @see mainGroup
         * @see AbstractGroup::isMainGroup
         **/
        void setMainGroup(AbstractGroup *group);

        /**
         * Returns the Main Group of this Containment.
         *
         * @see useMainGroup
         * @see AbstractGroup::isMainGroup
         **/
        AbstractGroup *mainGroup() const;

    signals:
        /**
         * Emitted when a new Group is added to this Containment.
         *
         * @param group the newly added Group
         * @param pos the position where it has been added
         *
         * @see groupRemoved
         **/
        void groupAdded(AbstractGroup *group, const QPointF &pos);

        /**
         * Emitted when a Group is removed from this Containment.
         *
         * @param group the group which was removed
         *
         * @see groupAdded
         **/
        void groupRemoved(AbstractGroup *group);

    private:
        Q_PRIVATE_SLOT(d, void handleDisappeared(Handle *handle))
        Q_PRIVATE_SLOT(d, void onGroupRemoved(AbstractGroup *group))
        Q_PRIVATE_SLOT(d, void onAppletRemoved(Plasma::Applet *applet))
        Q_PRIVATE_SLOT(d, void manageApplet(Plasma::Applet *applet, const QPointF &pos))
        Q_PRIVATE_SLOT(d, void newGroupClicked())
        Q_PRIVATE_SLOT(d, void deleteGroup())
        Q_PRIVATE_SLOT(d, void configureGroup())
        Q_PRIVATE_SLOT(d, void onAppletRemovedFromGroup(Plasma::Applet *applet, AbstractGroup *group))
        Q_PRIVATE_SLOT(d, void onSubGroupRemovedFromGroup(AbstractGroup *subGroup, AbstractGroup *group))
        Q_PRIVATE_SLOT(d, void onWidgetMoved(QGraphicsWidget *widget))
        Q_PRIVATE_SLOT(d, void onImmutabilityChanged(Plasma::ImmutabilityType immutability))
        Q_PRIVATE_SLOT(d, void restoreGroups())
        Q_PRIVATE_SLOT(d, void prepareWidgetToMove())
        Q_PRIVATE_SLOT(d, void widgetMovedAnimationComplete())

        GroupingContainmentPrivate *const d;

        friend class GroupingContainmentPrivate;
};

#endif // GROUPINGCONTAINMENT_H
