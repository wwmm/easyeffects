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
import ee.ui
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    property real from: 0
    property real to: 1
    property int decimals: 2
    property bool convertDecibelToLinear: false
    property bool topToBottom: false
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

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    implicitWidth: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    implicitHeight: valueLabel.implicitWidth + Kirigami.Units.largeSpacing
    radius: Kirigami.Units.cornerRadius
    border.width: 1
    clip: true
    color: Kirigami.Theme.neutralBackgroundColor
    border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)

    Connections {
        target: EeMetersReset

        function onReset() {
            root.setValue(0);

            sampleTimer.value = 0;
            root.latestDisplayValue = 0;

            valueLabel.text = Number(0).toLocaleString(Qt.locale(), 'f', root.decimals);
        }
    }

    function setValue(value: real) {
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

        if (newDisplayValue > root.latestDisplayValue) {
            valueLabel.text = Number(root.latestDisplayValue).toLocaleString(Qt.locale(), 'f', root.decimals);
        }

        root.latestDisplayValue = newDisplayValue;

        // level rect

        const normalizedClampedValue = (root.clampedValue - root.from) * root.invRange;
        const normalizedClampedValueLinear = (Common.dbToLinear(root.clampedValue) - root.liFrom) * root.invLiRange;

        const reversedNormalizedClampedValue = (root.clampedValue - root.to) * root.invReverseRange;
        const reversedNormalizedClampedValueLinear = (Common.dbToLinear(root.clampedValue) - root.liTo) * root.invLiReverseRange;

        const newYScale = root.convertDecibelToLinear ? (root.topToBottom === false ? normalizedClampedValueLinear : reversedNormalizedClampedValueLinear) : (root.topToBottom === false ? normalizedClampedValue : reversedNormalizedClampedValue);

        if (Math.abs(levelScale.yScale - newYScale) >= 1.0e-3) {
            levelScale.yScale = newYScale;
        }

        //hist rect

        const clampedNewDisplayValue = Common.clamp(newDisplayValue, root.from, root.to);

        const liFrac = (Common.dbToLinear(clampedNewDisplayValue) - root.liFrom) * root.invLiRange;
        const frac = (clampedNewDisplayValue - root.from) * root.invRange;

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

        onTriggered: function (): void {
            value = root.value;
            valueLabel.text = Number(root.latestDisplayValue).toLocaleString(Qt.locale(), 'f', root.decimals);
        }
    }
}
