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
    id: multibandCompressorPage

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
        if (!multibandCompressorPage.pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(multibandCompressorPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(multibandCompressorPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(multibandCompressorPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(multibandCompressorPage.pluginBackend.getOutputLevelRight());
        bandFrequencyEnd = multibandCompressorPage.pluginBackend.getFrequencyRangeEnd();
        bandReductionLevelLeft = multibandCompressorPage.pluginBackend.getReductionLevelLeft();
        bandReductionLevelRight = multibandCompressorPage.pluginBackend.getReductionLevelRight();
        bandEnvelopeLevelLeft = multibandCompressorPage.pluginBackend.getEnvelopeLevelLeft();
        bandEnvelopeLevelRight = multibandCompressorPage.pluginBackend.getEnvelopeLevelRight();
        bandCurveLevelLeft = multibandCompressorPage.pluginBackend.getCurveLevelLeft();
        bandCurveLevelRight = multibandCompressorPage.pluginBackend.getCurveLevelRight();

        reductionLevelLeft.setValue(Common.toLocaleLabel(multibandCompressorPage.bandReductionLevelLeft[bandsListview.currentIndex] ?? 0, 0, ""));
        reductionLevelRight.setValue(Common.toLocaleLabel(multibandCompressorPage.bandReductionLevelRight[bandsListview.currentIndex] ?? 0, 0, ""));

        envelopeLevelLeft.setValue(Common.toLocaleLabel(multibandCompressorPage.bandEnvelopeLevelLeft[bandsListview.currentIndex] ?? 0, 0, ""));
        envelopeLevelRight.setValue(Common.toLocaleLabel(multibandCompressorPage.bandEnvelopeLevelRight[bandsListview.currentIndex] ?? 0, 0, ""));

        curveLevelLeft.setValue(Common.toLocaleLabel(multibandCompressorPage.bandCurveLevelLeft[bandsListview.currentIndex] ?? 0, 0, ""));
        curveLevelRight.setValue(Common.toLocaleLabel(multibandCompressorPage.bandCurveLevelRight[bandsListview.currentIndex] ?? 0, 0, ""));
    }

    Component.onCompleted: {
        multibandCompressorPage.pluginBackend = multibandCompressorPage.pipelineInstance.getPluginInstance(name);
    }

    Component {
        id: bandCompressorControls

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
                        checked: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Mute"]
                        onTriggered: {
                            if (multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Mute"] != checked)
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Mute"] = checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Solo") // qmllint disable
                        checkable: true
                        checked: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Solo"]
                        onTriggered: {
                            if (multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Solo"] != checked)
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Solo"] = checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Bypass") // qmllint disable
                        checkable: true
                        checked: !multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "CompressorEnable"]
                        onTriggered: {
                            multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "CompressorEnable"] = !checked;
                        }
                    }
                ]
            }

            GridLayout {
                columns: bandControlsLayout.columns === 1 ? 1 : 2
                uniformCellWidths: true
                Layout.alignment: Qt.AlignHCenter

                FormCard.FormComboBoxDelegate {
                    id: bandMode

                    Layout.fillWidth: bandControlsLayout.columns === 1 ? true : false
                    text: i18n("Compression mode") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "CompressionMode"]
                    editable: false
                    model: [i18n("Downward"), i18n("Upward"), i18n("Boosting")] // qmllint disable
                    onActivated: idx => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "CompressionMode"] = idx;
                    }
                }

                EeSpinBox {
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: bandControlsLayout.columns === 1 ? true : false
                    label: i18n("Boost threshold") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "BoostThreshold")
                    to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "BoostThreshold")
                    value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "BoostThreshold"]
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.dB
                    enabled: bandMode.currentIndex === 1
                    visible: bandMode.currentIndex === 1
                    onValueModified: v => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "BoostThreshold"] = v;
                    }
                }

                EeSpinBox {
                    Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                    Layout.fillWidth: bandControlsLayout.columns === 1 ? true : false
                    label: i18n("Boost amount") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "BoostAmount")
                    to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "BoostAmount")
                    value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "BoostAmount"]
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.dB
                    enabled: bandMode.currentIndex === 2
                    visible: bandMode.currentIndex === 2
                    onValueModified: v => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "BoostAmount"] = v;
                    }
                }
            }

            Kirigami.CardsLayout {
                id: bandControlsLayout

                maximumColumns: 4
                // The following is the minimumColumnWidth that contains
                // all the spinboxes in the gainFrame without overflowing
                // on the right border.
                minimumColumnWidth: Kirigami.Units.gridUnit * 20
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
                            from: bandsListview.currentIndex > 0 ? multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "SplitFrequency") : from
                            to: bandsListview.currentIndex > 0 ? multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "SplitFrequency") : to
                            value: bandsListview.currentIndex > 0 ? multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SplitFrequency"] : 0
                            decimals: 0
                            stepSize: 1
                            unit: Units.hz
                            enabled: bandsListview.currentIndex > 0
                            opacity: bandsListview.currentIndex > 0 ? 1 : 0
                            onValueModified: v => {
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SplitFrequency"] = v;
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
                                text: Common.toLocaleLabel(multibandCompressorPage.bandFrequencyEnd[bandsListview.currentIndex], 0, Units.hz)
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
                            text: i18n("Attack") // qmllint disable
                        }

                        EeSpinBox {
                            label: i18n("Time") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "AttackTime")
                            to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "AttackTime")
                            value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "AttackTime"]
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.ms
                            onValueModified: v => {
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "AttackTime"] = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Threshold") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "AttackThreshold")
                            to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "AttackThreshold")
                            value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "AttackThreshold"]
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.dB
                            onValueModified: v => {
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "AttackThreshold"] = v;
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
                            text: i18n("Release") // qmllint disable
                        }

                        EeSpinBox {
                            label: i18n("Time") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "ReleaseTime")
                            to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "ReleaseTime")
                            value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "ReleaseTime"]
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.ms
                            onValueModified: v => {
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "ReleaseTime"] = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Threshold") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "ReleaseThreshold")
                            to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "ReleaseThreshold")
                            value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "ReleaseThreshold"]
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: Units.dB
                            minusInfinityMode: true
                            onValueModified: v => {
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "ReleaseThreshold"] = v;
                            }
                        }
                    }
                }

                Controls.Frame {
                    id: gainFrame

                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: bandControlsLayout.columns === 1 ? 1 : 3
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Label {
                            Layout.columnSpan: bandControlsLayout.columns === 1 ? 1 : 3
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: false
                            text: i18n("Gain") // qmllint disable
                        }

                        EeSpinBox {
                            label: i18n("Ratio") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            Layout.minimumWidth: Kirigami.Units.gridUnit * 6
                            from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "Ratio")
                            to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "Ratio")
                            value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Ratio"]
                            decimals: 1
                            stepSize: 0.1
                            onValueModified: v => {
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Ratio"] = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Knee") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            Layout.minimumWidth: Kirigami.Units.gridUnit * 6
                            from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "Knee")
                            to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "Knee")
                            value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Knee"]
                            decimals: 1
                            stepSize: 0.1
                            unit: Units.dB
                            onValueModified: v => {
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Knee"] = v;
                            }
                        }

                        EeSpinBox {
                            id: bandMakeup
                            label: i18n("Makeup") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            Layout.minimumWidth: Kirigami.Units.gridUnit * 6
                            from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "Makeup")
                            to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "Makeup")
                            value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Makeup"]
                            decimals: 1
                            stepSize: 0.1
                            unit: Units.dB
                            onValueModified: v => {
                                multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "Makeup"] = v;
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
                    currentIndex: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainType"]
                    editable: false
                    model: [i18n("Internal"), i18n("External"), i18n("Link")] // qmllint disable
                    onActivated: idx => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainType"] = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Mode") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainMode"]
                    editable: false
                    model: [i18n("Peak"), i18n("RMS"), i18n("Low-pass"), i18n("SMA")] // qmllint disable
                    onActivated: idx => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainMode"] = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Source") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainSource"]
                    editable: false
                    model: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")] // qmllint disable
                    visible: !multibandCompressorPage.pluginDB.stereoSplit
                    onActivated: idx => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainSource"] = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Source") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "StereoSplitSource"]
                    editable: false
                    model: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")] // qmllint disable
                    visible: multibandCompressorPage.pluginDB.stereoSplit
                    onActivated: idx => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "StereoSplitSource"] = idx;
                    }
                }

                EeSpinBox {
                    label: i18n("Preamp") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "SidechainPreamp")
                    to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "SidechainPreamp")
                    value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainPreamp"]
                    decimals: 2 // Required to show "-inf"
                    stepSize: 0.01
                    unit: Units.dB
                    minusInfinityMode: true
                    onValueModified: v => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainPreamp"] = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Reactivity") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "SidechainReactivity")
                    to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "SidechainReactivity")
                    value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainReactivity"]
                    decimals: 1
                    stepSize: 0.1
                    unit: Units.ms
                    onValueModified: v => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainReactivity"] = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Lookahead") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "SidechainLookahead")
                    to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "SidechainLookahead")
                    value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainLookahead"]
                    decimals: 1
                    stepSize: 0.1
                    unit: Units.ms
                    onValueModified: v => {
                        multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainLookahead"] = v;
                    }
                }

                ColumnLayout {
                    Controls.CheckBox {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Low-cut") // qmllint disable
                        checked: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainCustomLowcutFilter"]
                        onCheckedChanged: {
                            multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainCustomLowcutFilter"] = checked;
                        }
                    }

                    EeSpinBox {
                        spinboxLayoutFillWidth: true
                        from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "SidechainLowcutFrequency")
                        to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "SidechainLowcutFrequency")
                        value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainLowcutFrequency"]
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        enabled: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainCustomLowcutFilter"]
                        onValueModified: v => {
                            multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainLowcutFrequency"] = v;
                        }
                    }
                }

                ColumnLayout {
                    Controls.CheckBox {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("High-cut") // qmllint disable
                        checked: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainCustomHighcutFilter"]
                        onCheckedChanged: {
                            multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainCustomHighcutFilter"] = checked;
                        }
                    }

                    EeSpinBox {
                        spinboxLayoutFillWidth: true
                        from: multibandCompressorPage.pluginDB.getMinValue(multibandCompressorPage.bandId + "SidechainHighcutFrequency")
                        to: multibandCompressorPage.pluginDB.getMaxValue(multibandCompressorPage.bandId + "SidechainHighcutFrequency")
                        value: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainHighcutFrequency"]
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        enabled: multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainCustomHighcutFilter"]
                        onValueModified: v => {
                            multibandCompressorPage.pluginDB[multibandCompressorPage.bandId + "SidechainHighcutFrequency"] = v;
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
                id: compressorMode

                text: i18n("Operating mode") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: multibandCompressorPage.pluginDB.compressorMode
                editable: false
                model: [i18n("Classic"), i18n("Modern"), i18n("Linear phase")] // qmllint disable
                onActivated: idx => {
                    multibandCompressorPage.pluginDB.compressorMode = idx;
                }
            }

            FormCard.FormComboBoxDelegate {
                id: envelopeBoost

                text: i18n("Sidechain boost") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: multibandCompressorPage.pluginDB.envelopeBoost
                editable: false
                model: [i18n("None"), i18n("Pink BT"), i18n("Pink MT"), i18n("Brown BT"), i18n("Brown MT")] // qmllint disable
                onActivated: idx => {
                    multibandCompressorPage.pluginDB.envelopeBoost = idx;
                }
            }

            FormCard.FormComboBoxDelegate {
                id: comboSideChainInputDevice

                Layout.preferredWidth: compressorMode.implicitWidth
                text: i18n("Sidechain input device") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                editable: false
                model: PW.ModelNodes
                textRole: "description"
                enabled: multibandCompressorPage.pluginDB.externalSidechainEnabled
                currentIndex: {
                    for (let n = 0; n < PW.ModelNodes.rowCount(); n++) {
                        if (PW.ModelNodes.getNodeName(n) === multibandCompressorPage.pluginDB.sidechainInputDevice)
                            return n;
                    }
                    return 0;
                }
                onActivated: idx => {
                    let selectedName = PW.ModelNodes.getNodeName(idx);
                    if (selectedName !== multibandCompressorPage.pluginDB.sidechainInputDevice)
                        multibandCompressorPage.pluginDB.sidechainInputDevice = selectedName;
                }
            }

            EeSpinBox {
                id: dry

                label: i18n("Dry") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: multibandCompressorPage.pluginDB.getMinValue("dry")
                to: multibandCompressorPage.pluginDB.getMaxValue("dry")
                value: multibandCompressorPage.pluginDB.dry
                decimals: 2 // Required to show "-inf"
                stepSize: 0.01
                unit: Units.dB
                minusInfinityMode: true
                onValueModified: v => {
                    multibandCompressorPage.pluginDB.dry = v;
                }
            }

            EeSpinBox {
                id: wet

                label: i18n("Wet") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: multibandCompressorPage.pluginDB.getMinValue("wet")
                to: multibandCompressorPage.pluginDB.getMaxValue("wet")
                value: multibandCompressorPage.pluginDB.wet
                decimals: 2 // Required to show "-inf"
                stepSize: 0.01
                unit: Units.dB
                minusInfinityMode: true
                onValueModified: v => {
                    multibandCompressorPage.pluginDB.wet = v;
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
                        readonly property string bandTitleTag: multibandCompressorPage.pluginDB.viewSidechain ? " - " + i18n("Sidechain") : "" // qmllint disable
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
                                checked: !multibandCompressorPage.pluginDB.viewSidechain
                                icon.name: "arrow-left-symbolic"
                                onTriggered: {
                                    if (multibandCompressorPage.pluginDB.viewSidechain === false) {
                                        return;
                                    }

                                    multibandCompressorPage.pluginDB.viewSidechain = false;
                                    bandStackview.replace(bandCompressorControls);
                                }
                            },
                            Kirigami.Action {
                                id: viewRight
                                checkable: true
                                checked: multibandCompressorPage.pluginDB.viewSidechain
                                icon.name: "arrow-right-symbolic"
                                onTriggered: {
                                    if (multibandCompressorPage.pluginDB.viewSidechain === true) {
                                        return;
                                    }

                                    multibandCompressorPage.pluginDB.viewSidechain = true;
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
                    initialItem: multibandCompressorPage.pluginDB.viewSidechain === false ? bandCompressorControls : bandSidechainControls
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
                                implicitWidth: multibandCompressorPage.radius
                                implicitHeight: multibandCompressorPage.radius
                                from: Common.minimumDecibelLevel
                                to: 72
                                decimals: 0
                                topToBottom: true
                            }

                            EeAudioLevel {
                                id: reductionLevelRight

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandCompressorPage.radius
                                implicitHeight: multibandCompressorPage.radius
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
                                implicitWidth: multibandCompressorPage.radius
                                implicitHeight: multibandCompressorPage.radius
                                from: Common.minimumDecibelLevel
                                to: 36
                                decimals: 0
                            }

                            EeAudioLevel {
                                id: envelopeLevelRight

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandCompressorPage.radius
                                implicitHeight: multibandCompressorPage.radius
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
                                implicitWidth: multibandCompressorPage.radius
                                implicitHeight: multibandCompressorPage.radius
                                from: Common.minimumDecibelLevel
                                to: 36
                                decimals: 0
                            }

                            EeAudioLevel {
                                id: curveLevelRight

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandCompressorPage.radius
                                implicitHeight: multibandCompressorPage.radius
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

                    Layout.fillHeight: false
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
                                checked: listItemDelegate.index > 0 ? multibandCompressorPage.pluginDB[bandName] : true
                                onCheckedChanged: {
                                    if (checked != multibandCompressorPage.pluginDB[bandName]) {
                                        multibandCompressorPage.pluginDB[bandName] = checked;
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

        pluginDB: multibandCompressorPage.pluginDB
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: multibandCompressorPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.lsp}</b>`) // qmllint disable
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
                    checked: multibandCompressorPage.pluginBackend ? multibandCompressorPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            multibandCompressorPage.pluginBackend.showNativeUi();
                        else
                            multibandCompressorPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Stereo split") // qmllint disable
                    icon.name: "view-split-left-right-symbolic"
                    checkable: true
                    checked: multibandCompressorPage.pluginDB.stereoSplit
                    onTriggered: {
                        if (multibandCompressorPage.pluginDB.stereoSplit != checked)
                            multibandCompressorPage.pluginDB.stereoSplit = checked;
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
                        multibandCompressorPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
