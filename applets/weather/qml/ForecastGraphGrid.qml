import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

Item {
    id: root

    property int horizontalLinesNumber: 0
    property int verticalLinesNumber: 0

    Repeater {
        model: root.verticalLinesNumber

        delegate: Rectangle {
            required property int index

            color: Kirigami.Theme.activeBackgroundColor

            width: Kirigami.Units.gridUnit * 0.1
            height: root.height

            x: {
                const ratio = index / (root.verticalLinesNumber - 1);
                return ratio * root.width - width / 2;
            }
        }
    }

    Repeater {
        model: root.horizontalLinesNumber

        delegate: Rectangle {
            required property int index

            color: Kirigami.Theme.activeBackgroundColor

            width: root.width
            height: Kirigami.Units.gridUnit * 0.1

            y: {
                const ratio = index / (root.horizontalLinesNumber - 1);
                return ratio * root.height - height / 2;
            }
        }
    }
}
