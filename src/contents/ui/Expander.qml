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
    id: expanderPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(expanderPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(expanderPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(expanderPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(expanderPage.pluginBackend.getOutputLevelRight());
        reductionLevelLeft.setValue(expanderPage.pluginBackend.getReductionLevelLeft());
        reductionLevelRight.setValue(expanderPage.pluginBackend.getReductionLevelRight());
        sideChainLevelLeft.setValue(expanderPage.pluginBackend.getSideChainLevelLeft());
        sideChainLevelRight.setValue(expanderPage.pluginBackend.getSideChainLevelRight());
        curveLevelLeft.setValue(expanderPage.pluginBackend.getCurveLevelLeft());
        curveLevelRight.setValue(expanderPage.pluginBackend.getCurveLevelRight());
        envelopeLevelLeft.setValue(expanderPage.pluginBackend.getEnvelopeLevelLeft());
        envelopeLevelRight.setValue(expanderPage.pluginBackend.getEnvelopeLevelRight());
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            maximumColumns: 6
            uniformCellWidths: true

            EeCard {
                title: i18n("Expander") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    Layout.alignment: Qt.AlignTop

                    FormCard.FormComboBoxDelegate {
                        id: mode

                        Layout.columnSpan: 2
                        verticalPadding: Kirigami.Units.smallSpacing
                        text: i18n("Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: expanderPage.pluginDB.mode
                        editable: false
                        model: [i18n("Downward"), i18n("Upward")] // qmllint disable
                        onActivated: idx => {
                            expanderPage.pluginDB.mode = idx;
                        }
                    }

                    EeSpinBox {
                        id: ratio

                        label: i18n("Ratio") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("ratio")
                        to: expanderPage.pluginDB.getMaxValue("ratio")
                        value: expanderPage.pluginDB.ratio
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            expanderPage.pluginDB.ratio = v;
                        }
                    }

                    EeSpinBox {
                        id: knee

                        label: i18n("Knee") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("knee")
                        to: expanderPage.pluginDB.getMaxValue("knee")
                        value: expanderPage.pluginDB.knee
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        onValueModified: v => {
                            expanderPage.pluginDB.knee = v;
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }

            EeCard {
                title: i18n("Threshold and Time") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0

                    EeSpinBox {
                        id: threshold

                        label: i18n("Attack") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("threshold")
                        to: expanderPage.pluginDB.getMaxValue("threshold")
                        value: expanderPage.pluginDB.threshold
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        onValueModified: v => {
                            expanderPage.pluginDB.threshold = v;
                        }
                    }

                    EeSpinBox {
                        id: attack

                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("attack")
                        to: expanderPage.pluginDB.getMaxValue("attack")
                        value: expanderPage.pluginDB.attack
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            expanderPage.pluginDB.attack = v;
                        }
                    }

                    EeSpinBox {
                        id: releaseThreshold

                        label: i18n("Release") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("releaseThreshold")
                        to: expanderPage.pluginDB.getMaxValue("releaseThreshold")
                        value: expanderPage.pluginDB.releaseThreshold
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.releaseThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: release

                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("release")
                        to: expanderPage.pluginDB.getMaxValue("release")
                        value: expanderPage.pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            expanderPage.pluginDB.release = v;
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }

            EeCard {
                title: i18n("Sidechain") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0

                    FormCard.FormComboBoxDelegate {
                        id: sidechainType

                        Layout.columnSpan: 2
                        verticalPadding: Kirigami.Units.smallSpacing
                        text: i18n("Type") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: expanderPage.pluginDB.sidechainType
                        editable: false
                        model: [i18n("Internal"), i18n("External"), i18n("Link")] // qmllint disable
                        onActivated: idx => {
                            expanderPage.pluginDB.sidechainType = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: comboSideChainInputDevice

                        Layout.columnSpan: 2
                        verticalPadding: Kirigami.Units.smallSpacing
                        text: i18n("Input device") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        editable: false
                        model: PW.ModelNodes
                        textRole: "description"
                        enabled: sidechainType.currentIndex === 1
                        currentIndex: {
                            for (let n = 0; n < PW.ModelNodes.rowCount(); n++) {
                                if (PW.ModelNodes.getNodeName(n) === expanderPage.pluginDB.sidechainInputDevice)
                                    return n;
                            }
                            return 0;
                        }
                        onActivated: idx => {
                            let selectedName = PW.ModelNodes.getNodeName(idx);
                            if (selectedName !== expanderPage.pluginDB.sidechainInputDevice)
                                expanderPage.pluginDB.sidechainInputDevice = selectedName;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: sidechainMode

                        verticalPadding: Kirigami.Units.smallSpacing
                        text: i18n("Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: expanderPage.pluginDB.sidechainMode
                        editable: false
                        model: [i18n("Peak"), i18n("RMS"), i18n("Low-pass"), i18n("SMA")] // qmllint disable
                        onActivated: idx => {
                            expanderPage.pluginDB.sidechainMode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: sidechainSource

                        verticalPadding: Kirigami.Units.smallSpacing
                        text: i18n("Source") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: expanderPage.pluginDB.sidechainSource
                        editable: false
                        model: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")] // qmllint disable
                        visible: !expanderPage.pluginDB.stereoSplit
                        onActivated: idx => {
                            expanderPage.pluginDB.sidechainSource = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: stereoSplitSource

                        verticalPadding: Kirigami.Units.smallSpacing
                        text: i18n("Source") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: expanderPage.pluginDB.stereoSplitSource
                        editable: false
                        model: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")] // qmllint disable
                        visible: expanderPage.pluginDB.stereoSplit
                        onActivated: idx => {
                            expanderPage.pluginDB.stereoSplitSource = idx;
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

                    EeSpinBox {
                        id: sidechainPreamp

                        label: i18n("Preamp") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("sidechainPreamp")
                        to: expanderPage.pluginDB.getMaxValue("sidechainPreamp")
                        value: expanderPage.pluginDB.sidechainPreamp
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.sidechainPreamp = v;
                        }
                    }

                    EeSpinBox {
                        id: sidechainReactivity

                        label: i18n("Reactivity") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("sidechainReactivity")
                        to: expanderPage.pluginDB.getMaxValue("sidechainReactivity")
                        value: expanderPage.pluginDB.sidechainReactivity
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            expanderPage.pluginDB.sidechainReactivity = v;
                        }
                    }

                    EeSpinBox {
                        id: sidechainLookahead

                        Layout.columnSpan: 2
                        label: i18n("Lookahead") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("sidechainLookahead")
                        to: expanderPage.pluginDB.getMaxValue("sidechainLookahead")
                        value: expanderPage.pluginDB.sidechainLookahead
                        decimals: 3
                        stepSize: 0.001
                        unit: Units.ms
                        onValueModified: v => {
                            expanderPage.pluginDB.sidechainLookahead = v;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: hpfMode

                        verticalPadding: 0
                        text: i18n("High-pass") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: expanderPage.pluginDB.hpfMode
                        editable: false
                        model: [i18n("Off"), `12 ${Units.dBoct}`, `24 ${Units.dBoct}`, `36 ${Units.dBoct}`] // qmllint disable
                        onActivated: idx => {
                            expanderPage.pluginDB.hpfMode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: lpfMode

                        verticalPadding: 0
                        text: i18n("Low-pass") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: expanderPage.pluginDB.lpfMode
                        editable: false
                        model: [i18n("Off"), `12 ${Units.dBoct}`, `24 ${Units.dBoct}`, `36 ${Units.dBoct}`] // qmllint disable
                        onActivated: idx => {
                            expanderPage.pluginDB.lpfMode = idx;
                        }
                    }

                    EeSpinBox {
                        id: hpfFrequency

                        verticalPadding: 0
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("hpfFrequency")
                        to: expanderPage.pluginDB.getMaxValue("hpfFrequency")
                        value: expanderPage.pluginDB.hpfFrequency
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        visible: hpfMode.currentIndex !== 0
                        onValueModified: v => {
                            expanderPage.pluginDB.hpfFrequency = v;
                        }
                    }

                    EeSpinBox {
                        id: lpfFrequency

                        Layout.column: 1
                        Layout.row: 3
                        verticalPadding: 0
                        labelFillWidth: false
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("lpfFrequency")
                        to: expanderPage.pluginDB.getMaxValue("lpfFrequency")
                        value: expanderPage.pluginDB.lpfFrequency
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        visible: lpfMode.currentIndex !== 0
                        onValueModified: v => {
                            expanderPage.pluginDB.lpfFrequency = v;
                        }
                    }

                    Item {
                        Layout.fillHeight: true
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
                    from: expanderPage.pluginDB.getMinValue("dry")
                    to: expanderPage.pluginDB.getMaxValue("dry")
                    value: expanderPage.pluginDB.dry
                    decimals: 2 // Required to show "-inf"
                    stepSize: 0.01
                    unit: Units.dB
                    minusInfinityMode: true
                    onValueModified: v => {
                        expanderPage.pluginDB.dry = v;
                    }
                }

                EeSpinBox {
                    id: wet

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Wet") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: expanderPage.pluginDB.getMinValue("wet")
                    to: expanderPage.pluginDB.getMaxValue("wet")
                    value: expanderPage.pluginDB.wet
                    decimals: 2 // Required to show "-inf"
                    stepSize: 0.01
                    unit: Units.dB
                    minusInfinityMode: true
                    onValueModified: v => {
                        expanderPage.pluginDB.wet = v;
                    }
                }

                EeSpinBox {
                    id: makeup

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Makeup") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: expanderPage.pluginDB.getMinValue("makeup")
                    to: expanderPage.pluginDB.getMaxValue("makeup")
                    value: expanderPage.pluginDB.makeup
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.dB
                    onValueModified: v => {
                        expanderPage.pluginDB.makeup = v;
                    }
                }
            }

            EeCard {
                title: i18n("Pre-Mix") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0

                    EeSpinBox {
                        label: i18n("Input to sidechain") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("inputToSidechain")
                        to: expanderPage.pluginDB.getMaxValue("inputToSidechain")
                        value: expanderPage.pluginDB.inputToSidechain
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.inputToSidechain = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Input to link") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("inputToLink")
                        to: expanderPage.pluginDB.getMaxValue("inputToLink")
                        value: expanderPage.pluginDB.inputToLink
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.inputToLink = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Sidechain to input") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("sidechainToInput")
                        to: expanderPage.pluginDB.getMaxValue("sidechainToInput")
                        value: expanderPage.pluginDB.sidechainToInput
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.sidechainToInput = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Sidechain to link") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("sidechainToLink")
                        to: expanderPage.pluginDB.getMaxValue("sidechainToLink")
                        value: expanderPage.pluginDB.sidechainToLink
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.sidechainToLink = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Link to sidechain") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("linkToSidechain")
                        to: expanderPage.pluginDB.getMaxValue("linkToSidechain")
                        value: expanderPage.pluginDB.linkToSidechain
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.linkToSidechain = v;
                        }
                    }

                    EeSpinBox {
                        label: i18n("Link to input") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("linkToInput")
                        to: expanderPage.pluginDB.getMaxValue("linkToInput")
                        value: expanderPage.pluginDB.linkToInput
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.linkToInput = v;
                        }
                    }
                }
            }
        }

        Kirigami.CardsLayout {
            maximumColumns: 2
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
                        to: 60
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: reductionLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 60
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: sideChainLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 36
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: sideChainLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 36
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
                        to: 36
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: curveLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 36
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: envelopeLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 36
                        decimals: 0
                    }

                    EeAudioLevel {
                        id: envelopeLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 36
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
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: expanderPage.pluginDB
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
                    checked: expanderPage.pluginBackend ? expanderPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            expanderPage.pluginBackend.showNativeUi();
                        else
                            expanderPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Listen") // qmllint disable
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: expanderPage.pluginDB.sidechainListen
                    onTriggered: {
                        if (expanderPage.pluginDB.sidechainListen != checked)
                            expanderPage.pluginDB.sidechainListen = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Stereo split") // qmllint disable
                    icon.name: "view-split-left-right-symbolic"
                    checkable: true
                    checked: expanderPage.pluginDB.stereoSplit
                    onTriggered: {
                        if (expanderPage.pluginDB.stereoSplit != checked)
                            expanderPage.pluginDB.stereoSplit = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        expanderPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
