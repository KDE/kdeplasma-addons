/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef GLOBAL_P_H
#define GLOBAL_P_H

#include "Global.h"

#include <QSet>

#include <KGlobal>
#include <KStandardDirs>

#include <Plasma/Theme>

namespace Lancelot
{

// Group
class GroupPrivate {
public:
    GroupPrivate();
    ~GroupPrivate();
    void setObjectProperty(QObject * object,
            const QString & property, const QVariant & value);
    void copyFrom(GroupPrivate * d);
    void reset();
    KConfigGroup confGroupTheme();

    QString name;
    QMap < QString, QVariant > properties;
    QSet < QString > persistentProperties;

    QSet < QObject * > objects;

    Group::ColorScheme foregroundColor;
    Group::ColorScheme backgroundColor;
    Plasma::FrameSvg * backgroundSvg;

    bool ownsBackgroundSvg : 1;
    bool loaded : 1;
};

class Global::Private: public QObject {
    Q_OBJECT
public:
    Private();

    ~Private();

    static Global * instance;

    QMap < QString, Group * > groups;

    KConfig * confMain;
    KConfig * confTheme;

    Plasma::ImmutabilityType immutability;

    void createConfTheme();

public Q_SLOTS:
    void loadAllGroups(bool clearFirst = true);
    void themeChanged();
    void objectDeleted(QObject * object);
};

} // namespace Lancelot

#endif /* GLOBAL_P_H */
