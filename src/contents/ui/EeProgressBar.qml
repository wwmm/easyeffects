/**
 * Copyright © 2025-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import ee.ui
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
    property real latestDisplayValue: 0
    readonly property real liFrom: Common.dbToLinear(from)
    readonly property real liTo: Common.dbToLinear(to)
    readonly property real invRange: to !== from ? 1.0 / (to - from) : 0
    readonly property real invReverseRange: from !== to ? 1.0 / (from - to) : 0
    readonly property real invLiRange: liTo !== liFrom ? 1.0 / (liTo - liFrom) : 0
    readonly property real invLiReverseRange: liFrom !== liTo ? 1.0 / (liFrom - liTo) : 0
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

    Connections {
        target: EeMetersReset // qmllint disable

        function onReset() {
            control.setValue(0);

            sampleTimer.value = 0;
            control.latestDisplayValue = 0;

            valueLabel.text = Number(0).toLocaleString(Qt.locale(), 'f', control.decimals) + control.unitSuffix;
        }
    }

    function setValue(value: real) {

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

        if (newDisplayValue > control.latestDisplayValue) {
            valueLabel.text = Number(control.latestDisplayValue).toLocaleString(Qt.locale(), 'f', control.decimals) + control.unitSuffix;
        }

        control.latestDisplayValue = newDisplayValue;

        const normalizedClampedValue = (control.clampedValue - control.from) * control.invRange;
        const normalizedClampedValueLinear = (Common.dbToLinear(control.clampedValue) - control.liFrom) * control.invLiRange;

        const rlNormalizedClampedValue = (control.clampedValue - control.to) * control.invReverseRange;
        const rlNormalizedClampedValueLinear = (Common.dbToLinear(control.clampedValue) - control.liTo) * control.invLiReverseRange;

        // level rect

        const newXScale = control.convertDecibelToLinear ? (control.rightToLeft === false ? normalizedClampedValueLinear : rlNormalizedClampedValueLinear) : (control.rightToLeft === false ? normalizedClampedValue : rlNormalizedClampedValue);

        if (Math.abs(levelScale.xScale - newXScale) >= 1.0e-3) {
            levelScale.xScale = newXScale;
        }

        const clampedNewDisplayValue = Common.clamp(newDisplayValue, control.from, control.to);

        const normalizedDisplayValue = (clampedNewDisplayValue - control.from) * invRange;
        const normalizedDisplayValueLinear = (Common.dbToLinear(clampedNewDisplayValue) - control.liFrom) * invLiRange;

        const rlNormalizedDisplayValue = (clampedNewDisplayValue - control.to) * invReverseRange;
        const rlNormalizedDisplayValueLinear = (Common.dbToLinear(clampedNewDisplayValue) - control.liTo) * invLiReverseRange;

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

        Text {
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

        Text {
            id: valueLabel

            horizontalAlignment: Qt.AlignRight
            verticalAlignment: Qt.AlignVCenter
            text: Number(0).toLocaleString(Qt.locale(), 'f', control.decimals) + control.unitSuffix
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

        onTriggered: function (): void {
            value = control.value;
            valueLabel.text = Number(control.latestDisplayValue).toLocaleString(Qt.locale(), 'f', control.decimals) + control.unitSuffix;
        }
    }
}
