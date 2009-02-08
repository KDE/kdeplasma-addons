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

#include "Global.h"
#include "widgets/Widget.h"
#include <KGlobal>
#include <plasma/theme.h>
#include <KStandardDirs>
#include <QMutex>

namespace Lancelot
{

// Group
class WidgetGroup::Private {
public:
    Private()
      : confGroupTheme(NULL), instance(NULL), name(QString()), backgroundSvg(NULL),
       hasBackgroundColor(false), ownsBackgroundSvg(false), loaded(false)
       // TODO : Add caching?
       //cachedBackgroundNormal(NULL), cachedBackgroundActive(NULL), cachedBackgroundDisabled(NULL)
    {}

    ~Private()
    {
        delete confGroupTheme;
        if (ownsBackgroundSvg) {
            delete backgroundSvg;
        }
        //delete d->cachedBackgroundNormal;
        //delete d->cachedBackgroundActive;
        //delete d->cachedBackgroundDisabled;
    }

    KConfigGroup * confGroupTheme;

    Instance * instance;

    QString name;
    QMap < QString, QVariant > properties;

    QList < Widget * > widgets;

    ColorScheme foregroundColor;
    ColorScheme backgroundColor;
    Plasma::FrameSvg * backgroundSvg;

    bool hasBackgroundColor : 1;
    bool ownsBackgroundSvg : 1;
    bool loaded : 1;

    void copyFrom(WidgetGroup::Private * d)
    {
        if (this == d) return;

        properties = d->properties;

        foregroundColor = d->foregroundColor;
        hasBackgroundColor = d->hasBackgroundColor;
        backgroundColor = d->backgroundColor;

        if (ownsBackgroundSvg) {
            delete backgroundSvg;
        }
        backgroundSvg = d->backgroundSvg;
        ownsBackgroundSvg = false;
    }
};

WidgetGroup::WidgetGroup(Instance * instance, QString name)
    : d(new Private())
{
    d->instance = instance;
    d->name = name;
    d->confGroupTheme = new KConfigGroup(d->instance->theme(), "Group-" + name);
    qDebug() << "Creating group: " << name <<
        d->confGroupTheme->isValid();
}

WidgetGroup::~WidgetGroup()
{
    delete d;
}

Instance * WidgetGroup::instance()
{
    return d->instance;
}

void WidgetGroup::addWidget(Widget * widget)
{
    if (!widget) return;

    if (d->widgets.contains(widget)) return;
    d->widgets.append(widget);

}

void WidgetGroup::removeWidget(Widget * widget, bool setDefaultGroup)
{
    if (d->instance->defaultGroup() == this && setDefaultGroup) return;

    if (!d->widgets.contains(widget)) return;
    d->widgets.removeAll(widget);

    if (setDefaultGroup) {
        widget->setGroup(NULL);
    }
}

bool WidgetGroup::hasProperty(const QString & property) const
{
    return d->properties.contains(property);
}

QVariant WidgetGroup::property(const QString & property) const
{
    return d->properties.value(property);
}

void WidgetGroup::setProperty(const QString & property, const QVariant & value)
{
    d->properties[property] = value;
}

void WidgetGroup::clearProperty(const QString & property)
{
    d->properties.remove(property);
}

QString WidgetGroup::name() const
{
    return d->name;
}

Plasma::FrameSvg * WidgetGroup::backgroundSvg() const
{
    return d->backgroundSvg;
}

const WidgetGroup::ColorScheme * WidgetGroup::backgroundColor() const
{
    if (!d->hasBackgroundColor) {
        return NULL;
    }
    return & d->backgroundColor;
}

const WidgetGroup::ColorScheme * WidgetGroup::foregroundColor() const
{
    return & d->foregroundColor;
}

void WidgetGroup::load(bool full)
{
    if (d->loaded && !full) return;
    d->loaded = true;

    d->hasBackgroundColor = false;
    if (d->ownsBackgroundSvg) {
        delete d->backgroundSvg;
    }
    d->backgroundSvg = NULL;

    WidgetGroup * group;

    if (!d->confGroupTheme->exists()) {
        group = d->instance->defaultGroup();
        if (group == this) return;

        d->copyFrom(group->d);
        return;
    }

    group = d->instance->group(d->confGroupTheme->readEntry("parent", "Default"));
    if (group != this) {
        group->load(false);
        d->copyFrom(group->d);
    }

    // Load properties from theme configuration file
    d->foregroundColor.normal   = d->confGroupTheme->readEntry("foreground.color.normal",   d->foregroundColor.normal);
    d->foregroundColor.active   = d->confGroupTheme->readEntry("foreground.color.active",   d->foregroundColor.active);
    d->foregroundColor.disabled = d->confGroupTheme->readEntry("foreground.color.disabled", d->foregroundColor.disabled);

    QString type = d->confGroupTheme->readEntry("background.type", "none");
    if (type == "color") {
        d->hasBackgroundColor       = true;
        d->backgroundColor.normal   = d->confGroupTheme->readEntry("background.color.normal",   d->backgroundColor.normal);
        d->backgroundColor.active   = d->confGroupTheme->readEntry("background.color.active",   d->backgroundColor.active);
        d->backgroundColor.disabled = d->confGroupTheme->readEntry("background.color.disabled", d->backgroundColor.disabled);
    } else if (type == "svg") {
        if (d->ownsBackgroundSvg) {
            delete d->backgroundSvg;
        }

        d->backgroundSvg = new Plasma::FrameSvg(NULL);
        d->backgroundSvg->setImagePath(
            Plasma::Theme::defaultTheme()->imagePath(
                d->confGroupTheme->readEntry("background.svg")));
        d->backgroundSvg->setCacheAllRenderedFrames(true);
        d->ownsBackgroundSvg = true;
    }

    notifyUpdated();
}

void WidgetGroup::notifyUpdated()
{
    if (!d->instance->isActivated()) {
        return;
    }

    foreach (Widget * widget, d->widgets) {
        if (widget->L_isInitialized())
            widget->groupUpdated();
    }
}

// Instance

class Instance::Private {
public:
    Private()
      : processGroupChanges(false),
        confMain(NULL),
        confTheme(NULL)
    {}

    ~Private()
    {
    }

    static bool hasApplication;

    // TODO: Warning! When threading comes around this approach will break...
    // it'll need mutexes, or something else...
    static Instance * activeInstance;
    static QList < Instance * > activeInstanceStack;
    static QMutex activeInstanceLock;

    QList< Widget * > widgets;
    QMap < QString, WidgetGroup * > groups;

    bool processGroupChanges : 1;
    KConfig * confMain;
    KConfig * confTheme;

    void loadAllGroups()
    {
        foreach(WidgetGroup * group, groups) {
            group->load();
        }
    }
};

Instance * Instance::Private::activeInstance = NULL;
QList < Instance * > Instance::Private::activeInstanceStack;
QMutex Instance::Private::activeInstanceLock(QMutex::Recursive);
bool Instance::Private::hasApplication = false;

Instance * Instance::activeInstance()
{
    return Instance::Private::activeInstance;
}

void Instance::setActiveInstanceAndLock(Instance * instance)
{
    Instance::Private::activeInstanceLock.lock();
    Instance::Private::activeInstanceStack
        .append(Instance::Private::activeInstance);
    Instance::Private::activeInstance = instance;
}

void Instance::releaseActiveInstanceLock()
{
    Instance::Private::activeInstance =
        Instance::Private::activeInstanceStack.takeLast();
    Instance::Private::activeInstanceLock.unlock();
}

void Instance::activateAll()
{
    d->processGroupChanges = true;

    d->loadAllGroups();
}

void Instance::deactivateAll()
{
    d->processGroupChanges = false;
}

bool Instance::isActivated()
{
    return d->processGroupChanges;
}

Instance::Instance()
  : d(new Private)
{
    QString app = KGlobal::mainComponent().componentName();
    if (app == "lancelot") {
        app = "";
    } else {
        app += '-';
    }

    d->confMain = new KConfig("lancelot" + app + "rc");

    QString search = "desktoptheme/" + Plasma::Theme::defaultTheme()->themeName() + "/lancelot/" + app + "theme.config";

    QString path =  KStandardDirs::locate( "data", search );
    if (path.isEmpty()) {
        search = "desktoptheme/default/lancelot/theme.config";
        path =  KStandardDirs::locate( "data", search );
    }
    if (path.isEmpty()) {
        path = "lancelotrc";
    }
    d->confTheme = new KConfig(path);

    Instance::Private::activeInstance = this;
}

Instance::~Instance()
{
    Widget * widget;
    while (!d->widgets.empty()) {
        widget = d->widgets.takeFirst();
        delete widget;
    }
    foreach (WidgetGroup * group, d->groups) {
        delete group;
    }
    delete d->confMain;
    delete d->confTheme;

    delete d;
}

KConfig * Instance::theme()
{
    return d->confTheme;
}

KConfig * Instance::config()
{
    return d->confMain;
}

void Instance::addWidget(Widget * widget)
{
    if (widget == NULL) return;
    if (d->widgets.contains(widget)) return;
    d->widgets.append(widget);
}

void Instance::removeWidget(Widget * widget)
{
    if (widget == NULL) return;
    if (!d->widgets.contains(widget)) return;
    d->widgets.removeAll(widget);
}

WidgetGroup * Instance::group(const QString & name)
{
    QString groupName = name;
    if (groupName.isEmpty()) {
        groupName = "Default";
    }

    if (!d->groups.contains(groupName)) {
        WidgetGroup * group = new WidgetGroup(this, groupName);
        if (d->processGroupChanges) {
            group->load();
        }
        d->groups.insert(groupName, group);
    }

    return d->groups[groupName];
}

WidgetGroup * Instance::defaultGroup()
{
    return group("Default");
}

bool Instance::hasApplication()
{
    return Instance::Private::hasApplication;
}

void Instance::setHasApplication(bool value)
{
    Instance::Private::hasApplication = value;
}

} // namespace Lancelot

