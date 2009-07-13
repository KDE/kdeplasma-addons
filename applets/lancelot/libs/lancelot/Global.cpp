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
#include "Global_p.h"
#include "widgets/Widget.h"
#include "widgets/BasicWidget.h"
#include <KGlobal>
#include <plasma/theme.h>
#include <KStandardDirs>
#include <QMutex>

namespace Lancelot
{

// Group
WidgetGroup::Private::Private()
    : instance(NULL), name(QString()), backgroundSvg(NULL),
      ownsBackgroundSvg(false), loaded(false)
       // TODO : Add caching?
       //cachedBackgroundNormal(NULL), cachedBackgroundActive(NULL), cachedBackgroundDisabled(NULL)
{}

WidgetGroup::Private::~Private()
{
    if (ownsBackgroundSvg) {
        delete backgroundSvg;
    }
    //delete d->cachedBackgroundNormal;
    //delete d->cachedBackgroundActive;
    //delete d->cachedBackgroundDisabled;
}

void WidgetGroup::Private::copyFrom(WidgetGroup::Private * d)
{
    if (this == d) return;

    properties = d->properties;
    int_properties = d->int_properties;

    foregroundColor = d->foregroundColor;
    backgroundColor = d->backgroundColor;

    if (ownsBackgroundSvg) {
        delete backgroundSvg;
    }
    backgroundSvg = d->backgroundSvg;
    ownsBackgroundSvg = false;
}

KConfigGroup WidgetGroup::Private::confGroupTheme()
{
    return KConfigGroup(instance->theme(), "Group-" + name);
}

WidgetGroup::WidgetGroup(Instance * instance, QString name)
    : d(new Private())
{
    d->instance = instance;
    d->name = name;
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

bool WidgetGroup::hasProperty(int property) const
{
    return d->int_properties.contains(property);
}

QVariant WidgetGroup::property(int property) const
{
    return d->int_properties.value(property);
}

void WidgetGroup::setProperty(int property, const QVariant & value)
{
    d->int_properties[property] = value;
}

void WidgetGroup::clearProperty(int property)
{
    d->int_properties.remove(property);
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
    if (!hasProperty(Widget::WholeColorBackground) &&
        !hasProperty(BasicWidget::TextColorBackground)) {
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

    d->properties.clear();
    d->int_properties.clear();

    if (d->ownsBackgroundSvg) {
        delete d->backgroundSvg;
    }
    d->backgroundSvg = NULL;

    WidgetGroup * group;

    KConfigGroup confGroupTheme = d->confGroupTheme();
    if (!confGroupTheme.exists()) {
        group = d->instance->defaultGroup();
        if (group == this) return;

        d->copyFrom(group->d);
        return;
    }

    group = d->instance->group(confGroupTheme.readEntry("parent", "Default"));
    if (group != this) {
        group->load(false);
        d->copyFrom(group->d);
    }

    // Load properties from theme configuration file
    d->foregroundColor.normal   = confGroupTheme.readEntry("foreground.color.normal",   d->foregroundColor.normal);
    d->foregroundColor.active   = confGroupTheme.readEntry("foreground.color.active",   d->foregroundColor.active);
    d->foregroundColor.disabled = confGroupTheme.readEntry("foreground.color.disabled", d->foregroundColor.disabled);

    QString type = confGroupTheme.readEntry("background.type", "none");
    if (type == "color" || type == "color-compact") {
        if (type == "color") {
            setProperty(Widget::WholeColorBackground, 1);
        } else {
            setProperty(BasicWidget::TextColorBackground, 1);
        }
        d->backgroundColor.normal   = confGroupTheme.readEntry("background.color.normal",   d->backgroundColor.normal);
        d->backgroundColor.active   = confGroupTheme.readEntry("background.color.active",   d->backgroundColor.active);
        d->backgroundColor.disabled = confGroupTheme.readEntry("background.color.disabled", d->backgroundColor.disabled);
    } else if (type == "svg") {
        if (d->ownsBackgroundSvg) {
            delete d->backgroundSvg;
        }

        setProperty(Widget::SvgBackground, 1);
        d->backgroundSvg = new Plasma::FrameSvg(NULL);
        d->backgroundSvg->setImagePath(
            Plasma::Theme::defaultTheme()->imagePath(
                confGroupTheme.readEntry("background.svg")));
        d->backgroundSvg->setCacheAllRenderedFrames(true);
        d->ownsBackgroundSvg = true;
    }

    if (!confGroupTheme.readEntry(
                "foreground.blurtextshadow", QString()).isEmpty()) {
        setProperty(BasicWidget::BlurTextShadow, 1);
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

Instance * Instance::Private::activeInstance = NULL;
QList < Instance * > Instance::Private::activeInstanceStack;
QMutex Instance::Private::activeInstanceLock(QMutex::Recursive);
bool Instance::Private::hasApplication = false;

Instance::Private::Private()
      : processGroupChanges(false),
        confMain(NULL),
        confTheme(NULL)
{
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(themeChanged()));
}

Instance::Private::~Private()
{
}

void Instance::Private::createConfTheme()
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

void Instance::Private::themeChanged()
{
    createConfTheme();
    loadAllGroups();
}

void Instance::Private::loadAllGroups()
{
    foreach(WidgetGroup * group, groups) {
        group->load(true);
    }
}

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
  : d(new Private())
{

    d->confMain = new KConfig("lancelotrc");

    Plasma::Theme::defaultTheme()->setUseGlobalSettings(true);
    d->createConfTheme();

    Instance::Private::activeInstance = this;

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            d, SLOT(themeChanged()));
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

