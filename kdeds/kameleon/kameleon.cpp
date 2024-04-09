/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kameleon.h"
#include "kameleon_debug.h"

#include <KConfigGroup>
#include <KConfigWatcher>
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
#include <QFileSystemWatcher>
#include <QTimer>

K_PLUGIN_CLASS_WITH_JSON(Kameleon, "kameleon.json")

Kameleon::Kameleon(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , m_config(KSharedConfig::openConfig("kdeglobals"))
    , m_configWatcher(KConfigWatcher::create(m_config))
    , m_fsWatcher()
{
    findRgbLedDevices();
    if (!isSupported()) {
        qCInfo(KAMELEON) << "found no RGB LED devices";
        return;
    }

    loadLedColor();

    // When any device file changes, check all of them so we know if they have a uniform color.
    // Wait a little before we load the colors to ensure the ongoing write operation is done with all files.
    // While we're already checking, avoid firing more signals.
    m_loadLedColorTimer = new QTimer(this);
    m_loadLedColorTimer->setSingleShot(true);
    connect(m_loadLedColorTimer, &QTimer::timeout, this, &Kameleon::loadLedColor);
    connect(&m_fsWatcher, &QFileSystemWatcher::fileChanged, this, [this]() {
        m_fsWatcher.blockSignals(true);
        m_loadLedColorTimer->start(3000);
    });

    loadConfig();
    connect(m_configWatcher.get(), &KConfigWatcher::configChanged, this, &Kameleon::updateAccentColor);
}

void Kameleon::findRgbLedDevices()
{
    // See https://github.com/torvalds/linux/blob/master/Documentation/leds/leds-class-multicolor.rst
    QDir ledsDir(LED_SYSFS_PATH);
    ledsDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);
    auto ledDevices = ledsDir.entryList();

    // Sort to ensure keys light up nicely one next to the other
    QCollator coll;
    coll.setNumericMode(true);
    std::sort(ledDevices.begin(), ledDevices.end(), [&](const QString &s1, const QString &s2) {
        return coll.compare(s1, s2) < 0;
    });

    for (const QString &ledDevice : std::as_const(ledDevices)) {
        // Get multicolor index (= RGB capability with order of colors)
        QFile indexFile(LED_SYSFS_PATH + ledDevice + LED_INDEX_FILE);
        if (!QFileInfo(indexFile).exists()) {
            // Not a RGB capable device
            continue;
        }
        if (!indexFile.open(QIODevice::ReadOnly)) {
            qCWarning(KAMELEON) << "failed to open" << indexFile.fileName() << indexFile.error() << indexFile.errorString();
            continue;
        }
        QString colorIndexStr = indexFile.readAll().trimmed();
        indexFile.close();
        QString colorIndex =
            colorIndexStr.toLower().replace("red", "r").replace("green", "g").replace("blue", "b").replace(" ", ""); // eg "red green blue" -> "rgb"
        if (!(colorIndex.length() == 3 && colorIndex.contains("r") && colorIndex.contains("g") && colorIndex.contains("b"))) {
            qCWarning(KAMELEON) << "invalid color index" << colorIndexStr << "read from" << LED_INDEX_FILE << "for device" << ledDevice;
            continue;
        }

        qCInfo(KAMELEON) << "found RGB LED device" << ledDevice;
        m_rgbLedDevices.append(ledDevice);
        m_deviceRgbIndices.append(colorIndex);
        m_fsWatcher.addPath(LED_SYSFS_PATH + ledDevice + LED_RGB_FILE);
    }
}

bool Kameleon::isSupported()
{
    return !m_rgbLedDevices.isEmpty();
}

void Kameleon::loadLedColor()
{
    qCInfo(KAMELEON) << "load led color";
    QColor currentColor;
    for (const QString &ledDevice : std::as_const(m_rgbLedDevices)) {
        QFile intensityFile(LED_SYSFS_PATH + ledDevice + LED_RGB_FILE);
        if (!QFileInfo(intensityFile).exists()) {
            qCWarning(KAMELEON) << "failed to read from" << intensityFile.fileName() << "file does not exist";
            continue;
        }
        if (!intensityFile.open(QIODevice::ReadOnly)) {
            qCWarning(KAMELEON) << "failed to read from" << intensityFile.fileName() << intensityFile.error() << intensityFile.errorString();
            continue;
        }
        QString colorIndex = m_deviceRgbIndices.at(m_rgbLedDevices.indexOf(ledDevice));
        QString deviceColorStr = intensityFile.readAll().trimmed();
        intensityFile.close();
        QStringList deviceColorLst = deviceColorStr.split(" ");
        if (deviceColorLst.length() != colorIndex.length()) {
            qCWarning(KAMELEON) << "invalid color intensity" << deviceColorStr << "read from" << LED_RGB_FILE << "for device" << ledDevice;
            continue;
        }
        int red = deviceColorLst.at(colorIndex.indexOf("r")).toInt();
        int green = deviceColorLst.at(colorIndex.indexOf("g")).toInt();
        int blue = deviceColorLst.at(colorIndex.indexOf("b")).toInt();
        QColor deviceColor = QColor::fromRgb(qRgb(red, green, blue));
        if (!deviceColor.isValid()) {
            qCWarning(KAMELEON) << "invalid color" << deviceColorStr << "read from" << LED_RGB_FILE << "for device" << ledDevice;
            continue;
        }
        if (!currentColor.isValid()) {
            currentColor = deviceColor;
        } else if (currentColor != deviceColor) {
            qCWarning(KAMELEON) << "different colors found on multiple devices; treating as white";
            currentColor = QColor(QColorConstants::White);
        }
        if (currentColor != m_currentColor) {
            qCInfo(KAMELEON) << "led color changed" << currentColor.name();
            m_currentColor = currentColor;
            Q_EMIT activeColorChanged(currentColor.name());

            if (m_accent) {
                qCInfo(KAMELEON) << "accent changed" << false;
                m_config->group("General").writeEntry<bool>(CONFIG_KEY_ACCENT, false);
                m_config->sync();
                m_accent = false;
                Q_EMIT accentChanged(false);
            }
        }
    }
    m_fsWatcher.blockSignals(false);
}

void Kameleon::loadConfig()
{
    qCDebug(KAMELEON) << "load color config";

    qCInfo(KAMELEON) << "current color" << m_currentColor.name();
    m_targetColor = m_currentColor;

    bool accent = m_config->group("General").readEntry<bool>(CONFIG_KEY_ACCENT, true);
    if (accent != m_accent) {
        qCInfo(KAMELEON) << "accent syncing configured" << accent;
        m_accent = accent;
    }
    if (m_accent) {
        updateAccentColor();
        return;
    }

    QColor color = m_config->group("General").readEntry<QColor>(CONFIG_KEY_COLOR, QColor(QColorConstants::White));
    if (color != m_customColor) {
        qCInfo(KAMELEON) << "color configured" << color.name();
        m_customColor = color;
    }
    // Don't enforce applying custom color on startup, to provide a way of opting out of Plasma meddling with device LEDs.
    // If nothing else changed it in the meantime, the last set custom color will still be in effect after a reboot.
    // If it did get changed by an external source, leave it until a custom color is set again explicitly through the frontend.
}

void Kameleon::updateAccentColor()
{
    if (!m_accent) {
        return;
    }
    qCDebug(KAMELEON) << "load accent color";
    QColor customAccentColor = m_config->group("General").readEntry<QColor>("AccentColor", QColor::Invalid);
    QColor schemeAccentColor = m_config->group("Colors:View").readEntry<QColor>("ForegroundActive", QColor::Invalid);
    m_accentColor = customAccentColor.isValid() ? customAccentColor : schemeAccentColor.isValid() ? schemeAccentColor : QColor(QColorConstants::White);

    if (m_accentColor != m_targetColor) {
        qCInfo(KAMELEON) << "accent color changed" << m_accentColor.name();
        applyColor(m_accentColor);
    }
}

void Kameleon::updateCustomColor()
{
    if (m_accent) {
        return;
    }
    if (m_customColor != m_targetColor) {
        qCInfo(KAMELEON) << "custom color changed" << m_customColor.name();
        applyColor(m_customColor);
    }
}

bool Kameleon::isAccent()
{
    return m_accent;
}

QString Kameleon::currentColor()
{
    return m_currentColor.name();
}

void Kameleon::setAccent(bool enabled)
{
    if (enabled != m_accent) {
        qCInfo(KAMELEON) << "setting accent syncing" << enabled;
        m_config->group("General").writeEntry<bool>(CONFIG_KEY_ACCENT, enabled);
        m_config->sync();
        m_accent = enabled;
    }
    if (enabled) {
        updateAccentColor();
    } else {
        setColor(QColorConstants::White.name());
    }
    Q_EMIT accentChanged(enabled);
}

void Kameleon::setColor(QString colorName)
{
    QColor color = QColor(colorName);
    if (!color.isValid()) {
        qCWarning(KAMELEON) << "invalid color" << colorName;
        return;
    }

    if (m_accent) {
        qCInfo(KAMELEON) << "setting accent syncing false";
        m_config->group("General").writeEntry<bool>(CONFIG_KEY_ACCENT, false);
        m_config->sync();
        m_accent = false;
    }

    if (color != m_customColor) {
        qCInfo(KAMELEON) << "setting color" << colorName;
        m_config->group("General").writeEntry<QColor>(CONFIG_KEY_COLOR, colorName);
        m_config->sync();
        m_customColor = color;
    }
    updateCustomColor();
}

void Kameleon::applyColor(QColor color)
{
    m_targetColor = color;
    QStringList colorStrs;
    for (const QString &colorIndex : std::as_const(m_deviceRgbIndices)) {
        QStringList colorStrList = {QString(), QString(), QString()};
        colorStrList[colorIndex.indexOf("r")] = QString::number(color.red());
        colorStrList[colorIndex.indexOf("g")] = QString::number(color.green());
        colorStrList[colorIndex.indexOf("b")] = QString::number(color.blue());
        QString colorStr = colorStrList.join(" ");
        colorStrs.append(colorStr);
    }

    KAuth::Action action("org.kde.kameleonhelper.writecolor");
    action.setHelperId("org.kde.kameleonhelper");
    action.addArgument("devices", m_rgbLedDevices);
    action.addArgument("colors", colorStrs);
    auto *job = action.execute();

    connect(job, &KAuth::ExecuteJob::result, this, [this, job, color]() {
        m_fsWatcher.blockSignals(false);
        if (job->error()) {
            qCWarning(KAMELEON) << "failed to write color to devices" << job->errorText();
            return;
        }
        qCInfo(KAMELEON) << "wrote color" << color.name() << "to LED devices";
        m_currentColor = color;
        Q_EMIT activeColorChanged(color.name());
    });
    m_fsWatcher.blockSignals(true); // Don't fire loadLedColor() while we're writing a color to the devices ourselves
    job->start();
}

#include "kameleon.moc"
