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
    property real clampedValue: 0
    property real displayValue: 0
    readonly property real dbFrom: Common.dbToLinear(from)
    readonly property real dbTo: Common.dbToLinear(to)
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
        const newC = Common.clamp(value, control.from, control.to);

        // Only update if meaningfully different

        if (Math.abs(newC - control.clampedValue) > decimalFactor) {
            control.clampedValue = newC;
        }

        let newDisplayValue;

        if (control.rightToLeft === false) {
            newDisplayValue = control.clampedValue > sampleTimer.value ? control.clampedValue : sampleTimer.value;
        } else {
            newDisplayValue = control.clampedValue < sampleTimer.value ? control.clampedValue : sampleTimer.value;
        }

        const normalizedClampedValue = (control.clampedValue - control.from) / (control.to - control.from);
        const normalizedClampedValueDB = (Common.dbToLinear(control.clampedValue) - control.dbFrom) / (control.dbTo - control.dbFrom);

        const rlNormalizedClampedValue = (control.clampedValue - control.to) / (control.from - control.to);
        const rlNormalizedClampedValueDB = (Common.dbToLinear(control.clampedValue) - control.dbTo) / (control.dbFrom - control.dbTo);

        // level rect

        if (control.convertDecibelToLinear) {
            levelScale.xScale = control.rightToLeft === false ? normalizedClampedValueDB : rlNormalizedClampedValueDB;
        } else {
            levelScale.xScale = control.rightToLeft === false ? normalizedClampedValue : rlNormalizedClampedValue;
        }

        if (newDisplayValue !== control.displayValue) {
            control.displayValue = newDisplayValue;

            const normalizedDisplayValue = (control.displayValue - control.from) / (control.to - control.from);
            const normalizedDisplayValueDB = (Common.dbToLinear(control.displayValue) - control.dbFrom) / (control.dbTo - control.dbFrom);

            const rlNormalizedDisplayValue = (control.displayValue - control.to) / (control.from - control.to);
            const rlNormalizedDisplayValueDB = (Common.dbToLinear(control.displayValue) - control.dbTo) / (control.dbFrom - control.dbTo);

            // hist rect

            if (control.convertDecibelToLinear) {
                histScale.x = control.rightToLeft === false ? normalizedDisplayValueDB * control.width : control.width - rlNormalizedDisplayValueDB * control.width;
            } else {
                histScale.x = control.rightToLeft === false ? normalizedDisplayValue * control.width : control.width - rlNormalizedDisplayValue * control.width;
            }

            //label

            valueLabel.text = Number(control.displayValue).toLocaleString(Qt.locale(), 'f', control.decimals) + unitSuffix;
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
        z: 2
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
            text: ""
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

        property real value: control.clampedValue

        interval: DbMain.levelMetersLabelTimer
        repeat: true
        running: control.visible

        onTriggered: {
            value = control.clampedValue;
        }
    }
}
