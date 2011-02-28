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
