import "Common.js" as Common
import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    property real value: 0.5
    property real from: 0
    property real to: 1
    property int decimals: 2
    property bool convertDecibelToLinear: false
    readonly property real dbFrom: dbToLinear(from)
    readonly property real dbTo: dbToLinear(to)
    readonly property real clampedValue: Common.clamp(value, from, to)

    function dbToLinear(dbValue) {
        return Math.exp(dbValue / 20) * Math.LN10;
    }

    implicitWidth: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    implicitHeight: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    radius: Kirigami.Units.largeSpacing
    color: Kirigami.Theme.neutralBackgroundColor
    border.color: Kirigami.Theme.activeBackgroundColor
    border.width: 2
    Kirigami.Theme.colorSet: Kirigami.Theme.Button
    Kirigami.Theme.inherit: false

    Item {
        height: {
            if (convertDecibelToLinear)
                parent.height * (dbToLinear(clampedValue) - dbFrom) / (dbTo - dbFrom);
            else
                parent.height * (clampedValue - from) / (to - from);
        }
        clip: true

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        Rectangle {
            width: root.width - root.border.width * 2
            height: root.height - root.border.width * 2
            radius: Kirigami.Units.mediumSpacing
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
        text: Number(root.value).toLocaleString(Qt.locale(), 'f', decimals)
        color: Kirigami.Theme.textColor
    }

}
