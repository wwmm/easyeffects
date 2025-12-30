import QtQuick
import "Common.js" as Common
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    property real from: 0
    property real to: 1
    property int decimals: 2
    property bool convertDecibelToLinear: false
    property bool topToBottom: false
    readonly property real liFrom: Common.dbToLinear(from)
    readonly property real liTo: Common.dbToLinear(to)
    readonly property real decimalFactor: Math.pow(10, -decimals)
    property real value: 0
    property real clampedValue: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    implicitWidth: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    implicitHeight: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    radius: Kirigami.Units.cornerRadius
    border.width: 1
    clip: true
    color: Kirigami.Theme.neutralBackgroundColor
    border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)

    function setValue(value) {
        // Only update if meaningfully different

        if (Math.abs(value - root.value) >= root.decimalFactor) {
            root.value = value;
            root.clampedValue = Common.clamp(value, root.from, root.to);
        } else {
            return;
        }

        let newDisplayValue;

        if (root.topToBottom === false) {
            newDisplayValue = value > sampleTimer.value ? value : sampleTimer.value;
        } else {
            newDisplayValue = value < sampleTimer.value ? value : sampleTimer.value;
        }

        // label

        valueLabel.text = Number(newDisplayValue).toLocaleString(Qt.locale(), 'f', root.decimals);

        // level rect

        if (root.convertDecibelToLinear) {
            levelScale.yScale = root.topToBottom === false ? (Common.dbToLinear(root.clampedValue) - root.liFrom) / (root.liTo - root.liFrom) : (Common.dbToLinear(root.clampedValue) - root.liTo) / (root.liFrom - root.liTo);
        } else {
            levelScale.yScale = root.topToBottom === false ? (root.clampedValue - root.from) / (root.to - root.from) : (root.clampedValue - root.to) / (root.from - root.to);
        }

        //hist rect

        const clampedNewDisplayValue = Common.clamp(newDisplayValue, root.from, root.to);

        const liFrac = (Common.dbToLinear(clampedNewDisplayValue) - root.liFrom) / (root.liTo - root.liFrom);
        const frac = (clampedNewDisplayValue - root.from) / (root.to - root.from);

        if (root.convertDecibelToLinear) {
            histScale.y = root.height * (1.0 - liFrac);
        } else {
            histScale.y = root.height * (1.0 - frac);
        }
    }

    Rectangle {
        id: levelRect

        anchors.fill: parent
        radius: Kirigami.Units.cornerRadius
        border.width: 1
        color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.3)
        border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)

        transform: Scale {
            id: levelScale

            origin.y: root.topToBottom === false ? root.height : 0

            Behavior on yScale {
                enabled: DbMain.enableLevelMetersAnimation

                NumberAnimation {
                    duration: DbMain.levelMetersAnimationDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
    }

    Rectangle {
        id: histRect

        height: Kirigami.Units.smallSpacing * 0.5
        radius: Kirigami.Units.cornerRadius
        color: levelScale.yScale < 0.85 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.negativeTextColor

        transform: Translate {
            id: histScale

            Behavior on y {
                enabled: DbMain.enableLevelMetersAnimation

                NumberAnimation {
                    duration: DbMain.levelMetersAnimationDuration
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
        text: Number(0).toLocaleString(Qt.locale(), 'f', root.decimals)
        color: Kirigami.Theme.textColor
    }

    Timer {
        id: sampleTimer

        property real value: root.value

        interval: DbMain.levelMetersLabelTimer
        repeat: true
        running: root.visible

        onTriggered: {
            value = root.value;
        }
    }
}
