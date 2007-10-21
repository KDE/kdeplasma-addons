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

namespace Lancelot
{

// Group
QMap < QString, Group * > Group::m_groups; 

Group * Group::getGroup(QString name)
{
    if (!m_groups.contains(name)) {
        m_groups.insert(name, new Group(name));
    }
    return m_groups[name];
}

Group * Group::getDefaultGroup()
{
    return getGroup("Default");
}

Group::Group(QString name)
  : m_name(name), m_stop(false)
{
}

Group::~Group()
{
}

void Group::addWidget(Widget * widget)
{
    if (!widget) return;
    if (m_widgets.contains(widget)) return;
    
    if (m_stop) return;
    m_stop = true;
    widget->setGroup(this);
    m_widgets.append(widget);
    
    if (Global::getInstance()->processGroupChanges) {
        widget->groupUpdated();
    }
    
    m_stop = false;
}

void Group::removeWidget(Widget * widget)
{
    if (Group::getDefaultGroup() == this) return;
    widget->setGroup(NULL);
}

bool Group::hasProperty(QString property)
{
    return m_properties.contains(property);
}

QVariant Group::getProperty(QString property)
{
    return m_properties.value(property);
}

QColor Group::getPropertyAsColor(QString property)
{
    return m_properties.value(property).value<QColor>();
}

QString Group::getPropertyAsString(QString property)
{
    return m_properties.value(property).toString();
}

int Group::getPropertyAsInteger(QString property)
{
    return m_properties.value(property).toInt();
}

bool Group::getPropertyAsBoolean(QString property)
{
    return m_properties.value(property).toBool();
}

void * Group::getPropertyAsPointer(QString property)
{
    return m_properties.value(property).value< void * >();
}

QString Group::name()
{
    return m_name;
}

void Group::loadAll()
{
    kDebug() << "Load all\n";
    foreach(Group * group, m_groups) {
        group->load();
    }
}

void Group::load()
{
    // TODO: Load properties from theme configuration file
    m_foregroundColorNormal = QColor(125, 125, 125);
    m_foregroundColorActive = QColor(175, 175, 175);
    m_backgroundColorNormal = QColor(0, 0, 0);
    m_backgroundColorActive = QColor(255, 255, 255);
    
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
    Group::loadAll();
}

void Global::deactivateAll() {
    processGeometryChanges = false;
    processUpdateRequests = false;
    processGroupChanges = false;
}

Global::Global()
  : processGeometryChanges(false),
    processUpdateRequests(false),
    processGroupChanges(false)
{
}

Global::~Global()
{
}

void Global::addWidget(Widget * widget)
{
    if (widget == NULL) return;
    if (m_widgets.contains(widget)) return;
    m_widgets.append(widget);
}

}
