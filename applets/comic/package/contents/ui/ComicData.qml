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
import org.kde.qtextracomponents 0.1
 
Item {
    property string id
    property string first
    property string last
    property string current
    property string next
    property string prev
    property string stored
    property string currentReadable
    property string errorStrip
    property string author
    property string title
    property string stripTitle
    property string additionalText
    property alias image: image
    QImageItem {
        id: image
    }
    property url websiteUrl
    property string imageUrl
    property string shopUrl

    property int firstStripNum
    property int maxStripNum

    property bool isLeftToRight
    property bool isTopToBottom

    property string suffixType

    function setData(data){
        var hasError = data["Error"];

        if (!hasError) {
            console.log("no error = " + !hasError);

            comicData.image.image = data["Image"];
            comicData.prev = data["Previous identifier suffix"];
            comicData.next = data["Next identifier suffix"];
            comicData.additionalText = data["Additional text"];
        }

        comicData.author = data["Comic Author"];
        comicData.first = data["First strip identifier suffix"];

        comicData.title = data["Title"];
        comicData.stripTitle = data["Strip title"];
        comicData.suffixType = data["SuffixType"];
        comicData.current = data["Identifier"].replace(comicData.id + ":","");

        comicData.websiteUrl = data["Website Url"].toString;
        comicData.imageUrl = data["Image Url"].toString;
        comicData.shopUrl = data["Shop Url"].toString;

        comicData.isLeftToRight = data["isLeftToRight"];
        comicData.isTopToBottom = data["isTopToBottom"];

        console.log("identifier = " + data["Identifier"].replace(comicData.id + ":",""));
    }

    function hasNext() {
        return (comicData.next != undefined && comicData.next != "");
    }

    function hasPrev() {
        return (comicData.prev != undefined && comicData.prev != "");
    }

    function hasFirst() {
        return (comicData.first != undefined && comicData.first != "");
    }
}