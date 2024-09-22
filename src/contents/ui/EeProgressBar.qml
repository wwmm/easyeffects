import "Common.js" as Common
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AbstractFormDelegate {
    id: control

    property real value: 0
    property real from: 0
    property real to: 1
    property string label: ""
    property string unit: ""
    property int elide: Text.ElideRight
    property int wrapMode: Text.Wrap
    readonly property real clampedValue: Common.clamp(value, from, to)

    Kirigami.Theme.colorSet: Kirigami.Theme.Button
    Kirigami.Theme.inherit: false

    background: Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.neutralBackgroundColor
        radius: 3
    }

    contentItem: Item {
        anchors.fill: parent

        Rectangle {
            width: clampedValue / (to - from) * parent.width
            height: parent.height
            color: Kirigami.Theme.alternateBackgroundColor
            radius: 5
        }

        RowLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                text: control.label
                elide: control.elide
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: 1
            }

            Label {
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                text: Number(clampedValue).toLocaleString(Qt.locale()) + " " + control.unit
                elide: control.elide
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: 1
            }

        }

    }

}
