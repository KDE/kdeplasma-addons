/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <KConfigWatcher>
#include <kdedmodule.h>

#include <QColor>

#include "backend.h"

class Kameleon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kameleon")
public:
    Kameleon(QObject *parent, const QList<QVariant> &);

    /**
     * Returns whether there are any RGB capable LED devices.
     */
    Q_SCRIPTABLE bool isSupported();

    /**
     * Returns whether accent color syncing is enabled.
     */
    Q_SCRIPTABLE bool isEnabled();

    /**
     * Enables or disables accent color syncing.
     */
    Q_SCRIPTABLE void setEnabled(bool enabled);

Q_SIGNALS:
    void supportedChanged();

private:
    bool m_enabled = true;
    QColor m_accentColor = QColor(QColorConstants::White);

    KSharedConfig::Ptr m_config;
    KConfigWatcher::Ptr m_configWatcher;

    void loadConfig();
    void applyColor(const QColor &color);

    std::vector<std::shared_ptr<Backend>> m_backends;
};
