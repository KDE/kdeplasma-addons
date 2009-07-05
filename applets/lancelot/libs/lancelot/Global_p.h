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
#include "widgets/Widget.h"
#include "widgets/BasicWidget.h"
#include <KGlobal>
#include <plasma/theme.h>
#include <KStandardDirs>
#include <QMutex>

namespace Lancelot
{

// Group
class WidgetGroup::Private {
public:
    Private();

    ~Private();

    Instance * instance;

    QString name;
    QMap < QString, QVariant > properties;

    // TODO: rename when finished removing
    // QString properties
    QMap < int, QVariant > int_properties;

    QList < Widget * > widgets;

    ColorScheme foregroundColor;
    ColorScheme backgroundColor;
    Plasma::FrameSvg * backgroundSvg;

    bool ownsBackgroundSvg : 1;
    bool loaded : 1;

    void copyFrom(WidgetGroup::Private * d);
    KConfigGroup confGroupTheme();
};

class Instance::Private: public QObject {
    Q_OBJECT
public:
    Private();

    ~Private();

    static bool hasApplication;

    // TODO: Warning! When threading comes around this approach will break...
    // it'll need mutexes, or something else...
    static Instance * activeInstance;
    static QList < Instance * > activeInstanceStack;
    static QMutex activeInstanceLock;

    QList < Widget * > widgets;
    QMap < QString, WidgetGroup * > groups;

    bool processGroupChanges : 1;
    KConfig * confMain;
    KConfig * confTheme;

    void createConfTheme();

public Q_SLOTS:
    void loadAllGroups();
    void themeChanged();
};

} // namespace Lancelot

