import "Common.js" as Common
import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    property real value: 0.5
    property real from: 0
    property real to: 1
    readonly property real clampedValue: Common.clamp(value, from, to)

    implicitWidth: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    implicitHeight: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    radius: width / 2
    color: Kirigami.Theme.neutralBackgroundColor
    border.color: Kirigami.Theme.activeBackgroundColor
    border.width: 2
    Kirigami.Theme.colorSet: Kirigami.Theme.Button
    Kirigami.Theme.inherit: false

    Item {
        height: parent.height * (clampedValue - from) / (to - from)
        clip: true

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        Rectangle {
            width: root.width - root.border.width * 2
            height: root.height - root.border.width * 2
            radius: width / 2
            color: Kirigami.Theme.alternateBackgroundColor

            anchors {
                bottom: parent.bottom
                left: parent.left
                margins: root.border.width
            }

        }

    }

    Text {
        id: valueLabel

        anchors.centerIn: parent
        text: Number(root.value).toLocaleString(Qt.locale())
        color: Kirigami.Theme.textColor
    }

}
