/*
    Copyright (C) 2011  Luiz Romário Santana Rios <luizromario@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
	id: mainWidget
	PlasmaCore.DBusInterface {
		id: iface
		service: "org.kde.plasma-desktop"
		path: "/App"
		interfaceName: "local.PlasmaApp"
	}

	PlasmaWidgets.IconWidget {
		id: icon
		icon: QIcon("preferences-activities")

		onClicked: iface.call("toggleActivityManager")
	}
}
