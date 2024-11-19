import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ItemDelegate {
    id: listItemDelegate

    required property int index
    required property var pluginDB
    property bool mute
    property bool bypass

    down: false
    hoverEnabled: false
    height: ListView.view.height

    contentItem: ColumnLayout {
        Controls.Button {
            Layout.alignment: Qt.AlignCenter
            icon.name: "emblem-system-symbolic"
            onClicked: menuDialog.open()
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: {
                switch (index) {
                case 0:
                    return Number(250).toLocaleString(Qt.locale(), 'f', 0) + " Hz";
                case 1:
                    return Number(750).toLocaleString(Qt.locale(), 'f', 0) + " Hz";
                case 2:
                    return Number(1.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 3:
                    return Number(2.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 4:
                    return Number(3.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 5:
                    return Number(4.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 6:
                    return Number(5.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 7:
                    return Number(6.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 8:
                    return Number(7.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 9:
                    return Number(8.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 10:
                    return Number(9.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 11:
                    return Number(12.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                case 12:
                    return Number(17.5).toLocaleString(Qt.locale(), 'f', 1) + " kHz";
                default:
                    return "Hz";
                }
            }
            enabled: false
        }

        Controls.Slider {
            id: intensitySlider

            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            orientation: Qt.Vertical
            from: pluginDB.getMinValue("intensityBand" + index)
            to: pluginDB.getMaxValue("intensityBand" + index)
            value: pluginDB["intensityBand" + index]
            stepSize: 1
            onMoved: {
                if (value != pluginDB["intensityBand" + index])
                    pluginDB["intensityBand" + index] = value;

            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: Number(intensitySlider.value).toLocaleString(Qt.locale(), 'f', 0)
            enabled: false
        }

    }

}
