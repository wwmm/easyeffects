import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ItemDelegate {
    id: listItemDelegate

    required property int index
    property real intensity
    property real minIntensity
    property real maxIntensity
    property bool mute
    property bool bypass
    property string bandFrequency

    signal intensityModified(real value)
    signal muteModified(bool value)
    signal bypassModified(bool value)

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
            text: bandFrequency
            enabled: false
        }

        Controls.Slider {
            id: intensitySlider

            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            orientation: Qt.Vertical
            from: minIntensity
            to: maxIntensity
            value: intensity
            stepSize: 1
            onMoved: {
                intensityModified(value);
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: Number(intensitySlider.value).toLocaleString(Qt.locale(), 'f', 0)
            enabled: false
        }

    }

}
