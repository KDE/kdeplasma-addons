/*
 *  SPDX-FileCopyrightText: 2008-2009 Lukas Appelhans <l.appelhans@gmx.de>
 *  SPDX-FileCopyrightText: 2010-2011 Ingomar Wesp <ingomar@wesp.name>
 *  SPDX-FileCopyrightText: 2013 Bhushan Shah <bhush94@gmail.com>
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "quicklaunch_p.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRandomGenerator>
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KFileItem>
#include <KIO/CommandLauncherJob>
#include <KIO/OpenUrlJob>
#include <KNotificationJobUiDelegate>
#include <KOpenWithDialog>
#include <KPropertiesDialog>

#include <kio/global.h>

QuicklaunchPrivate::QuicklaunchPrivate(QObject *parent)
    : QObject(parent)
{
}

void QuicklaunchPrivate::openLauncherDialog(bool isPopup)
{
    KOpenWithDialog *dialog = new KOpenWithDialog();
    dialog->setModal(false);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->hideRunInTerminal();
    dialog->setSaveNewApplications(true);
    dialog->show();

    connect(dialog, &KOpenWithDialog::accepted, this, [this, dialog, isPopup]() {
        if (!dialog->service()) {
            return;
        }
        const QUrl &url = QUrl::fromLocalFile(dialog->service()->entryPath());
        if (url.isValid()) {
            Q_EMIT launcherAccepted(url.toString(), isPopup);
        }
    });
}

