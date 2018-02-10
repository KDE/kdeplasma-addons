/*
 * Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "colorpickerplugin.h"

// Qt
#include <QColor>
#include <QQmlEngine>

#include "grabwidget.h"

class Utils : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE bool isValidColor(const QString &colorString) const
    {
        return QColor::isValidColor(colorString);
    }

};

static QObject *utils_singletontype_provider(QQmlEngine *, QJSEngine *)
{
    return new Utils();
}

void ColorPickerPlugin::registerTypes(const char* uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.colorpicker"));
    qmlRegisterType<GrabWidget>(uri, 2, 0, "GrabWidget");
    qmlRegisterSingletonType<Utils>(uri, 2, 0, "Utils", utils_singletontype_provider);
}

#include "colorpickerplugin.moc"
