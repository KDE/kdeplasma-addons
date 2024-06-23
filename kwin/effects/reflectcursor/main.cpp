/*
    SPDX-FileCopyrightText: 2024 Yifan Zhu <fanzhuyifan@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "reflectcursor.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED(ReflectCursorEffect, "metadata.json", return ReflectCursorEffect::supported();)

} // namespace KWin

#include "main.moc"
