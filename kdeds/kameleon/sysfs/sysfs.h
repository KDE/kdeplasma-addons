/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "backend.h"

class Sysfs : public Backend
{
public:
    Sysfs(QObject *parent = nullptr);
    bool isSupported() override;
    void applyColor(const QColor &color) override;

private:
    void findRgbLedDevices();
    QStringList m_rgbLedDevices;
    QStringList m_deviceRgbIndices;
};
