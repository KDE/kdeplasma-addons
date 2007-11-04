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

#include "LancelotApplication.h"

namespace Lancelot
{

class Widget;
class Instance;

class WidgetGroup : public QObject {
public:

    class ColorScheme {
    public:
        QColor normal, disabled, active;
    };

    bool hasProperty(const QString & property) const;
    QVariant property(const QString & property) const;
    void setProperty(const QString & property, const QVariant & value);

    Plasma::Svg * backgroundSvg() const;
    const ColorScheme * backgroundColor() const;
    const ColorScheme * foregroundColor() const;
    
    void load(bool full = false);

    void addWidget(Widget * widget);
    void removeWidget(Widget * widget);

    QString name() const;

private:
    KConfigGroup * m_confGroupTheme;

    WidgetGroup(Instance * instance, QString name);
    virtual ~WidgetGroup();
    
    Instance * m_instance;

    QString m_name;
    QMap < QString, QVariant > m_properties;

    QList < Widget * > m_widgets;

    ColorScheme m_foregroundColor;
    ColorScheme m_backgroundColor;
    Plasma::Svg * m_backgroundSvg;
    bool m_hasBackgroundColor : 1;
    bool m_ownsBackgroundSvg : 1;
    bool m_loaded : 1;
    //QPixmap * m_cachedBackgroundNormal;
    //QPixmap * m_cachedBackgroundActive;
    //QPixmap * m_cachedBackgroundDisabled;

    void copyFrom(WidgetGroup * group);

    friend class Widget;
    friend class Instance;
};

class Instance : public QObject {
public:
    Instance();
    virtual ~Instance();

    bool processGroupChanges : 1;

	void activateAll();
	void deactivateAll();

	void addWidget(Widget * widget);

    KConfig * theme();
    KConfig * config();
    
    WidgetGroup * group(const QString & name);
    WidgetGroup * defaultGroup();
    
    static Instance * activeInstance();
    static void setActiveInstance(Instance * instance);

private:
    void loadAllGroups();
    
    // TODO: Warning! When threading comes around this approach will break... 
    // it'll need mutexes, or something else...
    static Instance * m_activeInstance;

    QList< Widget * > m_widgets;
    QMap < QString, WidgetGroup * > m_groups;

    KConfig * m_confLancelot;
    KConfig * m_confTheme;

    friend class WidgetGroup;
};

}

#endif /*GLOBAL_H_*/
