/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "colorpickerplugin.h"

// Qt
#include <QColor>

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
