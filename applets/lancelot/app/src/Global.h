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
#include <KConfig>
#include <KConfigGroup>

namespace Lancelot
{

class Widget;

class WidgetGroup : public QObject {
public:
    static WidgetGroup * getGroup(QString name);
    static WidgetGroup * getDefaultGroup();
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
    static QMap < QString, WidgetGroup * > m_groups;
    KConfigGroup * m_confGroupTheme;
    
    WidgetGroup(QString name);
    virtual ~WidgetGroup();

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
	
    KConfig * theme();
    KConfig * config();
	
	
private:
    static Global * m_instance;

    Global();
    virtual ~Global();

    QList< Widget * > m_widgets;
    
    KConfig * m_confLancelot;
    KConfig * m_confTheme;
    //(const QString &file=QString(), OpenFlags mode=FullConfig, const char *resourceType="config")
    
    friend class WidgetGroup;
};

}

#endif /*GLOBAL_H_*/
