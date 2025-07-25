import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.ItemDelegate {
    id: delegate

    required property int index
    required property var pluginDB

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
                Layout.alignment: Qt.AlignCenter
                text: i18n("Mute")
                checkable: true
                checked: delegate.pluginDB["muteBand" + delegate.index]
                onCheckedChanged: {
                    if (checked != delegate.pluginDB["muteBand" + delegate.index])
                        delegate.pluginDB["muteBand" + delegate.index] = checked;
                }
            }

            Controls.Button {
                Layout.alignment: Qt.AlignCenter
                text: i18n("Bypass")
                checkable: true
                checked: delegate.pluginDB["bypassBand" + delegate.index]
                onCheckedChanged: {
                    if (checked != delegate.pluginDB["bypassBand" + delegate.index])
                        delegate.pluginDB["bypassBand" + delegate.index] = checked;
                }
            }
        }
    }

    contentItem: ColumnLayout {
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
                switch (delegate.index) {
                case 0:
                    return toLocaleLabel(250, 0, "Hz");
                case 1:
                    return toLocaleLabel(750, 0, "Hz");
                case 2:
                    return toLocaleLabel(1.5, 1, "kHz");
                case 3:
                    return toLocaleLabel(2.5, 1, "kHz");
                case 4:
                    return toLocaleLabel(3.5, 1, "kHz");
                case 5:
                    return toLocaleLabel(4.5, 1, "kHz");
                case 6:
                    return toLocaleLabel(5.5, 1, "kHz");
                case 7:
                    return toLocaleLabel(6.5, 1, "kHz");
                case 8:
                    return toLocaleLabel(7.5, 1, "kHz");
                case 9:
                    return toLocaleLabel(8.5, 1, "kHz");
                case 10:
                    return toLocaleLabel(9.5, 1, "kHz");
                case 11:
                    return toLocaleLabel(12.5, 1, "kHz");
                case 12:
                    return toLocaleLabel(17.5, 1, "kHz");
                default:
                    return "Hz";
                }
            }
            enabled: false
        }

        Controls.Slider {
            id: intensitySlider

            property real pageSteps: 10

            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            orientation: Qt.Vertical
            from: delegate.pluginDB.getMinValue("intensityBand" + delegate.index)
            to: delegate.pluginDB.getMaxValue("intensityBand" + delegate.index)
            value: delegate.pluginDB["intensityBand" + delegate.index]
            stepSize: 1
            enabled: !delegate.pluginDB["muteBand" + delegate.index] && !delegate.pluginDB["bypassBand" + delegate.index]
            onMoved: {
                if (value != delegate.pluginDB["intensityBand" + delegate.index])
                    delegate.pluginDB["intensityBand" + delegate.index] = value;
            }
            Keys.onPressed: event => {
                if (event.key === Qt.Key_PageUp) {
                    const v = value + pageSteps * stepSize;

                    delegate.pluginDB["intensityBand" + delegate.index] = Common.clamp(v, from, to);

                    event.accepted = true;
                } else if (event.key === Qt.Key_PageDown) {
                    const v = value - pageSteps * stepSize;

                    delegate.pluginDB["intensityBand" + delegate.index] = Common.clamp(v, from, to);

                    event.accepted = true;
                }
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: Number(intensitySlider.value).toLocaleString(Qt.locale(), 'f', 0)
            enabled: false
        }
    }
}
