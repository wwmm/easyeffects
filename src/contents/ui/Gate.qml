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

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.pipewire as PW
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: gatePage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(gatePage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(gatePage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(gatePage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(gatePage.pluginBackend.getOutputLevelRight());
        reductionLevelLeft.setValue(gatePage.pluginBackend.getReductionLevelLeft());
        reductionLevelRight.setValue(gatePage.pluginBackend.getReductionLevelRight());
        sideChainLevelLeft.setValue(gatePage.pluginBackend.getSideChainLevelLeft());
        sideChainLevelRight.setValue(gatePage.pluginBackend.getSideChainLevelRight());
        curveLevelLeft.setValue(gatePage.pluginBackend.getCurveLevelLeft());
        curveLevelRight.setValue(gatePage.pluginBackend.getCurveLevelRight());
        envelopeLevelLeft.setValue(gatePage.pluginBackend.getEnvelopeLevelLeft());
        envelopeLevelRight.setValue(gatePage.pluginBackend.getEnvelopeLevelRight());
        attackZoneStart.setValue(gatePage.pluginBackend.getAttackZoneStart());
        attackThreshold.setValue(gatePage.pluginBackend.getAttackThreshold());
        releaseZoneStart.setValue(gatePage.pluginBackend.getReleaseZoneStart());
        releaseThreshold.setValue(gatePage.pluginBackend.getReleaseThreshold());
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            maximumColumns: 6
            minimumColumnWidth: Kirigami.Units.gridUnit * 16
            uniformCellWidths: true

            EeCard {
                title: i18n("Gate") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    Layout.alignment: Qt.AlignTop

                    rowSpacing: 0

                    EeSpinBox {
                        id: attack

                        label: i18n("Attack") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("attack")
                        to: gatePage.pluginDB.getMaxValue("attack")
                        value: gatePage.pluginDB.attack
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            gatePage.pluginDB.attack = v;
                        }
                    }

                    EeSpinBox {
                        id: release

                        label: i18n("Release") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("release")
                        to: gatePage.pluginDB.getMaxValue("release")
                        value: gatePage.pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            gatePage.pluginDB.release = v;
                        }
                    }

                    EeSpinBox {
                        id: reduction

                        Layout.columnSpan: 2
                        label: i18n("Reduction") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("reduction")
                        to: gatePage.pluginDB.getMaxValue("reduction")
                        value: gatePage.pluginDB.reduction
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        onValueModified: v => {
                            gatePage.pluginDB.reduction = v;
                        }
                    }

                    EeSpinBox {
                        id: curveThreshold

                        label: i18n("Threshold") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("curveThreshold")
                        to: gatePage.pluginDB.getMaxValue("curveThreshold")
                        value: gatePage.pluginDB.curveThreshold
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        onValueModified: v => {
                            gatePage.pluginDB.curveThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: curveZone

                        label: i18n("Zone") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("curveZone")
                        to: gatePage.pluginDB.getMaxValue("curveZone")
                        value: gatePage.pluginDB.curveZone
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        onValueModified: v => {
                            gatePage.pluginDB.curveZone = v;
                        }
                    }
                }
            }

            EeCard {
                title: i18n("Hysteresis") // qmllint disable

                EeSwitch {
                    id: hysteresis

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Enable") // qmllint disable
                    isChecked: gatePage.pluginDB.hysteresis
                    onCheckedChanged: {
                        if (isChecked !== gatePage.pluginDB.hysteresis)
                            gatePage.pluginDB.hysteresis = isChecked;
                    }
                }

                EeSpinBox {
                    id: hysteresisThreshold

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Threshold") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: gatePage.pluginDB.getMinValue("hysteresisThreshold")
                    to: gatePage.pluginDB.getMaxValue("hysteresisThreshold")
                    value: gatePage.pluginDB.hysteresisThreshold
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.dB
                    enabled: hysteresis.isChecked
                    onValueModified: v => {
                        gatePage.pluginDB.hysteresisThreshold = v;
                    }
                }

                EeSpinBox {
                    id: hysteresisZone

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Zone") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: gatePage.pluginDB.getMinValue("hysteresisZone")
                    to: gatePage.pluginDB.getMaxValue("hysteresisZone")
                    value: gatePage.pluginDB.hysteresisZone
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.dB
                    enabled: hysteresis.isChecked
                    onValueModified: v => {
                        gatePage.pluginDB.hysteresisZone = v;
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            EeCard {
                title: i18n("Sidechain") // qmllint disable

                GridLayout {
                    columns: 2
                    rowSpacing: 0
                    uniformCellWidths: true
                    Layout.alignment: Qt.AlignTop

                    FormCard.FormComboBoxDelegate {
                        id: sidechainType

                        Layout.columnSpan: 2
                        verticalPadding: Kirigami.Units.largeSpacing
                        text: i18n("Type") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: gatePage.pluginDB.sidechainType
                        editable: false
                        model: [i18n("Internal"), i18n("External"), i18n("Link")] // qmllint disable
                        onActivated: idx => {
                            gatePage.pluginDB.sidechainType = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: comboSideChainInputDevice

                        Layout.columnSpan: 2
                        verticalPadding: Kirigami.Units.largeSpacing
                        text: i18n("Input device") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        editable: false
                        model: PW.ModelNodes
                        textRole: "description"
                        enabled: sidechainType.currentIndex === 1
                        currentIndex: {
                            for (let n = 0; n < PW.ModelNodes.rowCount(); n++) {
                                if (PW.ModelNodes.getNodeName(n) === gatePage.pluginDB.sidechainInputDevice)
                                    return n;
                            }
                            return 0;
                        }
                        onActivated: idx => {
                            let selectedName = PW.ModelNodes.getNodeName(idx);
                            if (selectedName !== gatePage.pluginDB.sidechainInputDevice)
                                gatePage.pluginDB.sidechainInputDevice = selectedName;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: sidechainMode

                        verticalPadding: Kirigami.Units.largeSpacing
                        text: i18n("Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: gatePage.pluginDB.sidechainMode
                        editable: false
                        model: [i18n("Peak"), i18n("RMS"), i18n("Low-pass"), i18n("SMA")] // qmllint disable
                        onActivated: idx => {
                            gatePage.pluginDB.sidechainMode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: sidechainSource

                        verticalPadding: Kirigami.Units.largeSpacing
                        text: i18n("Source") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: gatePage.pluginDB.sidechainSource
                        editable: false
                        model: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")] // qmllint disable
                        visible: !gatePage.pluginDB.stereoSplit
                        onActivated: idx => {
                            gatePage.pluginDB.sidechainSource = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: stereoSplitSource

                        verticalPadding: Kirigami.Units.largeSpacing
                        text: i18n("Source") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: gatePage.pluginDB.stereoSplitSource
                        editable: false
                        model: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")] // qmllint disable
                        visible: gatePage.pluginDB.stereoSplit
                        onActivated: idx => {
                            gatePage.pluginDB.stereoSplitSource = idx;
                        }
                    }
                }
            }

            EeCard {
                title: i18n("Sidechain") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0
                    Layout.alignment: Qt.AlignTop

                    EeSpinBox {
                        id: sidechainPreamp

                        label: i18n("Preamp") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("sidechainPreamp")
                        to: gatePage.pluginDB.getMaxValue("sidechainPreamp")
                        value: gatePage.pluginDB.sidechainPreamp
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            gatePage.pluginDB.sidechainPreamp = v;
                        }
                    }

                    EeSpinBox {
                        id: sidechainReactivity

                        label: i18n("Reactivity") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("sidechainReactivity")
                        to: gatePage.pluginDB.getMaxValue("sidechainReactivity")
                        value: gatePage.pluginDB.sidechainReactivity
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            gatePage.pluginDB.sidechainReactivity = v;
                        }
                    }

                    EeSpinBox {
                        id: sidechainLookahead

                        Layout.columnSpan: 2
                        label: i18n("Lookahead") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("sidechainLookahead")
                        to: gatePage.pluginDB.getMaxValue("sidechainLookahead")
                        value: gatePage.pluginDB.sidechainLookahead
                        decimals: 3
                        stepSize: 0.001
                        unit: Units.ms
                        onValueModified: v => {
                            gatePage.pluginDB.sidechainLookahead = v;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: hpfMode

                        verticalPadding: Kirigami.Units.largeSpacing
                        text: i18n("High-pass") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: gatePage.pluginDB.hpfMode
                        editable: false
                        model: [i18n("Off"), `12 ${Units.dBoct}`, `24 ${Units.dBoct}`, `36 ${Units.dBoct}`] // qmllint disable
                        onActivated: idx => {
                            gatePage.pluginDB.hpfMode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: lpfMode

                        verticalPadding: Kirigami.Units.largeSpacing
                        text: i18n("Low-pass") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: gatePage.pluginDB.lpfMode
                        editable: false
                        model: [i18n("Off"), `12 ${Units.dBoct}`, `24 ${Units.dBoct}`, `36 ${Units.dBoct}`] // qmllint disable
                        onActivated: idx => {
                            gatePage.pluginDB.lpfMode = idx;
                        }
                    }

                    EeSpinBox {
                        id: hpfFrequency

                        verticalPadding: Kirigami.Units.largeSpacing
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("hpfFrequency")
                        to: gatePage.pluginDB.getMaxValue("hpfFrequency")
                        value: gatePage.pluginDB.hpfFrequency
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        visible: hpfMode.currentIndex !== 0
                        onValueModified: v => {
                            gatePage.pluginDB.hpfFrequency = v;
                        }
                    }

                    EeSpinBox {
                        id: lpfFrequency

                        Layout.column: 1
                        Layout.row: 3
                        verticalPadding: Kirigami.Units.largeSpacing
                        labelFillWidth: false
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("lpfFrequency")
                        to: gatePage.pluginDB.getMaxValue("lpfFrequency")
                        value: gatePage.pluginDB.lpfFrequency
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        visible: lpfMode.currentIndex !== 0
                        onValueModified: v => {
                            gatePage.pluginDB.lpfFrequency = v;
                        }
                    }
                }
            }

            EeCard {
                title: i18n("Output") // qmllint disable

                EeSpinBox {
                    id: dry

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Dry") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: gatePage.pluginDB.getMinValue("dry")
                    to: gatePage.pluginDB.getMaxValue("dry")
                    value: gatePage.pluginDB.dry
                    decimals: 2 // Required to show "-inf"
                    stepSize: 0.01
                    unit: Units.dB
                    minusInfinityMode: true
                    onValueModified: v => {
                        gatePage.pluginDB.dry = v;
                    }
                }

                EeSpinBox {
                    id: wet

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Wet") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: gatePage.pluginDB.getMinValue("wet")
                    to: gatePage.pluginDB.getMaxValue("wet")
                    value: gatePage.pluginDB.wet
                    decimals: 2 // Required to show "-inf"
                    stepSize: 0.01
                    unit: Units.dB
                    minusInfinityMode: true
                    onValueModified: v => {
                        gatePage.pluginDB.wet = v;
                    }
                }

                EeSpinBox {
                    id: makeup

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Makeup") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: gatePage.pluginDB.getMinValue("makeup")
                    to: gatePage.pluginDB.getMaxValue("makeup")
                    value: gatePage.pluginDB.makeup
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.dB
                    onValueModified: v => {
                        gatePage.pluginDB.makeup = v;
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            EeCard {
                title: i18n("Pre-Mix") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    Layout.alignment: Qt.AlignTop

                    rowSpacing: 0

                    EeSpinBox {
                        label: i18n("Input to sidechain") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("inputToSidechain")
                        to: gatePage.pluginDB.getMaxValue("inputToSidechain")
                        value: gatePage.pluginDB.inputToSidechain
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            gatePage.pluginDB.inputToSidechain = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Input to link") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("inputToLink")
                        to: gatePage.pluginDB.getMaxValue("inputToLink")
                        value: gatePage.pluginDB.inputToLink
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            gatePage.pluginDB.inputToLink = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Sidechain to input") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("sidechainToInput")
                        to: gatePage.pluginDB.getMaxValue("sidechainToInput")
                        value: gatePage.pluginDB.sidechainToInput
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            gatePage.pluginDB.sidechainToInput = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Sidechain to link") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("sidechainToLink")
                        to: gatePage.pluginDB.getMaxValue("sidechainToLink")
                        value: gatePage.pluginDB.sidechainToLink
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            gatePage.pluginDB.sidechainToLink = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Link to sidechain") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("linkToSidechain")
                        to: gatePage.pluginDB.getMaxValue("linkToSidechain")
                        value: gatePage.pluginDB.linkToSidechain
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            gatePage.pluginDB.linkToSidechain = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Link to input") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("linkToInput")
                        to: gatePage.pluginDB.getMaxValue("linkToInput")
                        value: gatePage.pluginDB.linkToInput
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            gatePage.pluginDB.linkToInput = v;
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }

        Kirigami.CardsLayout {
            maximumColumns: 3
            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            uniformCellWidths: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            Kirigami.Card {
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignHCenter

                contentItem: GridLayout {
                    readonly property real radius: 2.5 * Kirigami.Units.gridUnit

                    columnSpacing: Kirigami.Units.largeSpacing
                    rowSpacing: Kirigami.Units.largeSpacing
                    columns: 4
                    rows: 3

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Reduction") // qmllint disable
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Sidechain") // qmllint disable
                    }

                    EeAudioLevel {
                        id: reductionLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        decimals: 0
                        topToBottom: true
                    }

                    EeAudioLevel {
                        id: reductionLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        decimals: 0
                        topToBottom: true
                    }

                    EeAudioLevel {
                        id: sideChainLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 24
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: sideChainLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 24
                        decimals: 0
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: Units.leftCh
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: Units.rightCh
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: Units.leftCh
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: Units.rightCh
                    }
                }
            }

            Kirigami.Card {
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignHCenter

                contentItem: GridLayout {
                    readonly property real radius: 2.5 * Kirigami.Units.gridUnit

                    columnSpacing: Kirigami.Units.largeSpacing
                    rowSpacing: Kirigami.Units.largeSpacing
                    columns: 4
                    rows: 3

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Curve") // qmllint disable
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Envelope") // qmllint disable
                    }

                    EeAudioLevel {
                        id: curveLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 24
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: curveLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 24
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: envelopeLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 24
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: envelopeLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 24
                        decimals: 0
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: Units.leftCh
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: Units.rightCh
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: Units.leftCh
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: Units.rightCh
                    }
                }
            }

            Kirigami.Card {
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignHCenter

                contentItem: GridLayout {
                    readonly property real radius: 2.5 * Kirigami.Units.gridUnit

                    columnSpacing: Kirigami.Units.smallSpacing
                    rowSpacing: Kirigami.Units.largeSpacing
                    columns: 4
                    rows: 3

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Attack") // qmllint disable
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Release") // qmllint disable
                    }

                    EeAudioLevel {
                        id: attackZoneStart

                        Layout.alignment: Qt.AlignBottom | Text.AlignHCenter
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: attackThreshold

                        Layout.alignment: Qt.AlignBottom | Text.AlignHCenter
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: releaseZoneStart

                        Layout.alignment: Qt.AlignBottom | Text.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: releaseThreshold

                        Layout.alignment: Qt.AlignBottom | Text.AlignHCenter
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        decimals: 0
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Start") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Threshold") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Start") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Threshold") // qmllint disable
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: gatePage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.lsp}</strong>`) // qmllint disable
            textFormat: Text.RichText
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: false
            Layout.leftMargin: Kirigami.Units.mediumSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 8
            color: Kirigami.Theme.disabledTextColor
        }

        Kirigami.ActionToolBar {
            Layout.margins: Kirigami.Units.smallSpacing
            alignment: Qt.AlignRight
            position: Controls.ToolBar.Footer
            flat: true
            actions: [
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Show native window") // qmllint disable
                    icon.name: "window-duplicate-symbolic"
                    enabled: DbMain.showNativePluginUi
                    checkable: true
                    checked: gatePage.pluginBackend ? gatePage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            gatePage.pluginBackend.showNativeUi();
                        else
                            gatePage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Listen") // qmllint disable
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: gatePage.pluginDB.sidechainListen
                    onTriggered: {
                        if (gatePage.pluginDB.sidechainListen != checked)
                            gatePage.pluginDB.sidechainListen = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Stereo split") // qmllint disable
                    icon.name: "view-split-left-right-symbolic"
                    checkable: true
                    checked: gatePage.pluginDB.stereoSplit
                    onTriggered: {
                        if (gatePage.pluginDB.stereoSplit != checked)
                            gatePage.pluginDB.stereoSplit = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        gatePage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
