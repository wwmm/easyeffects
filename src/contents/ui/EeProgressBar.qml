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
    property int decimals: 2
    property string label: ""
    property string unit: ""
    property int elide: Text.ElideRight
    property int wrapMode: Text.Wrap
    property bool rightToLeft: false
    readonly property real clampedValue: Common.clamp(value, from, to)

    Kirigami.Theme.colorSet: Kirigami.Theme.Button
    Kirigami.Theme.inherit: false

    background: Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.neutralBackgroundColor
        radius: 2
    }

    contentItem: Item {
        anchors.fill: parent

        Rectangle {
            width: rightToLeft === false ? (clampedValue - from) / (to - from) * parent.width : (clampedValue - to) / (from - to) * parent.width
            height: parent.height
            color: Kirigami.Theme.alternateBackgroundColor
            radius: 3
            anchors.right: rightToLeft ? parent.right : undefined
        }

        RowLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.smallSpacing
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                text: control.label
                elide: control.elide
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: 1
            }

            Label {
                Layout.rightMargin: Kirigami.Units.smallSpacing
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                text: Number(value).toLocaleString(Qt.locale(), 'f', decimals) + " " + control.unit
                elide: control.elide
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: 1
            }

        }

    }

}
