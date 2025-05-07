/*
 * SPDX-FileCopyrightText: 2025 Christoph Wolk <cwo.kde@posteo.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtNetwork as QtNetwork
import org.kde.plasma.extras as PlasmaExtras

PlasmaExtras.PlaceholderMessage {
    id: errorPlaceholder
    iconName: "error-symbolic"
    text: i18nc("@info placeholdermessage if comic loading failed", "Could not load comic")
    explanation: i18nc("@info placeholdermessage explanation", "Try again later, or choose a different comic")

    states: [
        State {
            name: "offline"
            when: QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Disconnected
            PropertyChanges {
                errorPlaceholder.iconName: "offline-symbolic"
                errorPlaceholder.text: i18nc("@info placeholdermessage title if offline", "Offline")
                errorPlaceholder.explanation: i18nc("@info placeholdermessage connection", "Reconnect to the internet to retry loading this image. You can still view any images in the local archive.")
            }
        },
        State {
            name: "possibly-offline"
            when: QtNetwork.NetworkInformation.reachability !== QtNetwork.NetworkInformation.Reachability.Online
            PropertyChanges {
                errorPlaceholder.iconName: "offline-symbolic"
                errorPlaceholder.text: i18nc("@info placeholdermessage title if connection local/unknown", "Limited connectivity")
                errorPlaceholder.explanation: i18nc("@info placeholdermessage explanation if connection local/unknown", "The image may not be available on your current connection. You can still view any images in the local archive.")
            }
        }
    ]
}
