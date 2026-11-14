import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root
    property alias cfg_useMinViewWidth: useMinViewWidth.checked
    property alias cfg_minViewWidth: minViewWidth.value
    property alias cfg_constantZoomFactor: constantZoomFactor.value
    property alias cfg_useDefaultUrl: useDefaultUrlRadio.checked
    property alias cfg_defaultUrl: defaultUrl.text
    property alias cfg_privateBrowsing: privateBrowsing.checked

    Kirigami.Form {
        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: defaultUrlGroup }

            Kirigami.FormEntry {
                title: i18nc("@title:group", "On startup, load:")
                contentItem: QQC2.RadioButton {
                    id: useUrlRadio
                    text: i18nc("@option:radio", "Last-visited page")
                    checked: !root.cfg_useDefaultUrl
                    QQC2.ButtonGroup.group: defaultUrlGroup

                    onToggled: root.cfg_useDefaultUrl = false;
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                        id: useDefaultUrlRadio
                        QQC2.ButtonGroup.group: defaultUrlGroup

                        onToggled: {
                            defaultUrl.forceActiveFocus();
                            defaultUrl.selectAll();
                        }
                    }

                trailingItems: [
                    QQC2.TextField {
                        id: defaultUrl
                        Layout.fillWidth: true
                        onEditingFinished: {
                            let url = text.trim();
                            if (url && url.indexOf(":/") < 0) {
                                url = "http://" + url;
                            }
                            text = url
                        }
                        placeholderText: Accessible.description
                        enabled: useDefaultUrlRadio.checked
                        Accessible.description: text.length > 0 ? text : i18nc("@info", "Type URL or leave empty for blank page")
                    }
                ]
            }
        }

        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: zoomGroup }

            Kirigami.FormEntry {
                title: i18nc("@title:group", "Content scaling:")
                contentItem: QQC2.RadioButton {
                    id: useConstantZoom
                    text: i18nc("@option:radio", "Fixed scale")
                    checked: !root.cfg_useMinViewWidth

                    QQC2.ButtonGroup.group: zoomGroup

                    onClicked: {
                        constantZoomFactor.forceActiveFocus();
                    }
                }
                trailingItems: [
                    QQC2.SpinBox {
                        id: constantZoomFactor
                        Layout.preferredWidth: Math.max(implicitWidth, minViewWidth.implicitWidth)
                        editable: true
                        enabled: useConstantZoom.checked

                        validator: RegularExpressionValidator {
                            regularExpression: /[0-9]?[0-9]{2}[ ]?%/
                        }

                        textFromValue: function(value) {
                            return value+"%";
                        }

                        valueFromText: function(text) {
                            return text.split(" ")[0].split("%")[0];
                        }

                        from: 25
                        to: 500
                    }
                ]
            }

            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                        id: useMinViewWidth
                        text: i18nc("@option:radio", "Automatic scaling if width is below")

                        QQC2.ButtonGroup.group: zoomGroup

                        onClicked: {
                            minViewWidth.forceActiveFocus();
                        }
                    }
                trailingItems: [
                    QQC2.SpinBox {
                        id: minViewWidth
                        Layout.preferredWidth: Math.max(implicitWidth, constantZoomFactor.implicitWidth)
                        editable: true
                        enabled: useMinViewWidth.checked

                        validator: RegularExpressionValidator {
                            regularExpression: /[0-9]?[0-9]{3}[ ]?px/
                        }

                        textFromValue: function(value) {
                            return value+"px";
                        }

                        valueFromText: function(text) {
                            return text.split(" ")[0].split("px")[0];
                        }

                        from: 320
                        to: 3840
                    }
                ]
            }
        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:group", "Browser Settings:")
                contentItem: QQC2.CheckBox {
                    id: privateBrowsing
                    text: i18nc("@option:radio", "Private Browsing")
                }
            }
        }
    }
}
