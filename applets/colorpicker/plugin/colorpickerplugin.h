/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QColor>
#include <QObject>
#include <qqmlregistration.h>

class Utils : public QObject
{
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
    Q_DISABLE_COPY_MOVE(Utils)

public:
    explicit Utils(QObject *parent = nullptr);

    Q_INVOKABLE QColor fromRGBA(QRgb value) const;
};
