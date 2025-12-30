import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Common.js" as Common
import org.kde.kirigami as Kirigami

Rectangle {
    id: control

    property real from: 0
    property real to: 1
    property int decimals: 2
    property string label: ""
    property string unit: ""
    property bool separateUnit: true
    property int elide: Text.ElideRight
    property int wrapMode: Text.Wrap
    property bool rightToLeft: false
    property bool convertDecibelToLinear: false
    property real value: 0
    property real clampedValue: 0
    readonly property real liFrom: Common.dbToLinear(from)
    readonly property real liTo: Common.dbToLinear(to)
    readonly property real decimalFactor: Math.pow(10, -decimals)

    readonly property string unitSuffix: if (!Common.isEmpty(control.unit)) {
        const split = control.separateUnit ? ' ' : '';
        return `${split}${control.unit}`;
    } else {
        return "";
    }

    Layout.fillWidth: true
    Layout.margins: Kirigami.Units.smallSpacing
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    implicitHeight: labelsItem.implicitHeight
    color: Kirigami.Theme.neutralBackgroundColor
    radius: 0
    clip: true

    function setValue(value) {

        // Only update if meaningfully different

        if (Math.abs(value - control.value) >= decimalFactor) {
            control.value = value;
            control.clampedValue = Common.clamp(value, control.from, control.to);
        } else {
            return;
        }

        let newDisplayValue;

        if (control.rightToLeft === false) {
            newDisplayValue = value > sampleTimer.value ? value : sampleTimer.value;
        } else {
            newDisplayValue = value < sampleTimer.value ? value : sampleTimer.value;
        }

        const normalizedClampedValue = (control.clampedValue - control.from) / (control.to - control.from);
        const normalizedClampedValueLinear = (Common.dbToLinear(control.clampedValue) - control.liFrom) / (control.liTo - control.liFrom);

        const rlNormalizedClampedValue = (control.clampedValue - control.to) / (control.from - control.to);
        const rlNormalizedClampedValueLinear = (Common.dbToLinear(control.clampedValue) - control.liTo) / (control.liFrom - control.liTo);

        //label

        valueLabel.text = Number(newDisplayValue).toLocaleString(Qt.locale(), 'f', control.decimals) + unitSuffix;

        // level rect

        if (control.convertDecibelToLinear) {
            levelScale.xScale = control.rightToLeft === false ? normalizedClampedValueLinear : rlNormalizedClampedValueLinear;
        } else {
            levelScale.xScale = control.rightToLeft === false ? normalizedClampedValue : rlNormalizedClampedValue;
        }

        const clampedNewDisplayValue = Common.clamp(newDisplayValue, control.from, control.to);

        const normalizedDisplayValue = (clampedNewDisplayValue - control.from) / (control.to - control.from);
        const normalizedDisplayValueLinear = (Common.dbToLinear(clampedNewDisplayValue) - control.liFrom) / (control.liTo - control.liFrom);

        const rlNormalizedDisplayValue = (newDisplayValue - control.to) / (control.from - control.to);
        const rlNormalizedDisplayValueLinear = (Common.dbToLinear(clampedNewDisplayValue) - control.liTo) / (control.liFrom - control.liTo);

        // hist rect

        let newHistPosition;

        if (control.convertDecibelToLinear) {
            newHistPosition = control.rightToLeft === false ? normalizedDisplayValueLinear * control.width : control.width - rlNormalizedDisplayValueLinear * control.width;
        } else {
            newHistPosition = control.rightToLeft === false ? normalizedDisplayValue * control.width : control.width - rlNormalizedDisplayValue * control.width;
        }

        if (newHistPosition >= control.width) {
            newHistPosition -= histRect.width;
        }

        if (histScale.x !== newHistPosition) {
            histScale.x = newHistPosition;
        }
    }

    Rectangle {
        id: levelRect

        anchors.fill: parent
        color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.3)

        transform: Scale {
            id: levelScale

            origin.x: control.rightToLeft === false ? 0 : control.width

            Behavior on xScale {
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

        width: Kirigami.Units.smallSpacing * 0.5
        radius: Kirigami.Units.cornerRadius
        color: levelScale.xScale < 0.85 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.negativeTextColor

        transform: Translate {
            id: histScale

            Behavior on x {
                enabled: DbMain.enableLevelMetersAnimation

                NumberAnimation {
                    duration: DbMain.levelMetersAnimationDuration
                    easing.type: Easing.OutCubic
                }
            }
        }

        anchors {
            bottom: parent.bottom
            top: parent.top
        }
    }

    Item {
        id: labelsItem

        anchors.fill: parent
        implicitHeight: valueLabel.implicitHeight

        Label {
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter
            text: control.label
            elide: control.elide
            color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
            wrapMode: control.wrapMode
            maximumLineCount: 1

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
                leftMargin: Kirigami.Units.smallSpacing
            }
        }

        Label {
            id: valueLabel

            horizontalAlignment: Qt.AlignRight
            verticalAlignment: Qt.AlignVCenter
            text: Number(0).toLocaleString(Qt.locale(), 'f', control.decimals) + unitSuffix
            elide: control.elide
            color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
            wrapMode: control.wrapMode
            maximumLineCount: 1

            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
                rightMargin: Kirigami.Units.smallSpacing
            }
        }
    }

    Timer {
        id: sampleTimer

        property real value: control.value

        interval: DbMain.levelMetersLabelTimer
        repeat: true
        running: control.visible

        onTriggered: {
            value = control.value;
        }
    }
}
