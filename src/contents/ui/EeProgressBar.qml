import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Common.js" as Common
import org.kde.kirigami as Kirigami

Control {
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
    property real normalizedClampedValue: 0
    property real normalizedClampedValueDB: 0
    property real normalizedDisplayValue: 0
    property real normalizedDisplayValueDB: 0
    readonly property real dbFrom: Common.dbToLinear(from)
    readonly property real dbTo: Common.dbToLinear(to)
    readonly property real decimalFactor: Math.pow(10, -decimals)

    readonly property string unitSuffix: if (!Common.isEmpty(control.unit)) {
        const split = control.separateUnit ? ' ' : '';
        return `${split}${control.unit}`;
    } else {
        return "";
    }

    Kirigami.Theme.colorSet: Kirigami.Theme.View

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    background: null

    Layout.fillWidth: true

    function setValue(value) {
        const newC = Common.clamp(value, control.from, control.to);

        // Only update if meaningfully different

        if (Math.abs(newC - control.clampedValue) > decimalFactor) {
            control.clampedValue = newC;
        }

        // For the history/peak indicator

        if (control.rightToLeft === false) {
            control.displayValue = control.clampedValue > sampleTimer.value ? control.clampedValue : sampleTimer.value;
        } else {
            control.displayValue = control.clampedValue < sampleTimer.value ? control.clampedValue : sampleTimer.value;
        }

        normalizedClampedValue = (control.clampedValue - control.from) / (control.to - control.from);
        normalizedClampedValueDB = (Common.dbToLinear(control.clampedValue) - control.dbFrom) / (control.dbTo - control.dbFrom);

        normalizedDisplayValue = (control.displayValue - control.from) / (control.to - control.from);
        normalizedDisplayValueDB = (Common.dbToLinear(control.displayValue) - control.dbFrom) / (control.dbTo - control.dbFrom);

        // level rect

        if (control.convertDecibelToLinear) {
            levelScale.xScale = control.rightToLeft === false ? control.normalizedClampedValueDB : (Common.dbToLinear(control.clampedValue) - control.dbTo) / (control.dbFrom - control.dbTo);
        } else {
            levelScale.xScale = control.rightToLeft === false ? control.normalizedClampedValue : (control.clampedValue - control.to) / (control.from - control.to);
        }

        // hist rect

        if (control.convertDecibelToLinear) {
            histScale.x = control.rightToLeft === false ? control.normalizedDisplayValueDB * item.width : item.width - (Common.dbToLinear(control.displayValue) - control.dbTo) / (control.dbFrom - control.dbTo) * item.width;
        } else {
            histScale.x = control.rightToLeft === false ? control.normalizedDisplayValue * item.width : item.width - (control.displayValue - control.to) / (control.from - control.to) * item.width;
        }

        //label

        valueLabel.text = Number(control.displayValue).toLocaleString(Qt.locale(), 'f', control.decimals) + unitSuffix;
    }

    contentItem: Rectangle {
        id: item

        color: Kirigami.Theme.neutralBackgroundColor
        radius: Kirigami.Units.cornerRadius

        implicitHeight: Kirigami.Units.largeSpacing * 3

        Rectangle {
            id: levelRect

            width: parent.width
            height: parent.height
            color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.3)

            transform: Scale {
                id: levelScale

                origin.x: control.rightToLeft === false ? 0 : levelRect.width

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
                id: valueLabel

                Layout.rightMargin: Kirigami.Units.smallSpacing
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                text: ""
                elide: control.elide
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: 1
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
