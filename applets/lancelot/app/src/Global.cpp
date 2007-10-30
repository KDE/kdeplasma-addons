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
#include "Widget.h"
#include <KDebug>
#include <plasma/theme.h>
#include <KStandardDirs>

namespace Lancelot
{

// Group
QMap < QString, WidgetGroup * > WidgetGroup::m_groups;

WidgetGroup * WidgetGroup::group(const QString & name)
{
    QString groupName = name;
    if (groupName == "") {
        groupName = "Default";
    }

    if (!m_groups.contains(groupName)) {
        WidgetGroup * group = new WidgetGroup(groupName);
        if (Global::instance()->processGroupChanges) {
            group->load();
        }
        m_groups.insert(groupName, group);
    }

    return m_groups[groupName];
}

WidgetGroup * WidgetGroup::defaultGroup()
{
    return group("Default");
}

WidgetGroup::WidgetGroup(QString name)
  : m_confGroupTheme(NULL), m_name(name), m_backgroundSvg(NULL),
    m_hasBackgroundColor(false), m_ownsBackgroundSvg(false), m_loaded(false)
    // TODO : Add caching?
    //m_cachedBackgroundNormal(NULL), m_cachedBackgroundActive(NULL), m_cachedBackgroundDisabled(NULL)
{
    kDebug() << "Creating group named " << name << "\n";
    m_confGroupTheme = new KConfigGroup(Global::instance()->theme(), "Group-" + name);
}

WidgetGroup::~WidgetGroup()
{
    delete m_confGroupTheme;
    if (m_ownsBackgroundSvg) {
        delete m_backgroundSvg;
    }
    //delete m_cachedBackgroundNormal;
    //delete m_cachedBackgroundActive;
    //delete m_cachedBackgroundDisabled;
}

void WidgetGroup::addWidget(Widget * widget)
{
    if (!widget) return;

    if (m_widgets.contains(widget)) return;
    m_widgets.append(widget);

}

void WidgetGroup::removeWidget(Widget * widget)
{
    if (WidgetGroup::defaultGroup() == this) return;

    if (!m_widgets.contains(widget)) return;
    m_widgets.removeAll(widget);

    widget->setGroup(NULL);
}

bool WidgetGroup::hasProperty(const QString & property) const
{
    return m_properties.contains(property);
}

QVariant WidgetGroup::property(const QString & property) const
{
    return m_properties.value(property);
}

void WidgetGroup::setProperty(const QString & property, const QVariant & value)
{
    m_properties[property] = value;
}

QString WidgetGroup::name() const
{
    return m_name;
}

Plasma::Svg * WidgetGroup::backgroundSvg() const
{
    return m_backgroundSvg;
}

const WidgetGroup::ColorScheme * WidgetGroup::backgroundColor() const
{
    if (!m_hasBackgroundColor) {
        return NULL;
    }
    return & m_backgroundColor;
}

const WidgetGroup::ColorScheme * WidgetGroup::foregroundColor() const
{
    return & m_foregroundColor;
}


void WidgetGroup::loadAll()
{
    kDebug() << "Load all\n";
    foreach(WidgetGroup * group, m_groups) {
        group->load();
    }
}

void WidgetGroup::copyFrom(WidgetGroup * group)
{
    if (this == group) return;

    m_properties = group->m_properties;

    m_foregroundColor = group->m_foregroundColor;
    m_hasBackgroundColor = group->m_hasBackgroundColor;
    m_backgroundColor = group->m_backgroundColor;

    if (m_ownsBackgroundSvg) {
        delete m_backgroundSvg;
    }
    m_backgroundSvg = group->m_backgroundSvg;
    m_ownsBackgroundSvg = false;
}

void WidgetGroup::load(bool full)
{
    if (m_loaded && !full) return;
    m_loaded = true;

    kDebug() << "Loading group " << m_name << "\n";

    m_hasBackgroundColor = false;
    if (m_ownsBackgroundSvg) {
        delete m_backgroundSvg;
    }
    m_backgroundSvg = NULL;

    WidgetGroup * group;

    if (!m_confGroupTheme->exists()) {
        kDebug() << "This (" << m_name << ") group is not defined in the theme. Loading the default group.\n";
        group = WidgetGroup::defaultGroup();
        if (group == this) return;

        copyFrom(group);
        return;
    }

    group = WidgetGroup::group(m_confGroupTheme->readEntry("parent", "Default"));
    if (group != this) {
        group->load(false);
        copyFrom(group);
    }

    // TODO: Load properties from theme configuration file
    m_foregroundColor.normal   = m_confGroupTheme->readEntry("foreground.color.normal",   m_foregroundColor.normal);
    m_foregroundColor.active   = m_confGroupTheme->readEntry("foreground.color.active",   m_foregroundColor.active);
    m_foregroundColor.disabled = m_confGroupTheme->readEntry("foreground.color.disabled", m_foregroundColor.disabled);

    QString type = m_confGroupTheme->readEntry("background.type", "none");
    kDebug() << "Background for group " << m_name << " is of " << type << " type\n";
    if (type == "color") {
        kDebug() << "Background for group " << m_name << " is a color scheme\n";
        m_hasBackgroundColor       = true;
        m_backgroundColor.normal   = m_confGroupTheme->readEntry("background.color.normal",   m_backgroundColor.normal);
        m_backgroundColor.active   = m_confGroupTheme->readEntry("background.color.active",   m_backgroundColor.active);
        m_backgroundColor.disabled = m_confGroupTheme->readEntry("background.color.disabled", m_backgroundColor.disabled);
    } else if (type == "svg") {
        if (m_ownsBackgroundSvg) {
            delete m_backgroundSvg;
        }
        kDebug() << "Background for group " << m_name << " is " << m_confGroupTheme->readEntry("background.svg") << "\n";
        m_backgroundSvg = new Plasma::Svg(m_confGroupTheme->readEntry("background.svg"));
        m_ownsBackgroundSvg = true;
        m_backgroundSvg->setContentType(Plasma::Svg::ImageSet);
    }

    kDebug() << "Notifying objects\n";
    foreach (Widget * widget, m_widgets) {
        widget->groupUpdated();
    }
}

// Global

Global * Global::m_instance = NULL;

Global * Global::instance() {
    if (!m_instance) {
        m_instance = new Global();
    }
    return m_instance;
}

void Global::activateAll() {
    processGeometryChanges = true;
    processUpdateRequests = true;
    processGroupChanges = true;

    WidgetGroup::loadAll();
}

void Global::deactivateAll() {
    processGeometryChanges = false;
    processUpdateRequests = false;
    processGroupChanges = false;
}

Global::Global()
  : processGeometryChanges(false),
    processUpdateRequests(false),
    processGroupChanges(false),
    m_confLancelot(NULL),
    m_confTheme(NULL)
{
    Plasma::Theme::self()->setApplication("Lancelot");
    m_confLancelot = new KConfig("lancelotrc");

    // TODO: If Plasma::Theme supports file(), alter the following code
    QString search = "desktoptheme/" + Plasma::Theme::self()->themeName() + "/lancelot/theme.config";
    QString path =  KStandardDirs::locate( "data", search );
    if (path == "") {
        kDebug() << "Error: No theme configuration file: " << search << "\n";
        path = "lancelotrc";
    }
    m_confTheme = new KConfig(path);

}

Global::~Global()
{
    delete m_confLancelot;
    delete m_confTheme;
}

KConfig * Global::theme()
{
    return m_confTheme;
}

KConfig * Global::config()
{
    return m_confLancelot;
}

void Global::addWidget(Widget * widget)
{
    if (widget == NULL) return;
    if (m_widgets.contains(widget)) return;
    m_widgets.append(widget);
}

}
