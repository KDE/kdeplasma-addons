/*
 *    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
 *    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QIcon>
#include <QObject>
#include <qqmlregistration.h>

class NotesHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit NotesHelper(QObject *parent = nullptr);

    ~NotesHelper() override = default;

    Q_INVOKABLE QString fileContents(const QString &path) const;

    Q_INVOKABLE QIcon noteIcon(const QString &color) const;
};
