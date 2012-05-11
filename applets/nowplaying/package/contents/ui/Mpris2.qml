/*
 *   Copyright 2012 Alex Merry <alex.merry@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

PlasmaCore.DataSource {
    id: source
    engine: "mpris2"
    connectedSources: Array("@multiplex")
    interval: 0
    property bool hasMetadata: getHasMetadata()
    property string title: getMetadata("xesam:title", '')
    property string artist: getMetadata("xesam:artist", []).join(", ")
    property string album: getMetadata("xesam:album", '')
    property url artUrl: getMetadata("mpris:artUrl", '')
    property string playbackStatus: getHasData() ? data["@multiplex"]["PlaybackStatus"] : 'unknown'
    property bool canControl: getHasData() && data["@multiplex"]["CanControl"]
    property int trackLength: getMetadata("mpris:length", 0) / 1000

    function getHasData() {
        return data["@multiplex"] != undefined
            && data["@multiplex"]["PlaybackStatus"] != undefined;
    }

    function getHasMetadata() {
        return data["@multiplex"] != undefined
            && data["@multiplex"]["Metadata"] != undefined
            && data["@multiplex"]["Metadata"]["mpris:trackid"] != undefined;
    }

    function getMetadata(entry, def) {
        if (hasMetadata && data["@multiplex"]["Metadata"][entry] != undefined)
            return data["@multiplex"]["Metadata"][entry];
        else
            return def;
    }

    function currentPos() {
        if (!getHasData())
            return 0;
        var pos = data["@multiplex"]["Position"] / 1000;
        var pbStatus = data["@multiplex"]["PlaybackStatus"];
        if (pbStatus == "Playing") {
            var lastUpd = data["@multiplex"]["Position last updated (UTC)"];
            var diffMs = Date.now() - lastUpd.getTime();
            var rate = data["@multiplex"]["Rate"];
            return pos + (diffMs * rate);
        } else {
            return pos;
        }
    }
}

// vi:sts=4:sw=4:et
