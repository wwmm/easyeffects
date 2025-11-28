import QtQuick
import "Common.js" as Common
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    property real value: 0.5
    property real from: 0
    property real to: 1
    property int decimals: 2
    property bool convertDecibelToLinear: false
    property bool topToBottom: false
    readonly property real dbFrom: Common.dbToLinear(from)
    readonly property real dbTo: Common.dbToLinear(to)
    readonly property real clampedValue: Common.clamp(value, from, to)
    readonly property real displayValue: root.topToBottom === false ? (root.clampedValue > sampleTimer.value ? root.clampedValue : sampleTimer.value) : (root.clampedValue < sampleTimer.value ? root.clampedValue : sampleTimer.value)

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    implicitWidth: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    implicitHeight: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    radius: Kirigami.Units.cornerRadius
    border.width: 1
    clip: true
    color: Kirigami.Theme.neutralBackgroundColor
    border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)

    Rectangle {
        id: levelRect

        width: parent.width
        height: parent.height
        radius: Kirigami.Units.cornerRadius
        border.width: 1
        color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.3)
        border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)

        transform: Scale {
            id: levelScale

            yScale: {
                if (root.convertDecibelToLinear)
                    root.topToBottom === false ? (Common.dbToLinear(root.clampedValue) - root.dbFrom) / (root.dbTo - root.dbFrom) : (Common.dbToLinear(root.clampedValue) - root.dbTo) / (root.dbFrom - root.dbTo);
                else
                    root.topToBottom === false ? (root.clampedValue - root.from) / (root.to - root.from) : (root.clampedValue - root.to) / (root.from - root.to);
            }

            origin.y: root.topToBottom === false ? levelRect.height : 0
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
        radius: Kirigami.Units.cornerRadius
        color: levelScale.yScale < 0.85 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.negativeTextColor

        transform: Translate {
            readonly property real dbFrac: (Common.dbToLinear(root.displayValue) - root.dbFrom) / (root.dbTo - root.dbFrom)
            readonly property real frac: (root.displayValue - root.from) / (root.to - root.from)

            y: {
                if (root.convertDecibelToLinear)
                    return root.height * (1.0 - dbFrac);
                else
                    return root.height * (1.0 - frac);
            }

            Behavior on y {
                NumberAnimation {
                    duration: 400
                    easing.type: Easing.OutCubic
                }
            }
        }

        anchors {
            left: parent.left
            right: parent.right
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

        interval: DbMain.levelMetersLabelTimer
        repeat: true
        running: root.visible

        onTriggered: {
            value = root.clampedValue;
        }
    }
}
