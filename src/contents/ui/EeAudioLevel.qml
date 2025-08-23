import "Common.js" as Common
import QtQuick
import org.kde.kirigami as Kirigami
import ee.database as DB

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
    readonly property real displayValue: root.clampedValue > sampleTimer.value ? root.clampedValue : sampleTimer.value

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
    clip: true

    Rectangle {
        id: levelRect

        width: parent.width
        height: parent.height
        radius: Kirigami.Units.largeSpacing
        color: Kirigami.Theme.alternateBackgroundColor
        border.width: 0

        transform: Scale {
            yScale: {
                if (root.convertDecibelToLinear)
                    (root.dbToLinear(root.clampedValue) - root.dbFrom) / (root.dbTo - root.dbFrom);
                else
                    (root.clampedValue - root.from) / (root.to - root.from);
            }

            origin.y: levelRect.height
        }

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }

    Rectangle {
        id: histRect

        height: Kirigami.Units.smallSpacing * 0.5
        width: parent.height
        color: Kirigami.Theme.positiveTextColor

        transform: Translate {
            y: {
                if (root.convertDecibelToLinear)
                    root.height * (1.0 - (root.dbToLinear(root.displayValue) - root.dbFrom) / (root.dbTo - root.dbFrom));
                else
                    root.height * (1.0 - (root.displayValue - root.from) / (root.to - root.from));
            }
        }
    }

    Text {
        id: valueLabel

        anchors.centerIn: parent
        text: Number(root.displayValue).toLocaleString(Qt.locale(), 'f', root.decimals)
        color: Kirigami.Theme.textColor
    }

    Timer {
        id: sampleTimer

        property real value: root.clampedValue

        interval: DB.Manager.main.levelMetersLabelTimer
        repeat: true
        running: root.visible

        onTriggered: {
            value = root.clampedValue;
        }
    }
}
