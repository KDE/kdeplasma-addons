/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "sysfs.h"
#include "kameleon_debug.h"

#include <KAuth/Action>
#include <KAuth/ActionReply>
#include <KAuth/ExecuteJob>
#include <KAuth/HelperSupport>

#include <QCollator>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QString>

using namespace Qt::StringLiterals;

namespace
{
constexpr auto LED_SYSFS_PATH = "/sys/class/leds/"_L1;
constexpr auto LED_INDEX_FILE = "/multi_index"_L1;
constexpr auto LED_RGB_FILE = "/multi_intensity"_L1;
} // namespace

Sysfs::Sysfs(QObject *parent)
    : Backend(parent)
{
    // TODO this really should be non-blocking
    findRgbLedDevices();
}

void Sysfs::findRgbLedDevices()
{
    QDir ledsDir(LED_SYSFS_PATH);
    ledsDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);
    auto ledDevices = ledsDir.entryList();

    // Sort to ensure keys light up nicely one next to the other
    QCollator coll;
    coll.setNumericMode(true);
    std::ranges::sort(ledDevices, [&](const QString &s1, const QString &s2) {
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
        QString colorIndexStr = QString::fromLocal8Bit(indexFile.readAll()).trimmed();
        indexFile.close();
        QString colorIndex =
            colorIndexStr.toLower().replace("red"_L1, "r"_L1).replace("green"_L1, "g"_L1).replace("blue"_L1, "b"_L1).replace(" "_L1, ""_L1); // eg "red green
                                                                                                                                             // blue"
                                                                                                                                             // -> "rgb"
        if (!(colorIndex.length() == 3 && colorIndex.contains("r"_L1) && colorIndex.contains("g"_L1) && colorIndex.contains("b"_L1))) {
            qCWarning(KAMELEON) << "invalid color index" << colorIndexStr << "read from" << LED_INDEX_FILE << "for device" << ledDevice;
            continue;
        }

        qCDebug(KAMELEON) << "found RGB LED device" << ledDevice;
        m_rgbLedDevices.append(ledDevice);
        m_deviceRgbIndices.append(colorIndex);
    }

    Q_EMIT supportedChanged();
}

bool Sysfs::isSupported()
{
    return !m_rgbLedDevices.isEmpty();
}

void Sysfs::applyColor(const QColor &color)
{
    QStringList colorStrs;
    for (const QString &colorIndex : std::as_const(m_deviceRgbIndices)) {
        QStringList colorStrList = {QString(), QString(), QString()};
        colorStrList[colorIndex.indexOf('r'_L1)] = QString::number(color.red());
        colorStrList[colorIndex.indexOf('g'_L1)] = QString::number(color.green());
        colorStrList[colorIndex.indexOf('b'_L1)] = QString::number(color.blue());
        QString colorStr = colorStrList.join(' '_L1);
        colorStrs.append(colorStr);
    }

    KAuth::Action action(u"org.kde.kameleonhelper.writecolor"_s);
    action.setHelperId(u"org.kde.kameleonhelper"_s);
    action.addArgument(u"devices"_s, m_rgbLedDevices);
    action.addArgument(u"colors"_s, colorStrs);
    auto *job = action.execute();

    connect(job, &KAuth::ExecuteJob::result, this, [job, color] {
        if (job->error()) {
            qCWarning(KAMELEON) << "failed to write color to devices" << job->errorText();
            return;
        }
        qCDebug(KAMELEON) << "wrote color" << color.name() << "to LED devices";
    });
    job->start();
}
