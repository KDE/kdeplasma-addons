/*
 * SPDX-FileCopyrightText: 2025 Christoph Wolk <cwo.kde@posteo.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtNetwork as QtNetwork
import org.kde.plasma.extras as PlasmaExtras
import plasma.applet.org.kde.plasma.comic as Comic

PlasmaExtras.PlaceholderMessage {
    id: errorPlaceholder
    required property string identifier
    required property int identifierType

    iconName: "error-symbolic"
    text: i18nc("@info placeholdermessage if comic loading failed", "Could not load comic") // qmllint disable unqualified
    explanation: i18nc("@info placeholdermessage explanation", "Try again later, or choose a different comic") // qmllint disable unqualified

    states: [
        State {
            name: "offline"
            when: QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Disconnected
            PropertyChanges {
                errorPlaceholder.iconName: "offline-symbolic"
                errorPlaceholder.text: i18nc("@info placeholdermessage title if offline", "Offline") // qmllint disable unqualified
                errorPlaceholder.explanation: i18nc("@info placeholdermessage connection", "Reconnect to the internet to retry loading this image. You can still view any images in the local archive.") // qmllint disable unqualified
            }
        },
        State {
            name: "possibly-offline"
            when: QtNetwork.NetworkInformation.reachability !== QtNetwork.NetworkInformation.Reachability.Online
            PropertyChanges {
                errorPlaceholder.iconName: "offline-symbolic"
                errorPlaceholder.text: i18nc("@info placeholdermessage title if connection local/unknown", "Limited connectivity") // qmllint disable unqualified
                errorPlaceholder.explanation: i18nc("@info placeholdermessage explanation if connection local/unknown", "The image may not be available on your current connection. You can still view any images in the local archive.") // qmllint disable unqualified
            }
        },
        State {
            name: "today-not-available"
            when: errorPlaceholder.identifierType === Comic.comicData.DateIdentifier && errorPlaceholder.identifier == ""
            PropertyChanges {
                errorPlaceholder.iconName: "error-symbolic"
                errorPlaceholder.text: i18nc("@info placeholdermessage title if today not available", "Not available") // qmllint disable unqualified
                errorPlaceholder.explanation: i18nc("@info placeholdermessage explanation if today not available", "Today's image is not yet available or cannot be read. Try a different date.") // qmllint disable unqualified
            }
        },
        State {
            name: "date-not-available"
            when: errorPlaceholder.identifierType === Comic.comicData.DateIdentifier
            PropertyChanges {
                errorPlaceholder.iconName: "error-symbolic"
                errorPlaceholder.text: i18nc("@info placeholdermessage title if date not available", "Not available") // qmllint disable unqualified
                errorPlaceholder.explanation: i18nc("@info placeholdermessage explanation if date not available", "The image for this day is not available or cannot be read. Try a different date.") // qmllint disable unqualified
            }
        }
    ]
}
