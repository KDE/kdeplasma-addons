/*
 *    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
 *    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "noteshelper.h"

#include <QFile>
#include <QUrl>

NotesHelper::NotesHelper(QObject *parent)
    : QObject(parent)
{
}
QString NotesHelper::fileContents(const QString &path) const
{
    const QUrl &url = QUrl::fromUserInput(path);
    if (!url.isValid()) {
        return QString();
    }

    QFile file(url.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    return QString::fromUtf8(file.readAll());
}
