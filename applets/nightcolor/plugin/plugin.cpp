/*
 * SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "inhibitor.h"
#include "monitor.h"

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class NightColorControlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri)
    {
        qmlRegisterType<Inhibitor>(uri, 1, 0, "Inhibitor");
        qmlRegisterType<Monitor>(uri, 1, 0, "Monitor");
    }
};

#include "plugin.moc"
