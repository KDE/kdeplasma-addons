/*
 *  Copyright (C) 2011  Ivan Cukic <ivan.cukic at kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
// import org.kde.plasma.shelf 0.1 as Shelf
import org.kde.qtextracomponents 0.1
import org.kde.lancelot.components.data 0.1 as LancelotData

Item {
    id: root
    property int minimumWidth: 290
    property int minimumHeight: 340

    // PlasmaCore.DataSource {
    //     id: recommendationsModel
    //     engine: "org.kde.recommendations"
    //     interval: 0

    //     onSourceAdded: {
    //         connectSource(source)
    //     }

    //     Component.onCompleted: {
    //         connectedSources = sources
    //     }
    // }

    PlasmaComponents.TextField {
        id: textSearch

        anchors {
            top:   parent.top
            left:  parent.left
            right: parent.right
        }
    }

    ListView {
        id: list
        clip: true

        // model: PlasmaCore.DataModel {
        //     dataSource: recommendationsModel
        // }

        // model: LancelotData.FavoriteApplications { }

        // model: LancelotData.FolderModel {
        //     folder: "/home/ivan"
        // }

        model: LancelotData.DirModel {
            dir: "/home/ivan"
        }

        delegate: ExtenderButton {
            title:       model.display
            description: model.description
            icon:        model.decoration
        }

        anchors {
            top:    textSearch.bottom
            left:   parent.left
            bottom: parent.bottom
            right:  parent.right
        }
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: list

        anchors {
            right:  list.right
            top:    list.top
            bottom: list.bottom
        }
    }

}
