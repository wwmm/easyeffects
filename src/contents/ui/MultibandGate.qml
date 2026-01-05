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
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.pipewire as PW
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: multibandGatePage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend
    readonly property string bandId: "band" + bandsListview.currentIndex
    property list<real> bandFrequencyEnd: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandReductionLevelLeft: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandReductionLevelRight: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandEnvelopeLevelLeft: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandEnvelopeLevelRight: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandCurveLevelLeft: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandCurveLevelRight: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    readonly property real radius: 2.5 * Kirigami.Units.gridUnit

    function updateMeters() {
        if (!multibandGatePage.pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(multibandGatePage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(multibandGatePage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(multibandGatePage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(multibandGatePage.pluginBackend.getOutputLevelRight());
        bandFrequencyEnd = multibandGatePage.pluginBackend.getFrequencyRangeEnd();
        bandReductionLevelLeft = multibandGatePage.pluginBackend.getReductionLevelLeft();
        bandReductionLevelRight = multibandGatePage.pluginBackend.getReductionLevelRight();
        bandEnvelopeLevelLeft = multibandGatePage.pluginBackend.getEnvelopeLevelLeft();
        bandEnvelopeLevelRight = multibandGatePage.pluginBackend.getEnvelopeLevelRight();
        bandCurveLevelLeft = multibandGatePage.pluginBackend.getCurveLevelLeft();
        bandCurveLevelRight = multibandGatePage.pluginBackend.getCurveLevelRight();

        reductionLevelLeft.setValue(Common.toLocaleLabel(multibandGatePage.bandReductionLevelLeft[bandsListview.currentIndex] ?? 0, 0, ""));
        reductionLevelRight.setValue(Common.toLocaleLabel(multibandGatePage.bandReductionLevelRight[bandsListview.currentIndex] ?? 0, 0, ""));

        envelopeLevelLeft.setValue(Common.toLocaleLabel(multibandGatePage.bandEnvelopeLevelLeft[bandsListview.currentIndex] ?? 0, 0, ""));
        envelopeLevelRight.setValue(Common.toLocaleLabel(multibandGatePage.bandEnvelopeLevelRight[bandsListview.currentIndex] ?? 0, 0, ""));

        curveLevelLeft.setValue(Common.toLocaleLabel(multibandGatePage.bandCurveLevelLeft[bandsListview.currentIndex] ?? 0, 0, ""));
        curveLevelRight.setValue(Common.toLocaleLabel(multibandGatePage.bandCurveLevelRight[bandsListview.currentIndex] ?? 0, 0, ""));
    }

    Component.onCompleted: {
        multibandGatePage.pluginBackend = multibandGatePage.pipelineInstance.getPluginInstance(name);
    }

    Component {
        id: bandGateControls

        ColumnLayout {
            spacing: Kirigami.Units.gridUnit

            Kirigami.ActionToolBar {
                Layout.margins: Kirigami.Units.smallSpacing
                alignment: Qt.AlignHCenter
                position: Controls.ToolBar.Header
                flat: false

                actions: [
                    Kirigami.Action {
                        text: i18n("Mute") // qmllint disable
                        checkable: true
                        checked: multibandGatePage.pluginDB[multibandGatePage.bandId + "Mute"]
                        onTriggered: {
                            if (multibandGatePage.pluginDB[multibandGatePage.bandId + "Mute"] != checked)
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "Mute"] = checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Solo") // qmllint disable
                        checkable: true
                        checked: multibandGatePage.pluginDB[multibandGatePage.bandId + "Solo"]
                        onTriggered: {
                            if (multibandGatePage.pluginDB[multibandGatePage.bandId + "Solo"] != checked)
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "Solo"] = checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Bypass") // qmllint disable
                        checkable: true
                        checked: !multibandGatePage.pluginDB[multibandGatePage.bandId + "GateEnable"]
                        onTriggered: {
                            multibandGatePage.pluginDB[multibandGatePage.bandId + "GateEnable"] = !checked;
                        }
                    }
                ]
            }

            Kirigami.CardsLayout {
                maximumColumns: 5
                minimumColumnWidth: Kirigami.Units.gridUnit * 15
                uniformCellWidths: true

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Label {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: false
                            text: i18n("Frequency") // qmllint disable
                        }

                        /**
                         * This option is missing in band 0.
                         * We left it disabled, but some users were confused
                         * thinking the first band had an issue.
                         * The ideal solution would be to make it not visible,
                         * but this messes the position and the vertical
                         * alignment of the bandEndFrequency label.
                         * So the best and simple solution is setting the
                         * opacity to 0 in order to make the EeSpinBox fully
                         * transparent when the first band is selected.
                         */
                        EeSpinBox {
                            label: i18n("Start") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: bandsListview.currentIndex > 0 ? multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "SplitFrequency") : from
                            to: bandsListview.currentIndex > 0 ? multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "SplitFrequency") : to
                            value: bandsListview.currentIndex > 0 ? multibandGatePage.pluginDB[multibandGatePage.bandId + "SplitFrequency"] : 0
                            decimals: 0
                            stepSize: 1
                            unit: Units.hz
                            enabled: bandsListview.currentIndex > 0
                            opacity: bandsListview.currentIndex > 0 ? 1 : 0
                            onValueModified: v => {
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "SplitFrequency"] = v;
                            }
                        }

                        ColumnLayout {
                            Controls.Label {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: i18n("End") // qmllint disable
                            }

                            Controls.Label {
                                id: bandEndFrequency
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: Common.toLocaleLabel(multibandGatePage.bandFrequencyEnd[bandsListview.currentIndex], 0, Units.hz)
                            }
                        }
                    }
                }

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Label {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: false
                            text: i18n("Reaction") // qmllint disable
                        }

                        EeSpinBox {
                            label: i18n("Attack") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "AttackTime")
                            to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "AttackTime")
                            value: multibandGatePage.pluginDB[multibandGatePage.bandId + "AttackTime"]
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.ms
                            onValueModified: v => {
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "AttackTime"] = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Release") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "ReleaseTime")
                            to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "ReleaseTime")
                            value: multibandGatePage.pluginDB[multibandGatePage.bandId + "ReleaseTime"]
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.ms
                            onValueModified: v => {
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "ReleaseTime"] = v;
                            }
                        }
                    }
                }

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Label {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: false
                            text: i18n("Gain") // qmllint disable
                        }

                        EeSpinBox {
                            label: i18n("Reduction") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "Reduction")
                            to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "Reduction")
                            value: multibandGatePage.pluginDB[multibandGatePage.bandId + "Reduction"]
                            decimals: 1
                            stepSize: 0.1
                            unit: Units.dB
                            onValueModified: v => {
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "Reduction"] = v;
                            }
                        }

                        EeSpinBox {
                            id: bandMakeup
                            label: i18n("Makeup") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "Makeup")
                            to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "Makeup")
                            value: multibandGatePage.pluginDB[multibandGatePage.bandId + "Makeup"]
                            decimals: 1
                            stepSize: 0.1
                            unit: Units.dB
                            onValueModified: v => {
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "Makeup"] = v;
                            }
                        }
                    }
                }

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Label {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: false
                            text: i18n("Curve") // qmllint disable
                        }

                        EeSpinBox {
                            label: i18n("Threshold") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "CurveThreshold")
                            to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "CurveThreshold")
                            value: multibandGatePage.pluginDB[multibandGatePage.bandId + "CurveThreshold"]
                            decimals: 1
                            stepSize: 0.1
                            unit: Units.dB
                            onValueModified: v => {
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "CurveThreshold"] = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Zone") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "CurveZone")
                            to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "CurveZone")
                            value: multibandGatePage.pluginDB[multibandGatePage.bandId + "CurveZone"]
                            decimals: 1
                            stepSize: 0.1
                            unit: Units.dB
                            onValueModified: v => {
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "CurveZone"] = v;
                            }
                        }
                    }
                }

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Button {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignCenter
                            text: i18n("Hysteresis") // qmllint disable
                            checkable: true
                            checked: multibandGatePage.pluginDB[multibandGatePage.bandId + "Hysteresis"]
                            onCheckedChanged: {
                                if (multibandGatePage.pluginDB[multibandGatePage.bandId + "Hysteresis"] !== checked) {
                                    multibandGatePage.pluginDB[multibandGatePage.bandId + "Hysteresis"] = checked;
                                }
                            }
                        }

                        EeSpinBox {
                            label: i18n("Threshold") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "HysteresisThreshold")
                            to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "HysteresisThreshold")
                            value: multibandGatePage.pluginDB[multibandGatePage.bandId + "HysteresisThreshold"]
                            decimals: 1
                            stepSize: 0.1
                            unit: Units.dB
                            enabled: multibandGatePage.pluginDB[multibandGatePage.bandId + "Hysteresis"]
                            onValueModified: v => {
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "HysteresisThreshold"] = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Zone") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "HysteresisZone")
                            to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "HysteresisZone")
                            value: multibandGatePage.pluginDB[multibandGatePage.bandId + "HysteresisZone"]
                            decimals: 1
                            stepSize: 0.1
                            unit: Units.dB
                            enabled: multibandGatePage.pluginDB[multibandGatePage.bandId + "Hysteresis"]
                            onValueModified: v => {
                                multibandGatePage.pluginDB[multibandGatePage.bandId + "HysteresisZone"] = v;
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: bandSidechainControls

        ColumnLayout {
            Kirigami.CardsLayout {
                maximumColumns: 8

                FormCard.FormComboBoxDelegate {
                    text: i18n("Type") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainType"]
                    editable: false
                    model: [i18n("Internal"), i18n("External"), i18n("Link")] // qmllint disable
                    onActivated: idx => {
                        multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainType"] = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Mode") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainMode"]
                    editable: false
                    model: [i18n("Peak"), i18n("RMS"), i18n("Low-pass"), i18n("SMA")] // qmllint disable
                    onActivated: idx => {
                        multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainMode"] = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Source") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainSource"]
                    editable: false
                    model: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")] // qmllint disable
                    visible: !multibandGatePage.pluginDB.stereoSplit
                    onActivated: idx => {
                        multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainSource"] = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Source") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: multibandGatePage.pluginDB[multibandGatePage.bandId + "StereoSplitSource"]
                    editable: false
                    model: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")] // qmllint disable
                    visible: multibandGatePage.pluginDB.stereoSplit
                    onActivated: idx => {
                        multibandGatePage.pluginDB[multibandGatePage.bandId + "StereoSplitSource"] = idx;
                    }
                }

                EeSpinBox {
                    label: i18n("Preamp") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "SidechainPreamp")
                    to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainPreamp")
                    value: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainPreamp"]
                    decimals: 2 // Required to show "-inf"
                    stepSize: 0.01
                    unit: Units.dB
                    minusInfinityMode: true
                    onValueModified: v => {
                        multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainPreamp"] = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Reactivity") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "SidechainReactivity")
                    to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainReactivity")
                    value: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainReactivity"]
                    decimals: 1
                    stepSize: 0.1
                    unit: Units.ms
                    onValueModified: v => {
                        multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainReactivity"] = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Lookahead") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "SidechainLookahead")
                    to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainLookahead")
                    value: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainLookahead"]
                    decimals: 1
                    stepSize: 0.1
                    unit: Units.ms
                    onValueModified: v => {
                        multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainLookahead"] = v;
                    }
                }

                ColumnLayout {
                    Controls.CheckBox {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Low-cut") // qmllint disable
                        checked: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainCustomLowcutFilter"]
                        onCheckedChanged: {
                            multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainCustomLowcutFilter"] = checked;
                        }
                    }

                    EeSpinBox {
                        spinboxLayoutFillWidth: true
                        from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "SidechainLowcutFrequency")
                        to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainLowcutFrequency")
                        value: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainLowcutFrequency"]
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        enabled: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainCustomLowcutFilter"]
                        onValueModified: v => {
                            multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainLowcutFrequency"] = v;
                        }
                    }
                }

                ColumnLayout {
                    Controls.CheckBox {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("High-cut") // qmllint disable
                        checked: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainCustomHighcutFilter"]
                        onCheckedChanged: {
                            multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainCustomHighcutFilter"] = checked;
                        }
                    }

                    EeSpinBox {
                        spinboxLayoutFillWidth: true
                        from: multibandGatePage.pluginDB.getMinValue(multibandGatePage.bandId + "SidechainHighcutFrequency")
                        to: multibandGatePage.pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainHighcutFrequency")
                        value: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainHighcutFrequency"]
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        enabled: multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainCustomHighcutFilter"]
                        onValueModified: v => {
                            multibandGatePage.pluginDB[multibandGatePage.bandId + "SidechainHighcutFrequency"] = v;
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            maximumColumns: 5

            FormCard.FormComboBoxDelegate {
                id: gateMode

                text: i18n("Operating mode") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: multibandGatePage.pluginDB.gateMode
                editable: false
                model: [i18n("Classic"), i18n("Modern"), i18n("Linear phase")] // qmllint disable
                onActivated: idx => {
                    multibandGatePage.pluginDB.gateMode = idx;
                }
            }

            FormCard.FormComboBoxDelegate {
                id: envelopeBoost

                text: i18n("Sidechain boost") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: multibandGatePage.pluginDB.envelopeBoost
                editable: false
                model: [i18n("None"), i18n("Pink BT"), i18n("Pink MT"), i18n("Brown BT"), i18n("Brown MT")] // qmllint disable
                onActivated: idx => {
                    multibandGatePage.pluginDB.envelopeBoost = idx;
                }
            }

            FormCard.FormComboBoxDelegate {
                id: comboSideChainInputDevice

                Layout.preferredWidth: gateMode.implicitWidth
                text: i18n("Sidechain input device") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                editable: false
                model: PW.ModelNodes
                textRole: "description"
                enabled: multibandGatePage.pluginDB.externalSidechainEnabled
                currentIndex: {
                    for (let n = 0; n < PW.ModelNodes.rowCount(); n++) {
                        if (PW.ModelNodes.getNodeName(n) === multibandGatePage.pluginDB.sidechainInputDevice)
                            return n;
                    }
                    return 0;
                }
                onActivated: idx => {
                    let selectedName = PW.ModelNodes.getNodeName(idx);
                    if (selectedName !== multibandGatePage.pluginDB.sidechainInputDevice)
                        multibandGatePage.pluginDB.sidechainInputDevice = selectedName;
                }
            }

            EeSpinBox {
                id: dry

                label: i18n("Dry") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: multibandGatePage.pluginDB.getMinValue("dry")
                to: multibandGatePage.pluginDB.getMaxValue("dry")
                value: multibandGatePage.pluginDB.dry
                decimals: 2 // Required to show "-inf"
                stepSize: 0.01
                unit: Units.dB
                minusInfinityMode: true
                onValueModified: v => {
                    multibandGatePage.pluginDB.dry = v;
                }
            }

            EeSpinBox {
                id: wet

                label: i18n("Wet") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: multibandGatePage.pluginDB.getMinValue("wet")
                to: multibandGatePage.pluginDB.getMaxValue("wet")
                value: multibandGatePage.pluginDB.wet
                decimals: 2 // Required to show "-inf"
                stepSize: 0.01
                unit: Units.dB
                minusInfinityMode: true
                onValueModified: v => {
                    multibandGatePage.pluginDB.wet = v;
                }
            }
        }

        RowLayout {
            Kirigami.Card {
                id: bandCard

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignTop

                header: RowLayout {
                    Kirigami.Heading {
                        readonly property string bandTitleTag: multibandGatePage.pluginDB.viewSidechain ? " - " + i18n("Sidechain") : "" // qmllint disable
                        text: i18n("Band") + " " + (bandsListview.currentIndex + 1) + bandTitleTag // qmllint disable
                        level: 2
                    }

                    Kirigami.ActionToolBar {
                        Layout.margins: Kirigami.Units.smallSpacing
                        alignment: Qt.AlignRight
                        position: Controls.ToolBar.Header
                        flat: true
                        actions: [
                            Kirigami.Action {
                                id: viewLeft
                                checkable: true
                                checked: !multibandGatePage.pluginDB.viewSidechain
                                icon.name: "arrow-left-symbolic"
                                onTriggered: {
                                    if (multibandGatePage.pluginDB.viewSidechain === false) {
                                        return;
                                    }

                                    multibandGatePage.pluginDB.viewSidechain = false;
                                    bandStackview.replace(bandGateControls);
                                }
                            },
                            Kirigami.Action {
                                id: viewRight
                                checkable: true
                                checked: multibandGatePage.pluginDB.viewSidechain
                                icon.name: "arrow-right-symbolic"
                                onTriggered: {
                                    if (multibandGatePage.pluginDB.viewSidechain === true) {
                                        return;
                                    }

                                    multibandGatePage.pluginDB.viewSidechain = true;
                                    bandStackview.replace(bandSidechainControls);
                                }
                            }
                        ]
                    }
                }

                contentItem: Controls.StackView {
                    id: bandStackview

                    implicitWidth: currentItem.implicitWidth
                    implicitHeight: currentItem.implicitHeight
                    initialItem: multibandGatePage.pluginDB.viewSidechain === false ? bandGateControls : bandSidechainControls
                }

                footer: Kirigami.CardsLayout {
                    maximumColumns: 3
                    uniformCellWidths: true

                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    Controls.Frame {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: parent.columns === 3 ? false : true

                        GridLayout {

                            columnSpacing: Kirigami.Units.largeSpacing
                            rowSpacing: Kirigami.Units.largeSpacing
                            columns: 2
                            rows: 3
                            anchors.centerIn: parent

                            Controls.Label {
                                Layout.columnSpan: 2
                                Layout.alignment: Qt.AlignHCenter
                                topPadding: Kirigami.Units.smallSpacing
                                horizontalAlignment: Text.AlignHCenter
                                text: i18n("Reduction") // qmllint disable
                            }

                            EeAudioLevel {
                                id: reductionLevelLeft

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 72
                                decimals: 0
                                topToBottom: true
                            }

                            EeAudioLevel {
                                id: reductionLevelRight

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 72
                                decimals: 0
                                topToBottom: true
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
                        }
                    }

                    Controls.Frame {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: parent.columns === 3 ? false : true

                        GridLayout {
                            columnSpacing: Kirigami.Units.largeSpacing
                            rowSpacing: Kirigami.Units.largeSpacing
                            columns: 2
                            rows: 3
                            anchors.centerIn: parent

                            Controls.Label {
                                Layout.columnSpan: 2
                                Layout.alignment: Qt.AlignHCenter
                                topPadding: Kirigami.Units.smallSpacing
                                horizontalAlignment: Text.AlignHCenter
                                text: i18n("Envelope") // qmllint disable
                            }

                            EeAudioLevel {
                                id: envelopeLevelLeft

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 36
                                decimals: 0
                            }

                            EeAudioLevel {
                                id: envelopeLevelRight

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
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
                        }
                    }

                    Controls.Frame {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: parent.columns === 3 ? false : true

                        GridLayout {
                            columnSpacing: Kirigami.Units.largeSpacing
                            rowSpacing: Kirigami.Units.largeSpacing
                            columns: 2
                            rows: 3
                            anchors.centerIn: parent

                            Controls.Label {
                                Layout.columnSpan: 2
                                Layout.alignment: Qt.AlignHCenter
                                topPadding: Kirigami.Units.smallSpacing
                                horizontalAlignment: Text.AlignHCenter
                                text: i18n("Curve") // qmllint disable
                            }

                            EeAudioLevel {
                                id: curveLevelLeft

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 36
                                decimals: 0
                            }

                            EeAudioLevel {
                                id: curveLevelRight

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
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
                        }
                    }
                }
            }

            Kirigami.Card {
                Layout.fillHeight: true
                Layout.fillWidth: false
                Layout.preferredHeight: contentItem.childrenRect.height + 2 * padding
                contentItem: ListView {
                    id: bandsListview

                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredHeight: contentItem.childrenRect.height
                    implicitHeight: contentItem.childrenRect.height

                    model: 8
                    implicitWidth: contentItem.childrenRect.width
                    clip: true
                    delegate: Delegates.RoundedItemDelegate {
                        id: listItemDelegate

                        required property int index

                        width: implicitWidth
                        hoverEnabled: true
                        highlighted: ListView.isCurrentItem
                        onClicked: {
                            ListView.view.currentIndex = index;
                        }

                        contentItem: RowLayout {
                            Controls.Label {
                                Layout.fillWidth: true
                                text: i18n("Band") + " " + (listItemDelegate.index + 1) // qmllint disable
                            }

                            Controls.CheckBox {
                                readonly property string bandName: "band" + listItemDelegate.index + "Enable"
                                Layout.alignment: Qt.AlignHCenter
                                enabled: listItemDelegate.index > 0
                                checked: listItemDelegate.index > 0 ? multibandGatePage.pluginDB[bandName] : true
                                onCheckedChanged: {
                                    if (checked != multibandGatePage.pluginDB[bandName]) {
                                        multibandGatePage.pluginDB[bandName] = checked;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    LspPreMixDialog {
        id: preMixDialog

        pluginDB: multibandGatePage.pluginDB
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: multibandGatePage.pluginDB
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
                    checked: multibandGatePage.pluginBackend ? multibandGatePage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            multibandGatePage.pluginBackend.showNativeUi();
                        else
                            multibandGatePage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Stereo split") // qmllint disable
                    icon.name: "view-split-left-right-symbolic"
                    checkable: true
                    checked: multibandGatePage.pluginDB.stereoSplit
                    onTriggered: {
                        if (multibandGatePage.pluginDB.stereoSplit != checked)
                            multibandGatePage.pluginDB.stereoSplit = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Pre-Mix") // qmllint disable
                    icon.name: "channelmixer-symbolic"
                    onTriggered: {
                        preMixDialog.open();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        multibandGatePage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
