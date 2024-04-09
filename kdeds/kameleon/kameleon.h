/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <KConfigWatcher>
#include <kdedmodule.h>

#include <QColor>
#include <QFileSystemWatcher>
#include <QTimer>

#define LED_SYSFS_PATH "/sys/class/leds/"
#define LED_INDEX_FILE "/multi_index"
#define LED_RGB_FILE "/multi_intensity"

#define CONFIG_KEY_ACCENT "DeviceLedsAccentColored"
#define CONFIG_KEY_COLOR "DeviceLedsColor"

class Kameleon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kameleon")
public:
    Kameleon(QObject *parent, const QList<QVariant> &);

    /**
     * Returns whether there are any RGB cabaple LED devices.
     */
    Q_SCRIPTABLE bool isSupported();

    /**
     * Returns whether accent color syncing is enabled.
     */
    Q_SCRIPTABLE bool isAccent();

    /**
     * Returns the currently set color as a hex string.
     */
    Q_SCRIPTABLE QString currentColor();

    /**
     * Enables or disables following the accent color.
     */
    Q_SCRIPTABLE void setAccent(bool enabled);

    /**
     * Sets a custom color given as a hex string.
     */
    Q_SCRIPTABLE void setColor(QString colorName);

Q_SIGNALS:
    /**
     * Emitted when the accent syncing enabled state changes.
     */
    Q_SCRIPTABLE void accentChanged(bool enabled);
    /**
     * Emitted when the led color changes.
     */
    Q_SCRIPTABLE void activeColorChanged(QString colorName);

private:
    KSharedConfig::Ptr m_config;
    KConfigWatcher::Ptr m_configWatcher;
    QFileSystemWatcher m_fsWatcher;
    QTimer *m_loadLedColorTimer;
    QStringList m_rgbLedDevices;
    QStringList m_deviceRgbIndices;
    QColor m_targetColor;
    QColor m_currentColor;

    bool m_accent = true;
    QColor m_accentColor = QColor(QColorConstants::White);
    QColor m_customColor = QColor(QColorConstants::White);

    void findRgbLedDevices();
    void loadLedColor();
    void loadConfig();
    void updateAccentColor();
    void updateCustomColor();
    void applyColor(QColor color);
};
