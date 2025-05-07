/*
 * SPDX-FileCopyrightText: 2025 Christoph Wolk <cwo.kde@posteo.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.plasma.extras as PlasmaExtras

PlasmaExtras.PlaceholderMessage {
    iconName: "error-symbolic"
    text: i18nc("@info placeholdermessage if comic loading failed", "Could not load comic")
    explanation: i18nc("@info placeholdermessage explanation", "Try again later, or choose a different comic")
}
