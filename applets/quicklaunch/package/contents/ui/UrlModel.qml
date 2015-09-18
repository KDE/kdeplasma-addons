/*
 *  Copyright 2015 David Rosca <nowrep@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.2

ListModel {
    id: listModel

    property int dropMarkerIndex : -1

    signal urlsChanged()

    function urls()
    {
        var out = [];
        for (var i = 0; i < listModel.count; ++i) {
            out.push(get(i).url);
        }
        return out;
    }

    function setUrls(urls)
    {
        clear();
        insertUrls(0, urls);

        urlsChanged();
    }

    function appendUrl(url)
    {
        append({ url: url });

        urlsChanged();
    }

    function insertUrl(index, url)
    {
        insert(index, { url: url });

        urlsChanged();
    }

    function insertUrls(index, urls)
    {
        for (var i = 0; i < urls.length; ++i) {
            insert(index + i, { url: urls[i] });
        }

        if (urls.length) {
            urlsChanged();
        }
    }

    function changeUrl(index, url)
    {
        // Force reloading delegate data
        set(index, { url: "quicklaunch:empty" });
        set(index, { url: url });

        urlsChanged();
    }

    function moveUrl(from, to)
    {
        if (from == -1 || to == -1 || from == to) {
            return false;
        }

        move(from, to, 1);

        urlsChanged();
        return true;
    }

    function removeUrl(index)
    {
        remove(index, 1);

        urlsChanged();
    }

    // Drop marker is internally represented as "quicklaunch:drop" url
    function showDropMarker(index)
    {
        if (index == -1) {
            index = dropMarkerIndex == -1 ? count : count - 1;
        }

        if (dropMarkerIndex != -1) {
            move(dropMarkerIndex, index, 1);
            dropMarkerIndex = index;
        } else {
            insert(index, { url: "quicklaunch:drop" });
            dropMarkerIndex = index;
        }
    }

    function clearDropMarker()
    {
        if (dropMarkerIndex != -1) {
            remove(dropMarkerIndex, 1);
            dropMarkerIndex = -1;
        }
    }
}
