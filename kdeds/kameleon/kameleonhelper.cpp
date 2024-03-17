/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kameleonhelper.h"
#include "kameleonhelper_debug.h"

#include <KAuth/HelperSupport>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

KAuth::ActionReply KameleonHelper::writecolor(const QVariantMap &args)
{
    Q_UNUSED(args);
    const qint64 uid = QCoreApplication::applicationPid();
    qDebug() << "executing uid=" << uid;

    QByteArray color = args.value(QStringLiteral("color")).toByteArray();
    QStringList devices = args.value(QStringLiteral("devices")).toStringList();

    qCInfo(KAMELEONHELPER) << "writing color" << color << "to LED devices";

    for (const QString &device : devices) {
        QFile file(device);
        if (!QFileInfo(file).exists()) {
            qCWarning(KAMELEONHELPER) << "writing to " << device << "failed:"
                                      << "file does not exist";
            continue;
        }
        if (!QFileInfo(file).isWritable()) {
            qCWarning(KAMELEONHELPER) << "writing to " << device << "failed:"
                                      << "file is not writable";
            continue;
        }
        if (!file.open(QIODevice::WriteOnly)) {
            qCWarning(KAMELEONHELPER) << "writing to " << device << "failed:" << file.error() << file.errorString();
            continue;
        }
        const int bytesWritten = file.write(color);
        if (bytesWritten == -1) {
            qCWarning(KAMELEONHELPER) << "writing to " << device << "failed:" << file.error() << file.errorString();
            continue;
        } else {
            qCInfo(KAMELEONHELPER) << "wrote accent color to" << device;
        }
    }

    return KAuth::ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.kameleonhelper", KameleonHelper)

#include "moc_kameleonhelper.cpp"
