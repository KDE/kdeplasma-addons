/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "utils.h"

bool Utils::isValidColor(const QString &colorString) const
{
    return QColor::isValidColorName(colorString);
}
