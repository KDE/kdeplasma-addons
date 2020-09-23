/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef QUOTA_WATCH_PLUGIN_H
#define QUOTA_WATCH_PLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class DiskQuotaPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // QUOTA_WATCH_PLUGIN_H
