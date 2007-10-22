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

#ifndef LANCELOT_GLOBAL_H_
#define LANCELOT_GLOBAL_H_

#include <QColor>
#include <QString>
#include <QMap>
#include <QList>
#include <QVariant>

namespace Lancelot
{

class Widget;

class Group : public QObject {
public:
    static Group * getGroup(QString name);
    static Group * getDefaultGroup();
    static void loadAll();

    bool     hasProperty(QString property);
    QVariant getProperty(QString property);
    QColor   getPropertyAsColor(QString property);
    QString  getPropertyAsString(QString property);
    int      getPropertyAsInteger(QString property);
    bool     getPropertyAsBoolean(QString property);
    void *   getPropertyAsPointer(QString property);
    
    void load();
    
    void addWidget(Widget * widget);
    void removeWidget(Widget * widget);
    
    QString name();
    
private:
    static QMap < QString, Group * > m_groups;
    
    Group(QString name);
    virtual ~Group();

    QString m_name;
    QMap < QString, QVariant > m_properties;
    
    QList < Widget * > m_widgets;
    
    QColor m_foregroundColorNormal;
    QColor m_foregroundColorActive;
    QColor m_backgroundColorNormal;
    QColor m_backgroundColorActive;
};

class Global : public QObject {
public:
    static Global * getInstance();

    bool processGeometryChanges;
	bool processUpdateRequests;
    bool processGroupChanges;

	void activateAll();
	void deactivateAll();

	void addWidget(Widget * widget);
	
private:
    static Global * m_instance;

    Global();
    virtual ~Global();

    QList< Widget * > m_widgets;
};

}

#endif /*GLOBAL_H_*/
