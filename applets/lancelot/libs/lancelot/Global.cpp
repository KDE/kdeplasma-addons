/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "Global.h"
#include "Global_p.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <QMutex>

#include <KGlobal>
#include <KStandardDirs>

#include <plasma/theme.h>

namespace Lancelot
{

// Group
Group::ColorScheme::ColorScheme()
    : normal(QColor()), disabled(QColor()), active(QColor())
{
}

Group::Private::Private()
    : name(QString()), backgroundSvg(NULL),
      ownsBackgroundSvg(false), loaded(false)
       // TODO : Add caching?
       //cachedBackgroundNormal(NULL), cachedBackgroundActive(NULL), cachedBackgroundDisabled(NULL)
{
}


Group::Private::~Private()
{
    if (ownsBackgroundSvg) {
        delete backgroundSvg;
    }
    //delete d->cachedBackgroundNormal;
    //delete d->cachedBackgroundActive;
    //delete d->cachedBackgroundDisabled;
}

void Group::Private::setObjectProperty(QObject * object,
        const QString & property, const QVariant & value)
{
    object->setProperty(property.toAscii(), value);
}

// clearing all info
void Group::Private::reset()
{
    QMutableMapIterator < QString, QVariant > i(properties);
    while (i.hasNext()) {
        i.next();
        if (!persistentProperties.contains(i.key())) {
            qDebug() << "Group::load: reloading property:" << i.key();
            i.remove();
        }
    }

    if (ownsBackgroundSvg) {
        delete backgroundSvg;
    }
    backgroundSvg = NULL;

    foregroundColor = Group::ColorScheme();
    backgroundColor = Group::ColorScheme();

}

void Group::Private::copyFrom(Group::Private * d)
{
    if (this == d) return;

    properties = d->properties;
    persistentProperties = d->persistentProperties;

    foregroundColor = d->foregroundColor;
    backgroundColor = d->backgroundColor;

    if (ownsBackgroundSvg) {
        delete backgroundSvg;
    }
    backgroundSvg = d->backgroundSvg;
    ownsBackgroundSvg = false;
}

KConfigGroup Group::Private::confGroupTheme()
{
    return KConfigGroup(Global::self()->theme(), "Group-" + name);
}

Group::Group(QString name)
    : d(new Private())
{
    d->name = name;
}

Group::~Group()
{
    delete d;
}

void Group::add(QObject * object)
{
    if (!object) return;

    if (d->objects.contains(object)) return;
    d->objects << object;

    QMapIterator < QString, QVariant > i(d->properties);
    while (i.hasNext()) {
        i.next();
        d->setObjectProperty(object,
            i.key(), i.value());
    }
}

void Group::remove(QObject * object, bool setDefaultGroup)
{
    if (Global::self()->defaultGroup() == this && setDefaultGroup) return;

    if (!d->objects.contains(object)) return;
    d->objects.remove(object);

    if (setDefaultGroup) {
        // object->setGroup(NULL);
    }
}

bool Group::hasProperty(const QString & property) const
{
    // TODO: remove this after 4.3
    // QString property = prop;
    // property[0] = property[0].toLower();

    return d->properties.contains(property);
}

QVariant Group::property(const QString & property) const
{
    // TODO: remove this after 4.3
    // QString property = prop;
    // property[0] = property[0].toLower();

    return d->properties.value(property);
}

void Group::setProperty(const QString & property, const QVariant & value, bool persistent)
{
    // TODO: remove this after 4.3
    // QString property = prop;
    // property[0] = property[0].toLower();

    qDebug() << "Group::setProperty:" << property << value;

    d->properties[property] = value;

    if (persistent) {
        d->persistentProperties << property;
    }

    foreach (QObject * child, d->objects) {
        d->setObjectProperty(child, property, value);
    }
}

void Group::clearProperty(const QString & property)
{
    // TODO: remove this after 4.3
    // QString property = prop;
    // property[0] = property[0].toLower();

    d->properties.remove(property);
}

QString Group::name() const
{
    return d->name;
}

Plasma::FrameSvg * Group::backgroundSvg() const
{
    return d->backgroundSvg;
}

const Group::ColorScheme * Group::backgroundColor() const
{
    if (!hasProperty("WholeColorBackground") &&
        !hasProperty("TextColorBackground")) {
        return NULL;
    }
    return & d->backgroundColor;
}

const Group::ColorScheme * Group::foregroundColor() const
{
    return & d->foregroundColor;
}

void Group::load(bool full)
{
    if (d->loaded && !full) return;
    d->loaded = true;

    qDebug() << "Group::load: loading group:" << name();

    // d->properties.clear();
    d->reset();

    Group * group;

    qDebug() << "Loading group " << d->name;
    KConfigGroup confGroupTheme = d->confGroupTheme();
    if (!confGroupTheme.exists()) {
        group = Global::self()->defaultGroup();
        if (group == this) return;

        d->copyFrom(group->d);
        return;
    }

    QString parentName = confGroupTheme.readEntry("parent", "Default");
    if (Global::self()->groupExists(parentName)) {
        group = Global::self()->group(confGroupTheme.readEntry("parent", "Default"));
        if (group != this) {
            group->load(false);
            d->copyFrom(group->d);
        }
    }

    // Load properties from theme configuration file
    d->foregroundColor.normal   = confGroupTheme.readEntry("foreground.color.normal",   d->foregroundColor.normal);
    d->foregroundColor.active   = confGroupTheme.readEntry("foreground.color.active",   d->foregroundColor.active);
    d->foregroundColor.disabled = confGroupTheme.readEntry("foreground.color.disabled", d->foregroundColor.disabled);

    QString type = confGroupTheme.readEntry("background.type", "none");
    if (type == "color" || type == "color-compact") {
        qDebug() << "Group::load: loading color background";
        if (type == "color") {
            setProperty("WholeColorBackground", 1, false);
        } else {
            setProperty("TextColorBackground", 1, false);
        }
        d->backgroundColor.normal   = confGroupTheme.readEntry("background.color.normal",   d->backgroundColor.normal);
        d->backgroundColor.active   = confGroupTheme.readEntry("background.color.active",   d->backgroundColor.active);
        d->backgroundColor.disabled = confGroupTheme.readEntry("background.color.disabled", d->backgroundColor.disabled);
    } else if (type == "svg") {
        qDebug() << "Group::load: loading svg background";
        // we have already deleted the backgroundSvg
        // if (d->ownsBackgroundSvg) {
        //     delete d->backgroundSvg;
        // }

        d->backgroundSvg = new Plasma::FrameSvg(NULL);
        QString imagePath = Plasma::Theme::defaultTheme()->imagePath(
                confGroupTheme.readEntry("background.svg"));

        d->backgroundSvg->setImagePath(imagePath);
        qDebug() << "Background is: " <<
            d->backgroundSvg->imagePath();
        d->backgroundSvg->setCacheAllRenderedFrames(true);
        d->ownsBackgroundSvg = true;

        if (!d->backgroundSvg->isValid()) {
            qDebug() << "Background is not valid: " <<
                d->backgroundSvg->imagePath();
            delete d->backgroundSvg;
            d->backgroundSvg = NULL;
            d->ownsBackgroundSvg = false;
        } else {
            setProperty("SvgBackground", 1, false);
        }
    }

    if (!confGroupTheme.readEntry(
                "foreground.blurtextshadow", QString()).isEmpty()) {
        setProperty("BlurTextShadow", 1, false);
    }
}

bool Group::contains(QObject * object)
{
    return d->objects.contains(object);
}


// Global

Global * Global::Private::instance = NULL;

Global * Global::self()
{
    if (!Global::Private::instance) {
        Global::Private::instance = new Global();
    }
    return Global::Private::instance;
}

Global::Private::Private()
      : confMain(NULL),
        confTheme(NULL)
{
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(themeChanged()));
}

Global::Private::~Private()
{
}

void Global::Private::objectDeleted(QObject * object)
{
    Group * group = Global::self()->groupForObject(object);
    if (group) {
        group->remove(object);
    }
}

void Global::Private::createConfTheme()
{
    QString app = KGlobal::mainComponent().componentName();
    if (app == "lancelot") {
        app = "";
    } else {
        app += '-';
    }

    delete confTheme;
    QString search = "desktoptheme/" +
            Plasma::Theme::defaultTheme()->themeName()
            + "/lancelot/" + app + "theme.config";
    QString path =  KStandardDirs::locate( "data", search );
    qDebug() << "Global::Private::createConfTheme: path:" << search << "=" << path;

    // if we didn't find the theme specific for this application
    // we'll use the main theme file
    if (path.isEmpty()) {
        search = "desktoptheme/" +
            Plasma::Theme::defaultTheme()->themeName()
            + "/lancelot/theme.config";
        path =  KStandardDirs::locate( "data", search );
        qDebug() << "Global::Private::createConfTheme: path:" << search << "=" << path;
    }

    // if the above fails, we are loading the default theme's
    // file
    if (path.isEmpty()) {
        search = "desktoptheme/default/lancelot/theme.config";
        path =  KStandardDirs::locate( "data", search );
        qDebug() << "Global::Private::createConfTheme: path:" << search << "=" << path;
    }

    // this doesn't really do anything useful
    // TODO: remove later
    if (path.isEmpty()) {
        path = "lancelotrc";
    }

    confTheme = new KConfig(path);
}

void Global::Private::themeChanged()
{
    qDebug() << "Global::Private::themeChanged";
    createConfTheme();
    loadAllGroups(true);
}

void Global::Private::loadAllGroups(bool clearFirst)
{
    foreach(Group * group, groups) {
        group->d->reset();
    }

    foreach(Group * group, groups) {
        group->load(true);
    }
}

Global::Global()
  : d(new Private())
{
    d->confMain = new KConfig("lancelotrc");

    Plasma::Theme::defaultTheme()->setUseGlobalSettings(true);
    d->createConfTheme();

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            d, SLOT(themeChanged()));
}

Global::~Global()
{
    foreach (Group * group, d->groups) {
        delete group;
    }
    delete d->confMain;
    delete d->confTheme;

    delete d;
}

KConfig * Global::theme()
{
    return d->confTheme;
}

KConfig * Global::config()
{
    return d->confMain;
}

bool Global::groupExists(const QString & name) const
{
    QString groupName = name;
    if (groupName.isEmpty()) {
        groupName = "Default";
    }

    return (!d->groups.contains(groupName));
}

Group * Global::group(const QString & name)
{
    QString groupName = name;
    if (groupName.isEmpty()) {
        groupName = "Default";
    }

    if (!d->groups.contains(groupName)) {
        Group * group = new Group(groupName);
        d->groups.insert(groupName, group);
        group->load();
    }

    return d->groups[groupName];
}

Group * Global::defaultGroup()
{
    return group("Default");
}

Group * Global::groupForObject(QObject * object)
{
    foreach (Group * group, d->groups) {
        if (group->contains(object)) {
            return group;
        }
    }
    return NULL;
}

void Global::setGroupForObject(QObject * object, Group * group) {
    Group * oldGroup = groupForObject(object);
    if (oldGroup == group) {
        return;
    }

    if (oldGroup) {
        oldGroup->remove(object);
    }

    if (group) {
        group->add(object);
        connect(object, SIGNAL(destroyed(QObject *)),
                d, SLOT(objectDeleted(QObject *)));
    }
}

void Global::setImmutability(const Plasma::ImmutabilityType immutable)
{
    if (d->immutability == immutable) {
        return;
    }

    d->immutability = immutable;
    qDebug() << "Global::setImmutability " << immutable;
    emit immutabilityChanged(immutable);
}

Plasma::ImmutabilityType Global::immutability() const
{
    return d->immutability;
}


} // namespace Lancelot

