/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef QUICKLAUNCH_P_H
#define QUICKLAUNCH_P_H

#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <qqmlregistration.h>

class QuicklaunchPrivate : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Logic)

public:
    explicit QuicklaunchPrivate(QObject *parent = nullptr);

    Q_INVOKABLE void openLauncherDialog(bool isPopup = false);

Q_SIGNALS:
    void launcherAccepted(const QString &url, bool isPopup);
};

#endif // QUICKLAUNCH_P_H
