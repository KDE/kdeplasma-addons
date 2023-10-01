/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colorblindnesscorrection.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED(ColorBlindnessCorrectionEffect, "metadata.json", return ColorBlindnessCorrectionEffect::supported();)

} // namespace KWin

#include "main.moc"
