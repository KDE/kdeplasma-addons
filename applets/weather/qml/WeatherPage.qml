import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

ColumnLayout {
    id: root

    property int status: 0

    property int invalidUnit: 0
    property int displaySpeedUnit: 0
    property int displayPressureUnit: 0
    property int displayTemperatureUnit: 0
    property int displayVisibilityUnit: 0

    property var station: null
    property var futureHours: null
    property var futureDays: null
    property var warnings: null
    property var lastObservation: null
    property var metaData: null

    signal openWarnings

    Layout.margins: Kirigami.Units.smallSpacing

    TopPanel {
        id: topPanel
        visible: (!!root.station || !!root.lastObservation) && root.status === ForecastControl.Normal

        station: root.station
        lastObservation: root.lastObservation
        metaData: root.metaData
        warnings: root.warnings
        futureDays: root.futureDays

        invalidUnit: root.invalidUnit
        displayPressureUnit: root.displayPressureUnit
        displaySpeedUnit: root.displaySpeedUnit
        displayVisibilityUnit: root.displayVisibilityUnit
        displayTemperatureUnit: root.displayTemperatureUnit

        Layout.fillWidth: true
        // Allow the top panel to vertically grow but within a limit
        Layout.fillHeight: true
        Layout.maximumHeight: implicitHeight * 1.5

        onOpenWarnings: root.openWarnings()
    }

    SwitchPanel {
        id: switchPanel
        visible: root.status === ForecastControl.Normal
        Layout.fillWidth: true

        futureDays: root.futureDays
        futureHours: root.futureHours
        warnings: root.warnings
        lastObservation: root.lastObservation
        metaData: root.metaData

        invalidUnit: root.invalidUnit
        displayPressureUnit: root.displayPressureUnit
        displaySpeedUnit: root.displaySpeedUnit
        displayVisibilityUnit: root.displayVisibilityUnit
        displayTemperatureUnit: root.displayTemperatureUnit
    }

    PlasmaComponents.Label {
        id: sourceLabel
        visible: root.status === ForecastControl.Normal

        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
            cursorShape: !!metaData?.credit ? Qt.PointingHandCursor : Qt.ArrowCursor
        }

        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignRight
        font.pointSize: Kirigami.Theme.smallFont.pointSize
        linkColor: color
        opacity: 0.75
        textFormat: Text.StyledText

        text: {
            let result = "";
            if (!!metaData?.credit) {
                if (!!metaData.creditURL) {
                    result = "<a href=\"" + root.metaData.creditURL + "\">" + root.metaData.credit + "</a>";
                } else {
                    result = root.metaData.credit;
                }
            }
            return result;
        }

        onLinkActivated: link => {
            Qt.openUrlExternally(link);
        }
    }
}
