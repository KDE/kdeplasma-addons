/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kameleon.h"
#include "kameleon_debug.h"

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include <KAuth/Action>
#include <KAuth/ActionReply>
#include <KAuth/ExecuteJob>
#include <KAuth/HelperSupport>

#include <QCollator>
#include <QColor>
#include <QDir>
#include <QFileInfo>

K_PLUGIN_CLASS_WITH_JSON(Kameleon, "kameleon.json")

Kameleon::Kameleon(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , m_configWatcher(KConfigWatcher::create(KSharedConfig::openConfig("kdeglobals")))
{
    findRgbLedDevices();
    if (!isSupported()) {
        qCInfo(KAMELEON) << "found no RGB LED devices";
        return;
    }

    loadConfig();
    connect(m_configWatcher.get(), &KConfigWatcher::configChanged, this, [this]() {
        loadConfig();
    });
}

void Kameleon::findRgbLedDevices()
{
    QDir ledsDir(LED_SYSFS_PATH);
    ledsDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);
    auto ledDevices = ledsDir.entryList();

    // sort to ensure keys light up nicely one next to the other
    QCollator coll;
    coll.setNumericMode(true);
    std::sort(ledDevices.begin(), ledDevices.end(), [&](const QString &s1, const QString &s2) {
        return coll.compare(s1, s2) < 0;
    });

    for (const QString &ledDevice : ledDevices) {
        QFile rgbFile(LED_SYSFS_PATH + ledDevice + QLatin1String("/") + LED_RGB_FILE);
        if (QFileInfo(rgbFile).exists()) {
            qCInfo(KAMELEON) << "found RGB LED device" << ledDevice;
            m_rgbLedDevices.append(rgbFile.fileName());
        }
    }
}

bool Kameleon::isSupported()
{
    return !m_rgbLedDevices.isEmpty();
}

bool Kameleon::isEnabled()
{
    return m_enabled;
}

void Kameleon::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        qCInfo(KAMELEON) << "enabled changed" << enabled;
        m_enabled = enabled;

        KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");
        config->group("General").writeEntry<bool>("AccentColoredDeviceLeds", enabled);

        if (enabled) {
            applyColor(m_accentColor);
        } else {
            applyColor(QColor(QColorConstants::White));
        }
    }
}

void Kameleon::loadConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");

    m_enabled = config->group("General").readEntry<bool>("AccentColoredDeviceLeds", true);

    QColor customAccentColor = config->group("General").readEntry<QColor>("AccentColor", QColor::Invalid);
    QColor schemeAccentColor = config->group("Colors::View").readEntry<QColor>("ForegroundActive", QColor::Invalid);
    QColor activeAccentColor = customAccentColor.isValid() ? customAccentColor
        : schemeAccentColor.isValid()                      ? schemeAccentColor
                                                           : QColor(QColorConstants::White);

    if (activeAccentColor != m_accentColor) {
        qCInfo(KAMELEON) << "accent color changed" << activeAccentColor;
        m_accentColor = activeAccentColor;
        if (m_enabled) {
            applyColor(m_accentColor);
        }
    }
}

void Kameleon::applyColor(QColor color)
{
    const QByteArray accentColorStr = QByteArray::number(color.red()) + " " + QByteArray::number(color.green()) + " " + QByteArray::number(color.blue());

    KAuth::Action action("org.kde.kameleonhelper.writecolor");
    action.setHelperId("org.kde.kameleonhelper");
    action.addArgument("color", accentColorStr);
    action.addArgument("devices", m_rgbLedDevices);
    auto *job = action.execute();

    connect(job, &KAuth::ExecuteJob::result, this, [job] {
        if (job->error()) {
            qCWarning(KAMELEON) << "Failed to write color to devices" << job->errorText();
            return;
        }
    });
    job->start();
}

#include "kameleon.moc"
