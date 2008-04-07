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

#include "lancelot_export.h"

namespace Lancelot
{

class Widget;
class Instance;

class LANCELOT_EXPORT WidgetGroup : public QObject {
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
    void notifyUpdated();

    void load(bool full = false);

    void addWidget(Widget * widget);
    void removeWidget(Widget * widget, bool setDefaultGroup = true);

    QString name() const;

    Instance * instance();

private:
    class Private;
    Private * d;

    WidgetGroup(Instance * instance, QString name);
    virtual ~WidgetGroup();

    friend class Instance;
};

class LANCELOT_EXPORT Instance : public QObject {
public:
    Instance();
    virtual ~Instance();

    void activateAll();
    void deactivateAll();

    void addWidget(Widget * widget);

    KConfig * theme();
    KConfig * config();

    WidgetGroup * group(const QString & name);
    WidgetGroup * defaultGroup();

    ///bool isApplication();
    ///void setIsApplication(bool value);

    static Instance * activeInstance();
    static void setActiveInstance(Instance * instance);

    static bool hasApplication();
    static void setHasApplication(bool value);

private:
    class Private;
    Private * d;
};

}

#endif /*GLOBAL_H_*/
