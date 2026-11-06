/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick
import org.kde.plasma.plasmoid

ListModel {
    id: historyModel

    readonly property int maxColorCount: 9
    readonly property bool hasAnyColors: historyModel.count > 0

    function save() {
        let history = [];
        for (let i = 0; i < count; i++) {
            history.push(get(i).color);
        }
        Plasmoid.configuration.history = history;
    }

    function addColorToHistory(color) {
        // this .toString() is needed otherwise Qt completely screws it up
        // replacing *all* items in the list by the new items and other nonsense
        historyModel.insert(0, { color: color.toString() });
        // limit to 9 entries
        if (historyModel.count > maxColorCount) {
            historyModel.remove(maxColorCount);
        }
        historyModel.save();
    }
}
