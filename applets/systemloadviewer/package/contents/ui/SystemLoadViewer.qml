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

    readonly property bool setColorsManually: plasmoid.configuration.setColorsManually

    readonly property var cpuColors: setColorsManually ? [plasmoid.configuration.cpuSysColor,
                                                          plasmoid.configuration.cpuUserColor,
                                                          plasmoid.configuration.cpuNiceColor,
                                                          plasmoid.configuration.cpuIOWaitColor]
                                                       : [theme.buttonFocusColor,
                                                          theme.highlightColor,
                                                          theme.visitedLinkColor,
                                                          theme.linkColor]

    readonly property var memoryColors: setColorsManually ? [plasmoid.configuration.memApplicationColor,
                                                             plasmoid.configuration.memBuffersColor]
                                                          : [theme.buttonFocusColor,
                                                             theme.visitedLinkColor]
    readonly property var swapColors: setColorsManually ? [plasmoid.configuration.swapUsedColor]
                                                        : [theme.highlightColor]
    readonly property var cacheColors: setColorsManually ? [plasmoid.configuration.cacheDirtyColor,
                                                            plasmoid.configuration.cacheWritebackColor]
                                                         : [theme.buttonFocusColor,
                                                            theme.visitedLinkColor]

    // Make labels visible first time so that the
    // user knows which monitor is which.
    implicitWidth: widestLabelWidth()*1.3 * columnCount()
    implicitHeight: implicitWidth / 2

    readonly property bool circularMonitorsInUse: plasmoid.configuration.monitorType == 1
    readonly property bool compactBarMonitorsInUse: plasmoid.configuration.monitorType == 2
    readonly property bool allCPUsShown: plasmoid.configuration.cpuAllActivated

    readonly property double barsWidth: compactBarMonitorsInUse ? main.height * 0.35 * columnCount()
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
        if (plasmoid.configuration.cacheActivated) {
            wantedWidth += cacheColumn.minWidth
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
        if (plasmoid.configuration.cacheActivated) {
            wantedWidth += cacheColumn.minWidth
        }
        return wantedWidth
    }

    property bool labelsVisible

    Kio.KRun { id: kRun }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "systemmonitor"

        readonly property string cpuSystem: "cpu/system/"
        readonly property string niceLoad: cpuSystem + "nice"
        readonly property string userLoad: cpuSystem + "user"
        readonly property string sysLoad: cpuSystem + "sys"
        readonly property string ioWait: cpuSystem + "wait"
        readonly property string averageClock: cpuSystem + "AverageClock"
        readonly property string totalLoad: cpuSystem + "TotalLoad"
        readonly property string memPhysical: "mem/physical/"
        readonly property string memFree: memPhysical + "free"
        readonly property string memApplication: memPhysical + "application"
        readonly property string memBuffers: memPhysical + "buf"
        readonly property string memUsed: memPhysical + "used"
        readonly property string swap: "mem/swap/"
        readonly property string swapUsed: swap + "used"
        readonly property string swapFree: swap + "free"
        readonly property string cache: "mem/cache/"
        readonly property string cacheDirty: cache + "dirty"
        readonly property string cacheWriteback: cache + "writeback"
        readonly property string cores: "system/cores"

        property var totalCpuLoadProportions: [.0, .0, .0, .0]
        property int maxCpuIndex: 0
        property var memoryUsageProportions: [.0, .0]
        property double swapUsageProportion: .0
        property var cacheUsageProportions: [.0, .0]

        property double maxCache: 0.0

        connectedSources: [ cores ]

        function sources() {
            var array = [niceLoad, userLoad, sysLoad,
                         ioWait, memFree, memApplication, memBuffers,
                         memUsed, swapUsed, swapFree,
                         averageClock, totalLoad, cores, cacheDirty,
                         cacheWriteback]

            if (plasmoid.configuration.cpuAllActivated) {
                for (var i = 0; i <= maxCpuIndex; i++) {
                    array.push("cpu/cpu" + i + "/TotalLoad");
                    array.push("cpu/cpu" + i + "/clock");
                    array.push("cpu/cpu" + i + "/nice");
                    array.push("cpu/cpu" + i + "/user");
                    array.push("cpu/cpu" + i + "/sys");
                    array.push("cpu/cpu" + i + "/wait");
                }
            }
            return array;
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
                memoryUsageProportionsChanged()
            }
            else if (sourceName == swapUsed) {
                swapUsageProportion = fitSwapUsage(data.value)
                swapUsageProportionChanged()
            }
            else if (sourceName == cacheWriteback) {
                cacheUsageProportions = fitCacheUsage()
                cacheUsageProportionsChanged()
            }
            else if (sourceName == cores) {
                maxCpuIndex = data.value - 1
                connectedSources = sources()
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

        var cacheDirty = parseFloat(dataSource.data[dataSource.cacheDirty].value) / 1024
        var cacheWriteback = parseFloat(dataSource.data[dataSource.cacheWriteback].value) / 1024
        var cachePart = i18n("Cache Dirty, Writeback: %1 MiB, %2 MiB", Math.round(cacheDirty), Math.round(cacheWriteback))

        if (cpuClockPart === "") {
            return [cpuLoadPart, memoryPart, swapPart, cachePart].join("\n")
        }

        return [cpuLoadPart, cpuClockPart, memoryPart, swapPart, cachePart].join("\n")
    }

    function fitCpuLoad(load) {
        var x = load / maxCpuLoad;
        if (isNaN(x)) {return 0;}
        return Math.min(x, 1); // Ensure that we do not get values that might cause problems
    }

    function fitMemoryUsage(usage) {
        if (!dataSource.data[dataSource.memFree] || !dataSource.data[dataSource.memUsed])
            return 0;

        var x = (usage / (parseFloat(dataSource.data[dataSource.memFree].value) +
                         parseFloat(dataSource.data[dataSource.memUsed].value)))
        if (isNaN(x)) {return 0;}
        return Math.min(x, 1);
    }

    function fitSwapUsage(usage) {
        if (!dataSource.data[dataSource.swapFree])
            return 0;

        var x = (usage / (parseFloat(usage) + parseFloat(dataSource.data[dataSource.swapFree].value)))

        if (isNaN(x)) {return 0;}
        return Math.min(x, 1);
    }

    function fitCacheUsage() {
        var values = [ parseFloat(dataSource.data[dataSource.cacheDirty].value),
                       parseFloat(dataSource.data[dataSource.cacheWriteback].value) ];
        var total = values[0] + values[1];
        var props = [.0, .0];

        // hide the persistent small amount of dirty cache (10M seems a good threshold)
        if ( total < 10000 ) {
            dataSource.maxCache = 0;
            return props;
        }

        if (total > dataSource.maxCache) {
            dataSource.maxCache = total;
        }

        for (var i = 0; i < values.length; i++) {
           props[i] = values[i] / dataSource.maxCache;
           if (isNaN(props[i])) { props[i] = 0; }
           else { props[i] = Math.min(props[i], 0.99); }
        }

        return props;
    }

    function columnCount() {
        var columns = 0;
        var activeMonitors = [plasmoid.configuration.cpuActivated,
            plasmoid.configuration.memoryActivated,
            plasmoid.configuration.swapActivated,
            plasmoid.configuration.cacheActivated]

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
        if (plasmoid.configuration.cacheActivated) {
            widest = Math.max(widest, cacheLabel.paintedWidth)
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
                kRun.openService("org.kde.ksysguard")
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
                kRun.openService("org.kde.ksysguard")
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
            readonly property int minCpuWidth: memColumn.minWidth * 0.33
            readonly property int minWidth : Math.max(memColumn.minWidth * 2, cpuRepeater.count * minCpuWidth)
            visible: plasmoid.configuration.cpuActivated && plasmoid.configuration.cpuAllActivated
            Layout.minimumWidth: minWidth
            Layout.preferredWidth: height * 2

            spacing: 0
            Repeater {
                id: cpuRepeater
                model: (dataSource.maxCpuIndex+1)

                delegate: ColumnLayout {
                    Layout.minimumWidth: cpusRow.minCpuWidth
                    readonly property int cpuIndex: index

                    PlasmaExtras.Heading {
                        id: cpuLabel
                        level: main.headingLevel
                        text: i18n("CPU %1", cpuIndex)
                        visible: main.labelsVisible
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                    ConditionallyLoadedMonitors {
                        colors: cpuColors
                        readonly property string niceLoad: "cpu/cpu" + cpuIndex + "/nice"
                        readonly property string userLoad: "cpu/cpu" + cpuIndex + "/user"
                        readonly property string sysLoad:  "cpu/cpu" + cpuIndex + "/sys"
                        readonly property string ioWait:   "cpu/cpu" + cpuIndex + "/wait"
                        readonly property var cpuLoadProportions: dataSource.data[ioWait] ? [
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
            readonly property double minWidth: memColumn.minWidth
            visible: plasmoid.configuration.cpuActivated && !plasmoid.configuration.cpuAllActivated
            Layout.minimumWidth: minWidth
            Layout.preferredWidth: height

            PlasmaExtras.Heading {
                id: cpuLabel
                level: main.headingLevel
                text: i18n("CPU")
                visible: main.labelsVisible
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            ConditionallyLoadedMonitors {
                colors: cpuColors
                proportions: dataSource.totalCpuLoadProportions
            }
        }

        ColumnLayout {
            id: memColumn
            readonly property double minWidth: units.gridUnit * 0.7
            visible: plasmoid.configuration.memoryActivated
            Layout.minimumWidth: minWidth
            Layout.preferredWidth: height

            PlasmaExtras.Heading {
                id: memoryLabel
                level: main.headingLevel
                text: i18n("Memory")
                visible: main.labelsVisible
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            ConditionallyLoadedMonitors {
                colors: memoryColors
                proportions: dataSource.memoryUsageProportions
            }
        }

        ColumnLayout {
            id: swapColumn
            readonly property double minWidth: memColumn.minWidth
            visible: plasmoid.configuration.swapActivated
            Layout.minimumWidth: minWidth
            Layout.preferredWidth: height

            PlasmaExtras.Heading {
                id: swapLabel
                level: main.headingLevel
                text: i18n("Swap")
                visible: main.labelsVisible
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            ConditionallyLoadedMonitors {
                colors: swapColors
                proportions: [dataSource.swapUsageProportion]
            }
        }

        ColumnLayout {
            id: cacheColumn
            readonly property double minWidth: memColumn.minWidth
            visible: plasmoid.configuration.cacheActivated
            Layout.minimumWidth: minWidth
            Layout.preferredWidth: height

            PlasmaExtras.Heading {
                id: cacheLabel
                level: main.headingLevel
                text: i18n("Cache")
                visible: main.labelsVisible
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            ConditionallyLoadedMonitors {
                colors: cacheColors
                proportions: dataSource.cacheUsageProportions
            }
        }
    } // rowLayout
} // main
