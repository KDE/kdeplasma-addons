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

class Global;

/**
 * Represents a group of object
 *
 * @author Ivan Cukic
 *
 * This is not a visual grouping of object. For that, see QGraphicsItem,
 * QGraphicsWidget etc. documentation.
 *
 * Lancelot::Group provides a mechanism for grouping objects
 * semantically.
 *
 * For example, if you want to have a couple of buttons that should
 * change their appearances or behaviour simultaneously, you don't
 * need to change the properties for each object, but only to notify
 * the group object.
 *
 * One of the examples where this is heavily used is the theming
 * support in the %Lancelot menu application.
 *
 */
class LANCELOT_EXPORT Group : public QObject {
public:

    /**
     * This class contains the triplet od colors for
     * three standard object states - normal, disabled
     * and active
     */
    class ColorScheme {
    public:
        QColor normal, disabled, active;
    };

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
     * @param property the name of the property
     * @param value value
     * @param persistent whether the property should be preserved on group reload
     */
    void setProperty(const QString & property, const QVariant & value, bool persistent = true);

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
     * Loads the group properties from a configuration file.
     * @full if true, the group is reloaded even if it was already
     *       loaded before
     */
    void load(bool full = false);

    /**
     * Adds object to the group
     * @param object the object to add to the group
     */
    void add(QObject * object);

    /**
     * @returns whether the groeup contains the specified object
     * @param object the object
     */
    bool contains(QObject * object);

    /**
     * Removes the object from the group
     * @param object the object that should be removed from the group
     * @param setDefaultGroup if set, the object is moved to the default
     *     group. If not, the object will retain the pointer to the old
     *     group, but will not receive any updates from it.
     */
    void remove(QObject * object, bool setDefaultGroup = true);

    /**
     * @returns the name of the group
     */
    QString name() const;

    // TODO: delete me after 4.3
    void notifyUpdated() {}

private:
    class Private;
    Private * const d;

    Group(QString name);
    virtual ~Group();

    friend class Global;
};

/**
 * Global object represents one instance of Lancelot-based process
 * inside a main application.
 *
 * @author Ivan Cukic
 *
 * One application can have multiple Lancelot::Globals. For example,
 * when you add a couple of Lancelot applets to the Plasma desktop.
 *
 * Globals provide separate groups of objects, separate themes etc.
 * Also provides some events inhibition (mostly related to geometry
 * processing) to speed up form creation and initialization and avoid
 * unnecessary setGeometry calls.
 */
class LANCELOT_EXPORT Global : public QObject {
    Q_OBJECT
public:
    static Global * instance();

    /**
     * Destroys this Lancelot::Global
     */
    virtual ~Global();

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
    Group * group(const QString & name);

    /**
     * @returns a group to which the object belongs
     * @param object object
     */
    Group * groupForObject(QObject * object);

    /**
     * Sets the group for the object
     */
    void setGroupForObject(QObject * object, Group * group);

    /**
     * @returns whether a group with the specified name exists
     */
    bool groupExists(const QString & name) const;

    /**
     * @returns a pointer to the default group
     */
    Group * defaultGroup();

private:
    /**
     * Creates a new Lancelot::Global
     */
    Global();

    class Private;
    Private * const d;
};

// TODO: delete this class after KDE 4.3
class Instance {
public:
    Instance() {
        Global::instance();
    }

    void setHasApplication(bool value) {
        Q_UNUSED(value);
    }
    void activateAll() {};

    Group * group(const QString name) {
        return Global::instance()->group(name);
    }

};

} // namespace Lancelot

#endif /* LANCELOT_GLOBAL_H */

