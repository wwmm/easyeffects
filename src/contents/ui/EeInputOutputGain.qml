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
import "Common.js" as Common
import org.kde.kirigami as Kirigami

Item {
    id: control

    required property var pluginDB
    readonly property real radius: 2.5 * Kirigami.Units.gridUnit

    implicitHeight: column.implicitHeight
    implicitWidth: column.implicitWidth

    Accessible.role: Accessible.Grouping
    Accessible.name: i18n("Audio levels control") // qmllint disable
    Accessible.description: i18n("Input and output gain controls with level meters") // qmllint disable

    function setInputLevelLeft(value) {
        inputRow.setLevelLeft(value);
    }

    function setInputLevelRight(value) {
        inputRow.setLevelRight(value);
    }

    function setOutputLevelLeft(value) {
        outputRow.setLevelLeft(value);
    }

    function setOutputLevelRight(value) {
        outputRow.setLevelRight(value);
    }

    component GainRow: RowLayout {

        property alias label: gain.label
        property alias unit: gain.unit
        property alias from: gain.from
        property alias to: gain.to
        property alias value: gain.value

        signal gainChanged(real v)

        function setLevelLeft(value) {
            left.setValue(value);
        }

        function setLevelRight(value) {
            right.setValue(value);
        }

        EeSpinBox {
            id: gain

            horizontalPadding: 0
            verticalPadding: 0
            labelFillWidth: true
            labelAbove: true
            spinboxLayoutFillWidth: true
            decimals: 2
            stepSize: 0.1
            boxWidth: 5 * Kirigami.Units.gridUnit
            onValueModified: v => {
                parent.gainChanged(v);
            }
        }

        EeAudioLevel {
            id: left

            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            implicitWidth: control.radius
            implicitHeight: control.radius
            from: Common.minimumDecibelLevel
            to: 0
            decimals: 0
            convertDecibelToLinear: true
        }

        EeAudioLevel {
            id: right

            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            implicitWidth: control.radius
            implicitHeight: control.radius
            from: Common.minimumDecibelLevel
            to: 0
            decimals: 0
            convertDecibelToLinear: true
        }
    }

    ColumnLayout {
        id: column

        anchors.fill: parent

        Kirigami.CardsLayout {
            id: grid

            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.leftMargin: 3 * Kirigami.Units.mediumSpacing
            Layout.rightMargin: 3 * Kirigami.Units.mediumSpacing
            uniformCellWidths: true

            GainRow {
                id: inputRow

                label: i18n("Input") // qmllint disable
                from: control.pluginDB.getMinValue("inputGain")
                to: control.pluginDB.getMaxValue("inputGain")
                value: control.pluginDB.inputGain
                unit: Units.dB
                onGainChanged: v => {
                    control.pluginDB.inputGain = v;
                }
            }

            GainRow {
                id: outputRow

                label: i18n("Output") // qmllint disable
                from: control.pluginDB.getMinValue("outputGain")
                to: control.pluginDB.getMaxValue("outputGain")
                value: control.pluginDB.outputGain
                unit: Units.dB
                onGainChanged: v => {
                    control.pluginDB.outputGain = v;
                }
            }
        }
    }
}
