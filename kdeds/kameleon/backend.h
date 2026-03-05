// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

class QColor;

class Backend : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    [[nodiscard]] virtual bool isSupported() = 0;
    virtual void applyColor(const QColor &color) = 0;

Q_SIGNALS:
    void supportedChanged();
};
