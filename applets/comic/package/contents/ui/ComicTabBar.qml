/*
 * Copyright 2012  Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Row {
    id: root
    property bool showIcon: true
    property bool showText: true
    property variant tabModel

    anchors { left: parent.left; right: parent.right; top: parent.top}

    signal currentTabChanged(int newIndex)
    
    PlasmaComponents.TabBar{
        id: tabbar

        onCurrentTabChanged: {
            console.log("OnCurrentChanged triggered");
            root.currentTabChanged(currentButtonIndex());
            console.log("Current Index: " + currentButtonIndex());
        }

        anchors {
            left: parent.left
            right: (tabNavigation.visible) ? tabNavigation.left : parent.right
        }

        Repeater {
            model: tabModel
            delegate:  PlasmaComponents.TabButton {
                property string key: model.key

                id: tabButton
                text: showText ? model.title : ""
                iconSource: showIcon ? model.icon : ""
            }
        }
    }

    Rectangle {
        id: tabNavigation
        color: "transparent"
        anchors.right: parent.right
        visible: (tabModel.count > 1)
        width: prevButton.width*2 +4
        height: prevButton.height+4

        Row {
            anchors.centerIn: parent

            PlasmaComponents.ToolButton {
                id: prevButton
                iconSource: "arrow-left"
                onClicked : {
                    changeButtonIndex(false);
                }
                enabled: !(currentButtonIndex() == 0)
            }

            PlasmaComponents.ToolButton {
                id: nextButton
                iconSource: "arrow-right"
                onClicked : {
                    changeButtonIndex(true);
                }
                enabled: !(currentButtonIndex() == tabModel.count - 1)
            }
        }
    }

    function setCurrentButtonIndex(index) {
        if (index < 0 || index > tabbar.layout.children.length-1)
            return;

        tabbar.currentTab = tabbar.layout.children[index];
    }
    
    function changeButtonIndex(forward){
        var oldIndex = currentButtonIndex();
        if (forward) {
            if (oldIndex != tabbar.layout.children.length - 1) {
                oldIndex++
            }
        } else {
            if (oldIndex != 0) {
                oldIndex--
            }
        }

        tabbar.currentTab = tabbar.layout.children[oldIndex]
    }

    function currentButtonIndex() {
        for (var i = 0; i < tabbar.layout.children.length; ++i) {
            if (tabbar.layout.children[i] == tabbar.currentTab)
                return i
        }
        return -1
    }

    function updateButtonDisplayMode(mode) {
        console.log("updateButtonDisplayMode");
        for (var i = 0; i < tabbar.layout.children.length; ++i) {
            if (mode == 0) {
                tabbar.layout.children[i].iconSource = undefined;
                tabbar.layout.children[i].title = tabModel.get(i).title;
            } else if (mode == 1) {
                tabbar.layout.children[i].title = "";
                tabbar.layout.children[i].iconSource = tabModel.get(i).icon;
            }
        }
    }
}