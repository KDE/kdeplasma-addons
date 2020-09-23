/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef DICT_PLUGIN_H
#define DICT_PLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class DictPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif
