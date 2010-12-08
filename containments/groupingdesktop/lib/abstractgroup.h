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

#ifndef ABSTRACTGROUP_H
#define ABSTRACTGROUP_H

#include <QtGui/QGraphicsWidget>
#include <Plasma/Applet>

#include "groupinfo.h"
#include "groupfactory_p.h"
#include "groupingcontainment_export.h"

class KConfigGroup;

class GroupingContainment;
class Handle;
class AbstractGroupPrivate;

/**
 * @class AbstractGroup
 *
 * @short The base Group class
 *
 * AbstractGroup is a base class for special widgets thoughts to contain Plasma::Applet
 */
class PLASMA_GROUPINGCONTAINMENT_EXPORT AbstractGroup : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(bool hasInterface READ hasConfigurationInterface)
    Q_PROPERTY(uint id READ id)
    Q_PROPERTY(GroupType groupType READ groupType)
    Q_PROPERTY(bool isMainGroup READ isMainGroup)

    public:
        /**
         * Defines if the applets inside the group can be freely transformed or not by the user
         */
        enum GroupType {
            ConstrainedGroup = 0,   /**< The transformations of the applet are constrained by,
                                         e.g. a layout */
            FreeGroup = 1           /**< The applets can be freely transformed */
        };
        Q_ENUMS(GroupType)

        /**
         * Description on how draw a background for the group
         */
        enum BackgroundHint {
            NoBackground  = 0,        /**< Don't draw anything */
            StandardBackground = 1,   /**< Draw the translucent background from the theme */
            PlainBackground = 2       /**< Draw a plain, simpler background */
        };
        Q_DECLARE_FLAGS(BackgroundHints, BackgroundHint)

        /**
         * Constructor of the abstract class.
         **/
        explicit AbstractGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);

        /**
         * Default destructor.
         **/
        virtual ~AbstractGroup();

        /**
         * Method called a little after the constructor.
         * If you need methods like config() or containment() use them here,
         * not in the constructor.
         **/
        virtual void init();

        /**
         * Returns the Group that contains this one, 0 if there is no one.
         **/
        AbstractGroup *parentGroup() const;

        /**
         * Adds an Applet to this Group.
         *
         * @param applet the Applet to be managed by this
         * @param layoutApplet the parameter that defines if layoutChild(applet) will be called
         *
         * @see applets
         * @see removeApplet
         **/
        void addApplet(Plasma::Applet *applet, bool layoutApplet = true);

        /**
         * Adds a Group to this Group.
         *
         * @param group, the Group to be managed by this
         * @param layoutGroup the parameter that defines if layoutChild(applet) will be called
         *
         * @see subGroups
         * @see removeSubGroup
         **/
        void addSubGroup(AbstractGroup *group, bool layoutGroup = true);

        /**
         * Removes an Applet from this Group.
         *
         * @param applet the applet to be removed
         * @param newGroup the group in which the Applet must be added
         * after it is removed from this
         *
         * @see addApplet
         * @see applets
         **/
        void removeApplet(Plasma::Applet *applet, AbstractGroup *newGroup = 0);

        /**
         * Removes a Group from this Group.
         *
         * @param group the Group to be removed
         * @param newGroup the Group in which the Group must be added
         * after it is removed from this
         *
         * @see addSubGroup
         * @see subGroups
         **/
        void removeSubGroup(AbstractGroup *group, AbstractGroup *newGroup = 0);

        /**
         * Saves the group's specific configurations for a child.
         * This function must be reimplemented by a sub class.
         *
         * @param child the child whose info will be saved
         * @param config the config group for the configuration
         *
         * @see restoreChildGroupInfo
         **/
        virtual void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup config) const = 0;

        /**
         * Restores the group's specific configurations for a chils.
         * This function must be reimplemented by a sub class.
         *
         * @param child the child whose info will be restored
         * @param group the config group for the configuration
         *
         * @see saveChildGroupInfo
         **/
        virtual void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group) = 0;

        /**
         * Returns the view this widget is visible on, or 0 if none can be found.
         * @warning do NOT assume this will always return a view!
         * a null view probably means that either plasma hasn't finished loading, or your group is
         * on an activity that's not being shown anywhere.
         */
        QGraphicsView *view() const;

        /**
         * Destroy this Groups and its children, deleting the configurations too
         **/
        void destroy();

        /**
         * Returns the KConfigGroup to access the Group configuration.
         **/
        KConfigGroup config() const;

        /**
         * Saves state information about this Group.
         * Normally you don't reimplement this to save the
         * configuration of the Group, but you save it using config().
         *
         * @see saveChildren
         * @see restore
         * @see config
         **/
        virtual void save(KConfigGroup &group) const;

        /**
         * Calls saveChildGroupInfo for every child.
         *
         * @see save
         * @see saveChildGroupInfo
         **/
        virtual void saveChildren() const;

        /**
         * Restore the state information about this Group.
         * Normally you don't reimplement this to restore the configuration
         * of the Group, but you restore it in init() using config().
         *
         * @see restoreChildren
         * @see save
         * @see config
         **/
        virtual void restore(KConfigGroup &group);

        /**
         * Calls restoreChildGroupInfo for every child. When it is called
         * applets(), subGroups() and children() are already available.
         *
         * @see restore
         * @see restoreChildGroupInfo
         **/
        virtual void restoreChildren();

        /**
         * Shows a visual clue for drag and drop
         * The default implementation does nothing, reimplement it in groups that need it
         *
         * @param pos point where to show the drop target; if an invalid point is passed in
         *        the drop zone should not be shown
         *
         * @return true if it can accept a child at the passed position
         **/
        virtual bool showDropZone(const QPointF &pos);

        /**
         * Returns a list of the applets managed by this Group.
         *
         * @see addApplet
         * @see removeApplet
         **/
        Plasma::Applet::List applets() const;

        /**
         * Returns a list of the groups managed by this Group.
         *
         * @see addSubGroup
         * @see removeSubGroup
         **/
        QList<AbstractGroup *> subGroups() const;

        /**
         * Returns a list of all the children managed by this Group.
         *
         * @see applets
         * @see subGroups
         **/
        QList<QGraphicsWidget *> children() const;

        /**
         * Returns the id of this Group.
         **/
        uint id() const;

        /**
         * Returns the type of immutability of this Group.
         *
         * @see setImmutability
         **/
        Plasma::ImmutabilityType immutability() const;

        /**
         * Returns a pointer to the containment this Group is displayed in.
         **/
        GroupingContainment *containment() const;

        /**
         * Returns the plugin name for the Group
         **/
        virtual QString pluginName() const = 0;

        /**
         * Tells this Group it is a Main Group, causing it to:
         *  not paint a background;
         *  not be movable;
         *  not have contents margins.
         *
         * @see isMainGroup
         * @see GroupingContainment::useMainGroup
         **/
        void setIsMainGroup();

        /**
         * Returns true if this Group is a MainGroup.
         *
         * @see setIsMainGroup
         * @see GroupingContainment::useMainGroup
         **/
        bool isMainGroup() const;

        /**
         * Sets the BackgroundHints for this Group.
         *
         * @see backgroundHints
         * @see BackgroundHint
         **/
        void setBackgroundHints(BackgroundHints hint);

        /**
         * Returns the BackgroundHints set for this Group.
         *
         * @see setBackgroundHints
         * @see BackGroundHint
         **/
        BackgroundHints backgroundHints() const;

        /**
         * Returns true if this Group uses a simpler background for its children.
         *
         * @see setUseSimplerBackgroundForChildren
         **/
        bool useSimplerBackgroundForChildren() const;

        /**
         * Reimplemented from QGraphicsItem
         **/
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        /**
         * Reimplemented from QGraphicsItem
         **/
        enum {
            Type = UserType + 2
        };
        virtual int type() const;

        /**
         * Brings this Group on top of the other Groups and Applet
         **/
        void raise();

        /**
         * Returns an handle for a child of this Group.
         * Reimplement this if you want to provide a different handle than
         * the default one.
         *
         * @param child the child for which it must be created an handle
         *
         * @return the handle just created
         **/
        virtual Handle *createHandleForChild(QGraphicsWidget *child);

        /**
         * Returns the type of this Group
         *
         * @see setGroupType
         * @see GroupType
         **/
        GroupType groupType() const;

        /**
         * Reimplement this method to provide a configuration interface,
         * parented to the supplied widget.
         * Ownership of the widgets is passed to the parent widget.
         *
         * @param parent the dialog which is the parent of the configuration widgets
         **/
        virtual void createConfigurationInterface(KConfigDialog *parent);

        /**
         * Returns true if this Group provides a GUI configuration.
         *
         * @see setHasConfigurationInterface
         **/
        bool hasConfigurationInterface() const;

        /**
         * Called when any of the geometry constraints have been updated.
         * This method calls constraintsEvent, which may be reimplemented,
         * once the Group has been prepared for updating the constraints.
         *
         * @param constraints the type of constraints that were updated
         *
         * @see constraintsEvent
         **/
        void updateConstraints(Plasma::Constraints constraints = Plasma::AllConstraints);

        /**
         * Called when an Applet or a Group starts to be moved by the user.
         * A Group might want to reimplement this to free the widget from
         * a layout or whatever, so to allow it to be moved.
         **/
        virtual void releaseChild(QGraphicsWidget *child);

        static QString mimeType();

        /**
         * Creates a new Group.
         *
         * @param name the identifier for the type of Group to be created
         * @param parent the parent of the new Group
         **/
        static AbstractGroup *load(const QString &name, QGraphicsItem *parent = 0);

        /**
         * Returns a list of all identifiers of the available Groups.
         **/
        static QStringList availableGroups();

        static GroupInfo groupInfo(const QString &name);

    public slots:
        /**
         * Sets the immutability type for this Group (not immutable,
         * user immutable or system immutable)
         *
         * @param immutability the new immutability type of this Group
         *
         * @see immutability
         * @see Plasma::ImmutabilityType
         **/
        void setImmutability(Plasma::ImmutabilityType immutability);

        /**
         * Lets the user interact with the Group options.
         * Called when the user selects the configure entry
         * from the context menu.
         *
         * Unless there is a good reason for overriding this method,
         * AbstractGroup subclasses should actually override
         * createConfigurationInterface instead.
         *
         * @see createConfigurationInterface
         * @see setHasConfigurationInterface
         **/
        virtual void showConfigurationInterface();

    protected:
        /**
         * Reimplemented from QGraphicsItem
         **/
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        /**
         * Reimplemented from QGraphicsItem
         **/
        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * Reimplemented from QGraphicsWidget
         **/
        virtual void resizeEvent(QGraphicsSceneResizeEvent *event);

        /**
         * Lays out a child inside the Group
         * A sub class probably wants to reimplement this function
         *
         * @param child the child to be laid out
         * @param pos the position of the child mapped to the Group's coordinates
         **/
        virtual void layoutChild(QGraphicsWidget *child, const QPointF &pos) = 0;

        /**
         * Sets the type of this Group
         *
         * @see groupType
         * @see GroupType
         */
        void setGroupType(GroupType type);

        /**
         * Sets whether or not this Group provides a user interface for
         * configuring it.
         *
         * It defaults to false, and if true is passed in you should
         * also reimplement createConfigurationInterface()
         *
         * @param hasInterface whether or not there is a user interface available
         *
         * @see createConfigurationInterface
         **/
        void setHasConfigurationInterface(bool hasInterface);

        /**
         * Sets whether or not this Group should make its children have
         * a simpler background instead of the default one.
         * Default is false.
         *
         * @param use whether or not it should use a simpler background
         *
         * @see useSimplerBackgroundForChildren
         **/
        void setUseSimplerBackgroundForChildren(bool use);

        /**
         * Called when any of the geometry constraints have been updated.
         *
         * @param constraints the type of constraints that were updated
         **/
        virtual void constraintsEvent(Plasma::Constraints constraints);

    signals:
        /**
         * This signal is emitted when the Group's destructor is called.
         *
         * @param group a pointer to this Group
         **/
        void groupDestroyed(AbstractGroup *group);

        /**
         * Emitted when an Applet is assigned to this Group.
         *
         * @param applet a pointer to the Applet added
         * @param group a pointer to this Group
         *
         * @see subGroupAddedInGroup
         **/
        void appletAddedInGroup(Plasma::Applet *applet, AbstractGroup *group);

        /**
         * Emitted when a Group is assigned to this Group.
         *
         * @param subGroup a pointer to the Group added
         * @param group a pointer to this Group
         *
         * @see appletAddedInGroup
         **/
        void subGroupAddedInGroup(AbstractGroup *subGroup, AbstractGroup *group);

        /**
         * Emitted when an Applet is removed from this Group.
         *
         * @param applet a pointer to the Applet removed
         * @param group a pointer to this Group
         *
         * @see subGroupRemovedFromGroup
         **/
        void appletRemovedFromGroup(Plasma::Applet *applet, AbstractGroup *group);

        /**
         * Emitted when a Group is removed from this Group.
         *
         * @param subGroup a pointer to the Group removed
         * @param group a pointer to this Group
         *
         * @see appletRemovedFromGroup
         **/
        void subGroupRemovedFromGroup(AbstractGroup *subGroup, AbstractGroup *group);

        /**
         * This signal is emitted when the Group's geometry changes.
         **/
        void geometryChanged();

        /**
         * This signal is emitted when the Group is transformed by the user.
         **/
        void groupTransformedByUser();

        /**
         * Emitted when a Group has changed values in its configuration
         * and wishes for them to be saved at the next save point. As this implies
         * disk activity, this signal should be used with care.
         **/
        void configNeedsSaving();

        /**
         * Emitted when the immutability changes.
         *
         * @param immutability the new immutability
         **/
        void immutabilityChanged(Plasma::ImmutabilityType immutability);

    private:
        Q_PRIVATE_SLOT(d, void appletDestroyed(Plasma::Applet *applet))
        Q_PRIVATE_SLOT(d, void subGroupDestroyed(AbstractGroup *subGroup))
        Q_PRIVATE_SLOT(d, void destroyGroup())
        Q_PRIVATE_SLOT(d, void onChildGeometryChanged())

        AbstractGroupPrivate *const d;

        friend class AbstractGroupPrivate;
        friend class Handle;
        friend class GroupingContainment;
        friend class GroupingContainmentPrivate;

};

Q_DECLARE_METATYPE(AbstractGroup *)

/**
 * Register a Group to the factory. Call this macro in the .cpp of your Group outside any function.
 **/
#define REGISTER_GROUP(class) \
    static const bool g_##class = GroupFactory::instance()->registerGroup<class>();

#endif // ABSTRACTGROUP_H
