// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#pragma once

#include "backend.h"

class QMK : public Backend
{
    Q_OBJECT
public:
    QMK(QObject *parent = nullptr);

    bool isSupported() override;
    void applyColor(const QColor &color) override;

private:
    bool m_supported = false;
};
