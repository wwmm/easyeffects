import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.ItemDelegate {
    id: listItemDelegate

    required property int index
    required property var bandDB

    function toLocaleLabel(num, decimal, unit) {
        return Number(num).toLocaleString(Qt.locale(), 'f', decimal) + ` ${unit}`;
    }

    down: false
    hoverEnabled: false
    height: ListView.view.height

    Controls.Popup {
        id: menu

        parent: menuButton
        focus: true
        x: Math.round((parent.width - width) / 2)
        y: parent.height
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
        onClosed: {
            menuButton.checked = false;
        }

        contentItem: ColumnLayout {
            Controls.Button {
                readonly property string bandName: "band" + index + "Mute"

                Layout.alignment: Qt.AlignCenter
                text: i18n("Mute")
                checkable: true
                checked: bandDB[bandName]
                onCheckedChanged: {
                    if (checked != bandDB[bandName])
                        bandDB[bandName] = checked;
                }
            }

            Controls.Button {
                readonly property string bandName: "band" + index + "Solo"

                Layout.alignment: Qt.AlignCenter
                text: i18n("Solo")
                checkable: true
                checked: bandDB[bandName]
                onCheckedChanged: {
                    if (checked != bandDB[bandName])
                        bandDB[bandName] = checked;
                }
            }
        }
    }

    contentItem: ColumnLayout {
        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: index + 1
            enabled: false
        }

        Controls.Button {
            id: menuButton

            Layout.alignment: Qt.AlignCenter
            icon.name: "emblem-system-symbolic"
            checkable: true
            checked: false
            onCheckedChanged: {
                if (checked)
                    menu.open();
                else
                    menu.close();
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: {
                const f = bandDB["band" + index + "Frequency"];
                if (f < 1000) {
                    return toLocaleLabel(f, 0, "Hz");
                } else {
                    toLocaleLabel(f * 0.001, 1, "kHz");
                }
            }
            enabled: false
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Q") + " " + toLocaleLabel(bandDB["band" + index + "Q"], 2, "")
            enabled: false
        }

        Controls.Slider {
            id: gainSlider

            readonly property string bandName: "band" + index + "Gain"

            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            orientation: Qt.Vertical
            from: bandDB.getMinValue(bandName)
            to: bandDB.getMaxValue(bandName)
            value: bandDB[bandName]
            stepSize: 1
            enabled: true
            onMoved: {
                if (value != bandDB[bandName])
                    bandDB[bandName] = value;
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: Number(gainSlider.value).toLocaleString(Qt.locale(), 'f', 0)
            enabled: false
        }
    }
}
