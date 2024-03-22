/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kameleonhelper.h"
#include "kameleonhelper_debug.h"

#include <KAuth/HelperSupport>

#include <QDebug>
#include <QFile>
#include <QFileInfo>

KAuth::ActionReply KameleonHelper::writecolor(const QVariantMap &args)
{
    QMap<QString, QVariant> devices = args.value(QStringLiteral("entries")).toMap();
    for (auto i = devices.cbegin(), end = devices.cend(); i != end; ++i) {
        QString device = i.key();
        QByteArray color = i.value().toByteArray();
        if (color.length() != 3) {
            qCWarning(KAMELEONHELPER) << "invalid RGB color" << color << "for device" << device;
            return KAuth::ActionReply::HelperErrorReply();
        }

        QFile file(LED_SYSFS_PATH + device + LED_RGB_FILE);
        if (!QFileInfo(file).exists()) {
            qCWarning(KAMELEONHELPER) << "writing to" << file.fileName() << "failed:"
                                      << "file does not exist";
            return KAuth::ActionReply::HelperErrorReply();
        }
        if (!QFileInfo(file).isWritable()) {
            qCWarning(KAMELEONHELPER) << "writing to" << file.fileName() << "failed:"
                                      << "file is not writable";
            return KAuth::ActionReply::HelperErrorReply();
        }
        if (!file.open(QIODevice::WriteOnly)) {
            qCWarning(KAMELEONHELPER) << "writing to" << file.fileName() << "failed:" << file.error() << file.errorString();
            return KAuth::ActionReply::HelperErrorReply();
        }
        const int bytesWritten = file.write(color);
        if (bytesWritten == -1) {
            qCWarning(KAMELEONHELPER) << "writing to" << file.fileName() << "failed:" << file.error() << file.errorString();
            return KAuth::ActionReply::HelperErrorReply();
        } else {
            qCInfo(KAMELEONHELPER) << "wrote color to" << file.fileName();
        }
    }

    return KAuth::ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.kameleonhelper", KameleonHelper)

#include "moc_kameleonhelper.cpp"
