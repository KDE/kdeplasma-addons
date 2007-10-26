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
#include <plasma/svg.h>

namespace Lancelot
{

class Widget;

class WidgetGroup : public QObject {
public:
    
    class ColorScheme {
    public:
        QColor normal, disabled, active;
    };
    
    static WidgetGroup * group(const QString & name);
    static WidgetGroup * defaultGroup();
    static void loadAll();

    bool     hasProperty(QString property) const;
    QVariant property(QString property) const;
    QColor   propertyAsColor(QString property) const;
    QString  propertyAsString(QString property) const;
    int      propertyAsInteger(QString property) const;
    bool     propertyAsBoolean(QString property) const;
    void *   propertyAsPointer(QString property) const;
    
    Plasma::Svg * backgroundSvg() const;
    const ColorScheme * backgroundColor() const;
    const ColorScheme * foregroundColor() const;
    
    void load();
    
    void addWidget(Widget * widget);
    void removeWidget(Widget * widget);
    
    QString name() const;
    
private:
    static QMap < QString, WidgetGroup * > m_groups;
    KConfigGroup * m_confGroupTheme;
    
    WidgetGroup(QString name);
    virtual ~WidgetGroup();

    QString m_name;
    QMap < QString, QVariant > m_properties;
    
    QList < Widget * > m_widgets;
    
    ColorScheme m_foregroundColor;
    ColorScheme m_backgroundColor;
    Plasma::Svg * m_backgroundSvg;
    bool m_hasBackgroundColor : 1;
    bool m_ownsBackgroundSvg : 1;
    //QPixmap * m_cachedBackgroundNormal;
    //QPixmap * m_cachedBackgroundActive;
    //QPixmap * m_cachedBackgroundDisabled;
    
    void copyFrom(WidgetGroup * group);
    
    friend class Widget;
};

class Global : public QObject {
public:
    static Global * instance();

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
