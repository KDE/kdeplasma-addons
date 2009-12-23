/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

class GroupingContainment;
class AbstractGroupPrivate;

/**
 * @class AbstractGroup
 *
 * @short The base Group class
 *
 * AbstractGroup is a base class for special widgets thoughts to contain Plasma::Applet
 */
class PLASMA_EXPORT AbstractGroup : public QGraphicsWidget
{
    Q_OBJECT
    public:
        /**
         * Defines if the applets inside the group can be freely transformed or not by the user
         */
        enum GroupType {
            ConstrainedGroup = 0,   /**< The transformations of the applet are constrained by,
                                         e.g. a layout */
            FreeGroup = 1           /**< The applets can be freely transformed */
        };

        /**
         * Constructor of the abstract class.
         **/
        AbstractGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);

        /**
         * Default destructor
         **/
        virtual ~AbstractGroup();

        /**
         * Adds an Applet to this Group
         * @param applet the applet to be managed by this
         * @param layoutApplets if true calls layoutApplet(applet)
         * @see applets
         * @see removeApplet
         **/
        void addApplet(Plasma::Applet *applet, bool layoutApplet = true);

        void addSubGroup(AbstractGroup *group, bool layoutGroup = true);

        /**
         * Removes an applet from this group.
         * @param applet the applet to be removed
         * @see addApplet
         * @see applets
         **/
        void removeApplet(Plasma::Applet *applet, AbstractGroup *newGroup = 0);

        void removeSubGroup(AbstractGroup *group, AbstractGroup *newGroup = 0);

        /**
         * Saves the group's specific configurations for an applet.
         * This function must be reimplemented by a child class.
         * @param applet the applet which will be saved
         * @param group the config group for the configuration
         * @see restoreChildGroupInfo
         **/
        virtual void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup config) const = 0;

        /**
         * Restores the group's specific configurations for an applet.
         * This function must be reimplemented by a child class.
         * @param applet the applet which will be restored
         * @param group the config group for the configuration
         * @see saveChildGroupInfo
         **/
        virtual void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group) = 0;

        /**
         * Returns the view this widget is visible on, or 0 if none can be found.
         * @warning do NOT assume this will always return a view!
         * a null view probably means that either plasma isn't finished loading, or your group is
         * on an activity that's not being shown anywhere.
         */
        QGraphicsView *view() const;

        /**
         * Destroyed this groups and its applet, deleting the configurations too
         **/
        void destroy();

        /**
         * Returns the KConfigGroup to access the group configuration.
         **/
        KConfigGroup config() const;

        /**
         * Saves state information about this group.
         **/
        void save(KConfigGroup &group) const;

        /**
         * Shows a visual clue for drag and drop
         * The default implementation does nothing,
         * reimplement in groups that need it
         *
         * @param pos point where to show the drop target; if an invalid point is passed in
         *        the drop zone should not be shown
         */
        virtual void showDropZone(const QPointF &pos);

        /**
         * Returns a list of the applets managed by this group
         * @see addApplet
         * @see removeApplet
         **/
        Plasma::Applet::List applets() const;

        QList<AbstractGroup *> subGroups() const;

        QList<QGraphicsWidget *> children() const;

        /**
         * Returns the id of this group
         **/
        uint id() const;

        /**
         * Returns the type of immutability of this group
         * @see setImmutability
         **/
        Plasma::ImmutabilityType immutability() const;

        /**
         * Returns a pointer to the containment this group is displayed in.
         **/
        GroupingContainment *containment() const;

        /**
         * Returns the plugin name for the group
         **/
        virtual QString pluginName() const = 0;

        void setIsMainGroup(bool isMainGroup);

        bool isMainGroup() const;

        /**
         * Reimplemented from QGraphicsWidget
         **/
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        enum {
            Type = UserType + 2
        };
        virtual int type() const;

    public slots:
        /**
         * Sets the immutability type for this group (not immutable,
         * user immutable or system immutable)
         * @param immutable the new immutability type of this applet
         * @see immutability
         * @see Plasma::ImmutabilityType
         */
        void setImmutability(Plasma::ImmutabilityType immutability);

    protected:
        /**
         * Reimplemented from QGraphicsWidget
         **/
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);

        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);

        /**
         * Reimplemented from QGraphicsWidget
         **/
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * Reimplemented from QGraphicsWidget
         **/
        virtual void resizeEvent(QGraphicsSceneResizeEvent *event);

        /**
         * Reimplemented from QGraphicsWidget
         **/
        virtual bool eventFilter(QObject *obj, QEvent *event);

        /**
         * Lay outs a child inside the group
         * A child group probably wants to reimplement this function
         * @param child the child to be layed out
         * @param pos the position of the applet mapped to the group's coordinates
         **/
        virtual void layoutChild(QGraphicsWidget *child, const QPointF &pos) = 0;

        /**
         * Sets the type of this group
         * @see groupType
         * @see GroupType
         */
        void setGroupType(GroupType type);

        /**
         * @return the type of this group
         * @see setGroupType
         * @see GroupType
         */
        GroupType groupType() const;

    signals:
        /**
         * This signal is emitted when the group's destructor is called.
         * @param group a pointer to the group
         **/
        void groupDestroyed(AbstractGroup *group);

        /**
         * Emitted when an applet is assigned to this group.
         * @param applet a pointer to the applet added
         * @param group a pointer to this group
         **/
        void appletAddedInGroup(Plasma::Applet *applet, AbstractGroup *group);

        void subGroupAddedInGroup(AbstractGroup *subGroup, AbstractGroup *group);

        /**
         * Emitted when an applet is removed from this group.
         * @param applet a pointer to the applet removed
         * @param group a pointer to this group
         **/
        void appletRemovedFromGroup(Plasma::Applet *applet, AbstractGroup *group);

        void subGroupRemovedFromGroup(AbstractGroup *subGroup, AbstractGroup *group);

        /**
         * This signal is emitted when the group's geometry changes.
         **/
        void geometryChanged();

        /**
         * This signal is emitted when the group is transformed by the user.
         **/
        void groupTransformedByUser();

        void configNeedsSaving();

    private:
        Q_PRIVATE_SLOT(d, void appletDestroyed(Plasma::Applet *applet))
        Q_PRIVATE_SLOT(d, void subGroupDestroyed(AbstractGroup *subGroup))
        Q_PRIVATE_SLOT(d, void callLayoutChild())
        Q_PRIVATE_SLOT(d, void repositionRemovedChild())

        AbstractGroupPrivate *const d;

        friend class AbstractGroupPrivate;
        friend class GroupHandle;
        friend class GroupingContainment;
        friend class GroupingContainmentPrivate;
};

#endif // ABSTRACTGROUP_H
