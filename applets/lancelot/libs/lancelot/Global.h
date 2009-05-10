/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_GLOBAL_H
#define LANCELOT_GLOBAL_H

#include <QColor>
#include <QString>
#include <QMap>
#include <QList>
#include <QVariant>
#include <KConfig>
#include <KConfigGroup>
#include <plasma/framesvg.h>

#include <lancelot/lancelot_export.h>

namespace Lancelot
{

class Widget;
class Instance;

/**
 * Represents a group of widgets
 *
 * @author Ivan Cukic
 *
 * This is not a visual grouping of widgets. For that, see QGraphicsItem,
 * QGraphicsWidget etc. documentation.
 *
 * Lancelot::WidgetGroup provides a mechanism for grouping widgets
 * semantically.
 *
 * For example, if you want to have a couple of buttons that should
 * change their appearances or behaviour simultaneously, you don't
 * need to change the properties for each widget, but only to notify
 * the group object.
 *
 * One of the examples where this is heavily used is the theming
 * support in the %Lancelot menu application.
 *
 */
class LANCELOT_EXPORT WidgetGroup : public QObject {
public:

    /**
     * This class contains the triplet od colors for
     * three standard widget states - normal, disabled
     * and active
     */
    class ColorScheme {
    public:
        QColor normal, disabled, active;
    };

    /**
     * @param property the ID of the property
     * @returns whether this group has the specified property set
     */
    bool hasProperty(int property) const;

    /**
     * @param property the ID of the property
     * @returns the value of the specified property
     */
    QVariant property(int property) const;

    /**
     * Sets the value of the specified property
     * @param property the ID of the property
     * @param value value to assign to the property
     */
    void setProperty(int property, const QVariant & value);

    /**
     * Clears the value of the specified property
     * @param property the ID of the property
     */
    void clearProperty(int property);

    /**
     * @param the name of the property
     * @returns whether this group has the specified property set
     */
    bool hasProperty(const QString & property) const;

    /**
     * @param the name of the property
     * @returns the value of the specified property
     */
    QVariant property(const QString & property) const;

    /**
     * Sets the value of the specified property
     * @param the name of the property
     */
    void setProperty(const QString & property, const QVariant & value);

    /**
     * Clears the value of the specified property
     * @param the name of the property
     */
    void clearProperty(const QString & property);

    /**
     * Background SVG image is one of the common properties, so
     * a direct function that accesses it is provided
     * @returns the pointer to Plasma::FrameSvg object
     */
    Plasma::FrameSvg * backgroundSvg() const;

    /**
     * Background color is one of the common properties, so a direct
     * function that accesses it is provided
     * @returns the background color scheme
     */
    const ColorScheme * backgroundColor() const;

    /**
     * Foreground color is one of the common properties, so a direct
     * function that accesses it is provided
     * @returns the background color scheme
     */
    const ColorScheme * foregroundColor() const;

    /**
     * Notifies the widgets that belong to this group that the group
     * is updated.
     */
    void notifyUpdated();

    /**
     * Loads the group properties from a configuration file.
     * @full if true, the group is reloaded even if it was already
     *       loaded before
     */
    void load(bool full = false);

    /**
     * Adds widget to the group
     * @param widget the widget to add to the group
     */
    void addWidget(Widget * widget);

    /**
     * Removes the widget from the group
     * @param widget the widget that should be removed from the group
     * @param setDefaultGroup if set, the widget is moved to the default
     *     group. If not, the widget will retain the pointer to the old
     *     group, but will not receive any updates from it.
     */
    void removeWidget(Widget * widget, bool setDefaultGroup = true);

    /**
     * @returns the name of the group
     */
    QString name() const;

    /**
     * @returns a pointer to the Lancelot::Instance to which this group
     *     belongs.
     */
    Instance * instance();

private:
    class Private;
    Private * const d;

    WidgetGroup(Instance * instance, QString name);
    virtual ~WidgetGroup();

    friend class Instance;
};

/**
 * Instance object represents one instance of Lancelot-based process
 * inside a main application.
 *
 * @author Ivan Cukic
 *
 * One application can have multiple Lancelot::Instances. For example,
 * when you add a couple of Lancelot applets to the Plasma desktop.
 *
 * Instances provide separate groups of widgets, separate themes etc.
 * Also provides some events inhibition (mostly related to geometry
 * processing) to speed up form creation and initialization and avoid
 * unnecessary setGeometry calls.
 */
class LANCELOT_EXPORT Instance : public QObject {
    Q_OBJECT
public:
    /**
     * Creates a new Lancelot::Instance
     */
    Instance();

    /**
     * Destroys this Lancelot::Instance
     */
    virtual ~Instance();

    /**
     * @returns whether the instance is activated
     */
    bool isActivated();

public Q_SLOTS:
    /**
     * Removes the event processing inhibitions and activates
     * all widgets that belong to this instance.
     */
    void activateAll();

    /**
     * Activates the event processing inhibitions and deactivates
     * all widgets that belong to this instance.
     */
    void deactivateAll();

public:
    /**
     * Adds a widget to instance
     */
    void addWidget(Widget * widget);

    /**
     * Removes a widget from instance
     */
    void removeWidget(Widget * widget);

    /**
     * @returns a pointer to the theme configuration file
     */
    KConfig * theme();

    /**
     * @returns a pointer to the configuration file
     */
    KConfig * config();

    /**
     * @param name group name
     * @returns a pointer to the group with specified name
     */
    WidgetGroup * group(const QString & name);

    /**
     * @returns a pointer to the default group
     */
    WidgetGroup * defaultGroup();

    /**
     * Active instance is the instance to which all currently created
     * widgets are inserted into.
     * \note
     *   Note that only one instance can be active at a time
     * @returns a pointer to the active instance
     */
    static Instance * activeInstance();

    /**
     * Sets the active instance and locks the mutex for it
     * @param instance new active instance
     */
    static void setActiveInstanceAndLock(Instance * instance);

    /**
     * Releases the lock on the active instance
     */
    static void releaseActiveInstanceLock();

    // TODO: Replace the following finctions, and move them into the
    // Instance itself.
    /**
     * @returns whether instance is in application
     */
    static bool hasApplication();

    /**
     * Sets whether instance is in application
     */
    static void setHasApplication(bool value);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_GLOBAL_H */

