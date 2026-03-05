/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kameleon.h"
#include "kameleon_debug.h"

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QColor>

#include "qmk/qmk.h"
#include "sysfs/sysfs.h"

K_PLUGIN_CLASS_WITH_JSON(Kameleon, "kameleon.json")

using namespace Qt::StringLiterals;

Kameleon::Kameleon(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , m_config(KSharedConfig::openConfig(u"kdeglobals"_s))
    , m_configWatcher(KConfigWatcher::create(m_config))
    , m_backends{
          std::make_shared<QMK>(),
          std::make_shared<Sysfs>(),
      }
{
    for (const auto &backend : m_backends) {
        connect(backend.get(), &Backend::supportedChanged, this, &Kameleon::supportedChanged);
    }

    loadConfig();
    connect(m_configWatcher.get(), &KConfigWatcher::configChanged, this, [this]() {
        loadConfig();
    });
}

bool Kameleon::isSupported()
{
    return std::ranges::any_of(m_backends, [](const auto &backend) {
        return backend->isSupported();
    });
}

bool Kameleon::isEnabled()
{
    return m_enabled;
}

void Kameleon::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        qCDebug(KAMELEON) << "enabled changed" << enabled;
        m_enabled = enabled;
        m_config->group(u"General"_s).writeEntry<bool>("DeviceLedsAccentColored", enabled);
        m_config->sync();

        if (enabled) {
            applyColor(m_accentColor);
        } else {
            applyColor(QColor(QColorConstants::White));
        }
    }
}

void Kameleon::loadConfig()
{
    m_enabled = m_config->group(u"General"_s).readEntry<bool>("DeviceLedsAccentColored", true);

    auto customAccentColor = m_config->group(u"General"_s).readEntry<QColor>("AccentColor", QColor::Invalid);
    auto schemeAccentColor = m_config->group(u"Colors:View"_s).readEntry<QColor>("ForegroundActive", QColor::Invalid);

    auto activeAccentColor = customAccentColor;
    if (!activeAccentColor.isValid()) {
        activeAccentColor = schemeAccentColor;
    }
    if (!activeAccentColor.isValid()) {
        activeAccentColor = QColor(QColorConstants::White);
    }

    if (activeAccentColor != m_accentColor) {
        m_accentColor = activeAccentColor;
        if (m_enabled) {
            applyColor(m_accentColor);
        }
    }
}

void Kameleon::applyColor(const QColor &color)
{
    for (const auto &backend : m_backends) {
        backend->applyColor(color);
    }
}

#include "kameleon.moc"
