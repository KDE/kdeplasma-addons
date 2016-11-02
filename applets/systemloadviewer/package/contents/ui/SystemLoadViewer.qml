/*
 * Copyright (C) 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 * Copyright (C) 2015 Joshua Worth <joshua@worth.id.au>
 * Copyright (C) 2015 Kåre Särs <kae.sars@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kio 1.0 as Kio
import org.kde.plasma.plasmoid 2.0

Item {
    id: main

    readonly property double maxCpuLoad: 100.0
    readonly property int borderWidth: 1
    readonly property int borderRounding: 3
    readonly property int headingLevel: 2

    property bool setColorsManually: plasmoid.configuration.setColorsManually

    property var cpuColors: setColorsManually ? [plasmoid.configuration.cpuSysColor,
                                                 plasmoid.configuration.cpuUserColor,
                                                 plasmoid.configuration.cpuNiceColor,
                                                 plasmoid.configuration.cpuIOWaitColor]
                                              : [theme.buttonFocusColor,
                                                 theme.highlightColor,
                                                 theme.visitedLinkColor,
                                                 theme.linkColor]

    property var memoryColors: setColorsManually ? [plasmoid.configuration.memApplicationColor,
                                                    plasmoid.configuration.memBuffersColor,
                                                    plasmoid.configuration.memCachedColor]
                                                 : [theme.buttonFocusColor,
                                                    theme.visitedLinkColor,
                                                    theme.highlightColor]

    property var swapColors: setColorsManually ? [plasmoid.configuration.swapUsedColor]
                                               : [theme.hightlightColor]

    // Make labels visible first time so that the
    // user knows which monitor is which.
    implicitWidth: widestLabelWidth()*1.3 * columnCount()
    implicitHeight: implicitWidth / 2

    property bool circularMonitorsInUse: plasmoid.configuration.monitorType == 1
    property bool compactBarMonitorsInUse: plasmoid.configuration.monitorType == 2
    property bool allCPUsShown: plasmoid.configuration.cpuAllActivated

    property double barsWidth: compactBarMonitorsInUse ? main.height * 0.35 * columnCount()
                                                       : main.height * 0.5 * columnCount()


    // Don't show icon in panel.
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    // Correct the size when in panel
    Layout.preferredWidth: {
        if (circularMonitorsInUse) {
            return parent.height * 1.2 * columnCount()
        }
        else if (!plasmoid.configuration.cpuAllActivated) {
            return barsWidth + rowLayout.spacing * (columnCount() - 1) // 0 * x == 0 (compact)
        }
        // else plasmoid.configuration.cpuAllActivated
        var wantedWidth = cpusRow.minWidth
        if (plasmoid.configuration.swapActivated) {
            wantedWidth += memColumn.minWidth
        }
        if (plasmoid.configuration.memoryActivated) {
            wantedWidth += swapColumn.minWidth
        }
        return wantedWidth
    }

    Layout.minimumWidth: {
        if (circularMonitorsInUse) {
            return units.gridUnit * columnCount() * 2
        }
        else if (!plasmoid.configuration.cpuAllActivated) {
            return memColumn.minWidth * columnCount() + rowLayout.spacing * (columnCount() - 1)
        }
        var wantedWidth = cpusRow.minWidth
        if (plasmoid.configuration.swapActivated) {
            wantedWidth += memColumn.minWidth
        }
        if (plasmoid.configuration.memoryActivated) {
            wantedWidth += swapColumn.minWidth
        }
        return wantedWidth
    }

    property bool labelsVisible

    Kio.KRun { id: kRun }

    // We need to get the full path to KSysguard to be able to run it
    PlasmaCore.DataSource {
        id: apps
        engine: "apps"
        connectedSources: ["org.kde.ksysguard.desktop"]
    }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "systemmonitor"

        property string cpuSystem: "cpu/system/"
        property string niceLoad: cpuSystem + "nice"
        property string userLoad: cpuSystem + "user"
        property string sysLoad: cpuSystem + "sys"
        property string ioWait: cpuSystem + "wait"
        property string averageClock: cpuSystem + "AverageClock"
        property string totalLoad: cpuSystem + "TotalLoad"
        property string memPhysical: "mem/physical/"
        property string memFree: memPhysical + "free"
        property string memApplication: memPhysical + "application"
        property string memBuffers: memPhysical + "buf"
        property string memCached: memPhysical + "cached"
        property string memUsed: memPhysical + "used"
        property string swap: "mem/swap/"
        property string swapUsed: swap + "used"
        property string swapFree: swap + "free"

        property var totalCpuLoadProportions: [.0, .0, .0, .0]
        property int maxCpuIndex: 0
        property var memoryUsageProportions: [.0, .0, .0]
        property double swapUsageProportion: .0

        connectedSources: [niceLoad, userLoad, sysLoad,
            ioWait, memFree, memApplication, memBuffers,
            memCached, memUsed, swapUsed, swapFree,
            averageClock, totalLoad]

        onSourceAdded: {
            var match = source.match(/^cpu\/cpu(\w+)\//)
            if (match) {
                connectSource(source)
                if (maxCpuIndex < match[1]) {
                    maxCpuIndex = match[1]
                }
            }
        }

        onNewData: {
            if (sourceName == sysLoad) {
                totalCpuLoadProportions[0] = fitCpuLoad(data.value)
            }
            else if (sourceName == userLoad) {
                totalCpuLoadProportions[1] = fitCpuLoad(data.value)
            }
            else if (sourceName == niceLoad) {
                totalCpuLoadProportions[2] = fitCpuLoad(data.value)
            }
            else if (sourceName == ioWait) {
                totalCpuLoadProportions[3] = fitCpuLoad(data.value)
                totalCpuLoadProportionsChanged()
            }
            else if (sourceName == memApplication) {
                memoryUsageProportions[0] = fitMemoryUsage(data.value)
            }
            else if (sourceName == memBuffers) {
                memoryUsageProportions[1] = fitMemoryUsage(data.value)
            }
            else if (sourceName == memCached) {
                memoryUsageProportions[2] = fitMemoryUsage(data.value)
                memoryUsageProportionsChanged()
            }
            else if (sourceName == swapUsed) {
                swapUsageProportion = fitSwapUsage(data.value)
                swapUsageProportionChanged()
            }
        }
        interval: 1000 * plasmoid.configuration.updateInterval
    }

    onWidthChanged: labelsVisible = shouldLabelsBeVisible()
    onAllCPUsShownChanged: labelsVisible = shouldLabelsBeVisible()

    function toolTipSubText() {
        var cpuLoadPart = "";
        var cpuClockPart = "";
        if (plasmoid.configuration.cpuAllActivated) {
            for (var i=0; i<=dataSource.maxCpuIndex; i++) {
                if (i>0) {
                    cpuLoadPart += "\n"
                }
                var totalName = "cpu/cpu" + i + "/TotalLoad"
                var clockName = "cpu/cpu" + i + "/clock"
                var totValue = dataSource.data[totalName] ? dataSource.data[totalName].value : 0
                var clockValue = dataSource.data[clockName] ? dataSource.data[clockName].value : 0
                cpuLoadPart += i18n("CPU%1: %2% @ %3 Mhz", i,
                                    Math.round(totValue),
                                    Math.round(clockValue))
            }
        }
        else {
            var cpuLoad = dataSource.data[dataSource.totalLoad] ? dataSource.data[dataSource.totalLoad].value : 0
            var averageClock = dataSource.data[dataSource.averageClock] ? dataSource.data[dataSource.averageClock].value : 0
            cpuLoadPart = i18n("CPU: %1%", Math.round(cpuLoad))
            cpuClockPart = i18n("Average clock: %1 MHz", Math.round(averageClock))
        }
        var memFree = parseFloat(dataSource.data[dataSource.memFree] ? dataSource.data[dataSource.memFree].value : 0) / 1024
        var memUsed = parseFloat(dataSource.data[dataSource.memUsed] ? dataSource.data[dataSource.memUsed].value : 0) / 1024
        var memApplication = parseFloat(dataSource.data[dataSource.memApplication] ? dataSource.data[dataSource.memApplication].value : 0) / 1024
        var memTotal = memFree + memUsed
        var memoryPart = i18n("Memory: %1/%2 MiB", Math.round(memApplication), Math.round(memTotal))

        var swapFree = parseFloat(dataSource.data[dataSource.swapFree] ? dataSource.data[dataSource.swapFree].value : 0) / 1024
        var swapUsed = parseFloat(dataSource.data[dataSource.swapUsed] ? dataSource.data[dataSource.swapUsed].value : 0) / 1024
        var swapTotal = swapFree + swapUsed
        var swapPart = i18n("Swap: %1/%2 MiB", Math.round(swapUsed), Math.round(swapTotal))

        if (cpuClockPart === "") {
            return [cpuLoadPart, memoryPart, swapPart].join("\n")
        }

        return [cpuLoadPart, cpuClockPart, memoryPart, swapPart].join("\n")
    }

    function fitCpuLoad(load) {
        var x = load / maxCpuLoad;
        if (isNaN(x)) {return 0;}
        return Math.min(x, 1); // Ensure that we do not get values that might cause problems
    }

    function fitMemoryUsage(usage) {
        var x = (usage / (parseFloat(dataSource.data[dataSource.memFree].value) +
                         parseFloat(dataSource.data[dataSource.memUsed].value)))
        if (isNaN(x)) {return 0;}
        return Math.min(x, 1);
    }

    function fitSwapUsage(usage) {
        var x = (usage / (parseFloat(usage) + parseFloat(dataSource.data[dataSource.swapFree].value)))

        if (isNaN(x)) {return 0;}
        return Math.min(x, 1);
    }

    function columnCount() {
        var columns = 0;
        var activeMonitors = [plasmoid.configuration.cpuActivated,
            plasmoid.configuration.memoryActivated,
            plasmoid.configuration.swapActivated]

        for (var i = 0; i < activeMonitors.length; i++) {
            if (activeMonitors[i]) {
                columns++;
            }
        }
        return columns
    }

    function columnWidth(compact) {
        var columns = columnCount();
        var width = rowLayout.width / columns;
        if (!compact) {
            width -= rowLayout.spacing * (columns - 1);
        }
        return width;
    }

    function widestLabelWidth() {
        var widest = 0;
        if (plasmoid.configuration.cpuActivated) {
            widest = cpuLabel.paintedWidth
        }
        if (plasmoid.configuration.memoryActivated) {
            widest = Math.max(widest, memoryLabel.paintedWidth)
        }
        if (plasmoid.configuration.swapActivated) {
            widest = Math.max(widest, swapLabel.paintedWidth)
        }
        return widest
    }

    // Hide all labels when one of them is wider than the column width
    function shouldLabelsBeVisible() {
        if (plasmoid.configuration.cpuAllActivated) {
            return false;
        }
        return widestLabelWidth() <= columnWidth()
    }

    PlasmaCore.ToolTipArea {
        id: stdToolTip
        anchors.fill: parent
        active: true
        mainText: i18n("System load")
        subText: toolTipSubText()
        visible: !plasmoid.configuration.cpuAllActivated || dataSource.maxCpuIndex < 5

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                kRun.openUrl(apps.data["org.kde.ksysguard.desktop"].entryPath)
            }
        }
    }

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        active: true
        visible: !stdToolTip.visible

        mainItem: Item {
            height: childrenRect.height + units.gridUnit * 2
            width: childrenRect.width + units.gridUnit * 2
            ColumnLayout {
                anchors {
                    top: parent.top
                    left:parent.left
                    margins: units.gridUnit
                }
                PlasmaExtras.Heading {
                    id: tooltipMaintext
                    level: 3
                    text: stdToolTip.mainText
                }
                PlasmaComponents.Label {
                    id: tooltipSubtext
                    text: toolTipSubText()
                    opacity: 0.7
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                kRun.openUrl(apps.data["org.kde.ksysguard.desktop"].entryPath)
            }
        }
    }

    RowLayout {
        id: rowLayout

        anchors {
            fill: main
        }

        spacing: main.compactBarMonitorsInUse ? 0 : 5


        RowLayout {
            id: cpusRow
            property int minCpuWidth: memColumn.minWidth * 0.33
            property int minWidth : Math.max(memColumn.minWidth * 2, cpuRepeater.count * minCpuWidth)
            visible: plasmoid.configuration.cpuActivated && plasmoid.configuration.cpuAllActivated
            Layout.minimumWidth: minWidth
            Layout.preferredWidth: height * 2

            spacing: 0
            Repeater {
                id: cpuRepeater
                model: (dataSource.maxCpuIndex+1)

                delegate: ColumnLayout {
                    Layout.minimumWidth: cpusRow.minCpuWidth
                    property int cpuIndex: index

                    PlasmaExtras.Heading {
                        id: cpuLabel
                        level: main.headingLevel
                        text: i18n("CPU %1", cpuIndex)
                        visible: main.labelsVisible
                    }
                    ConditionallyLoadedMonitors {
                        colors: cpuColors
                        property string niceLoad: "cpu/cpu" + cpuIndex + "/nice"
                        property string userLoad: "cpu/cpu" + cpuIndex + "/user"
                        property string sysLoad:  "cpu/cpu" + cpuIndex + "/sys"
                        property string ioWait:   "cpu/cpu" + cpuIndex + "/wait"
                        property var cpuLoadProportions: dataSource.data[ioWait] ? [
                        fitCpuLoad(dataSource.data[sysLoad].value),
                        fitCpuLoad(dataSource.data[userLoad].value),
                        fitCpuLoad(dataSource.data[niceLoad].value),
                        fitCpuLoad(dataSource.data[ioWait].value)
                        ] : [.0, .0, .0, .0]
                        proportions: cpuLoadProportions
                    }
                }
            }
        }

        ColumnLayout {
            id: cpuColumn
            property double minWidth: memColumn.minWidth
            visible: plasmoid.configuration.cpuActivated && !plasmoid.configuration.cpuAllActivated
            Layout.minimumWidth: minWidth
            Layout.preferredWidth: height

            PlasmaExtras.Heading {
                id: cpuLabel
                level: main.headingLevel
                text: i18n("CPU")
                visible: main.labelsVisible
            }

            ConditionallyLoadedMonitors {
                colors: cpuColors
                proportions: dataSource.totalCpuLoadProportions
            }
        }

        ColumnLayout {
            id: memColumn
            property double minWidth: units.gridUnit * 0.7
            visible: plasmoid.configuration.memoryActivated
            Layout.minimumWidth: minWidth
            Layout.preferredWidth: height

            PlasmaExtras.Heading {
                id: memoryLabel
                level: main.headingLevel
                text: i18n("Memory")
                visible: main.labelsVisible
            }

            ConditionallyLoadedMonitors {
                colors: memoryColors
                proportions: dataSource.memoryUsageProportions
            }
        }

        ColumnLayout {
            id: swapColumn
            property double minWidth: memColumn.minWidth
            visible: plasmoid.configuration.swapActivated
            Layout.minimumWidth: minWidth
            Layout.preferredWidth: height

            PlasmaExtras.Heading {
                id: swapLabel
                level: main.headingLevel
                text: i18n("Swap")
                visible: main.labelsVisible
            }

            ConditionallyLoadedMonitors {
                colors: swapColors
                proportions: [dataSource.swapUsageProportion]
            }
        }
    } // rowLayout
} // main
