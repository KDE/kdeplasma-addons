/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free 
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
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

WidgetGroup * WidgetGroup::getGroup(QString name)
{
    if (!m_groups.contains(name)) {
        m_groups.insert(name, new WidgetGroup(name));
    }
    return m_groups[name];
}

WidgetGroup * WidgetGroup::getDefaultGroup()
{
    return getGroup("Default");
}

WidgetGroup::WidgetGroup(QString name)
  : m_confGroupTheme(NULL), m_name(name)
{
    m_confGroupTheme = new KConfigGroup(Global::getInstance()->theme(), "Group-" + name);
}

WidgetGroup::~WidgetGroup()
{
    delete m_confGroupTheme;
}

void WidgetGroup::addWidget(Widget * widget)
{
    if (!widget) return;
    
    if (m_widgets.contains(widget)) return;
    m_widgets.append(widget);
    
}

void WidgetGroup::removeWidget(Widget * widget)
{
    if (WidgetGroup::getDefaultGroup() == this) return;
    
    if (!m_widgets.contains(widget)) return;
    m_widgets.removeAll(widget);
    
    widget->setGroup(NULL);
}

bool WidgetGroup::hasProperty(QString property)
{
    return m_properties.contains(property);
}

QVariant WidgetGroup::getProperty(QString property)
{
    return m_properties.value(property);
}

QColor WidgetGroup::getPropertyAsColor(QString property)
{
    return m_properties.value(property).value<QColor>();
}

QString WidgetGroup::getPropertyAsString(QString property)
{
    return m_properties.value(property).toString();
}

int WidgetGroup::getPropertyAsInteger(QString property)
{
    return m_properties.value(property).toInt();
}

bool WidgetGroup::getPropertyAsBoolean(QString property)
{
    return m_properties.value(property).toBool();
}

void * WidgetGroup::getPropertyAsPointer(QString property)
{
    return m_properties.value(property).value< void * >();
}

QString WidgetGroup::name()
{
    return m_name;
}

void WidgetGroup::loadAll()
{
    kDebug() << "Load all\n";
    foreach(WidgetGroup * group, m_groups) {
        group->load();
    }
}

void WidgetGroup::load()
{
    // TODO: Load properties from theme configuration file
    m_foregroundColorNormal = m_confGroupTheme->readEntry("foreground.color.normal", QColor(Qt::black));
    m_foregroundColorActive = m_confGroupTheme->readEntry("foreground.color.active", QColor(Qt::black));
    m_backgroundColorNormal = m_confGroupTheme->readEntry("background.color.normal", QColor(Qt::white));
    m_backgroundColorActive = m_confGroupTheme->readEntry("background.color.active", QColor(Qt::white));
    
    m_properties["foregroundColorNormal"] = qVariantFromValue((void *) & m_foregroundColorNormal);
    m_properties["foregroundColorActive"] = qVariantFromValue((void *) & m_foregroundColorActive);
    m_properties["backgroundColorNormal"] = qVariantFromValue((void *) & m_backgroundColorNormal);
    m_properties["backgroundColorActive"] = qVariantFromValue((void *) & m_backgroundColorActive);
    
    foreach (Widget * widget, m_widgets) {
        widget->groupUpdated();
    }
}

// Global

Global * Global::m_instance = NULL;

Global * Global::getInstance() {
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
