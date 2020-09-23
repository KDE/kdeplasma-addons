/*
    SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TIMERPLUGIN_H
#define TIMERPLUGIN_H

#include <QQmlExtensionPlugin>

class TimerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // TIMERPLUGIN_H
