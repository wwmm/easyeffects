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
import ee.ui
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: autotunePage

    required property string name
    required property DbAutotune pluginDB
    required property EffectsBase pipelineInstance
    property BackendAutotune pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(pluginBackend.getOutputLevelRight());
        pitchErrorMeter.setValue(pluginBackend.getPitchError());
        pitchBendMeter.setValue(pluginBackend.getPitchBend());
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true
            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            maximumColumns: 6
            uniformCellWidths: true

            EeCard {
                title: i18n("Tuning") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0

                    FormCard.FormComboBoxDelegate {
                        id: mode

                        Layout.columnSpan: 2
                        verticalPadding: Kirigami.Units.largeSpacing
                        text: i18n("Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: autotunePage.pluginDB.mode
                        editable: false
                        model: [i18n("Auto"), i18n("MIDI"), i18n("Manual")] //qmllint disable
                        onActivated: idx => {
                            autotunePage.pluginDB.mode = idx;
                        }
                    }

                    EeSpinBox {
                        id: tuning

                        label: i18n("Tuning") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: autotunePage.pluginDB.getMinValue("tuning")
                        to: autotunePage.pluginDB.getMaxValue("tuning")
                        value: autotunePage.pluginDB.tuning
                        decimals: 1
                        stepSize: 0.5
                        unit: Units.hz
                        onValueModified: v => {
                            autotunePage.pluginDB.tuning = v;
                        }
                    }

                    EeSpinBox {
                        id: correction

                        label: i18n("Correction") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: autotunePage.pluginDB.getMinValue("correction")
                        to: autotunePage.pluginDB.getMaxValue("correction")
                        value: autotunePage.pluginDB.correction
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            autotunePage.pluginDB.correction = v;
                        }
                    }

                    EeSpinBox {
                        id: bias

                        label: i18n("Bias") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: autotunePage.pluginDB.getMinValue("bias")
                        to: autotunePage.pluginDB.getMaxValue("bias")
                        value: autotunePage.pluginDB.bias
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            autotunePage.pluginDB.bias = v;
                        }
                    }

                    EeSpinBox {
                        id: filterControl

                        label: i18n("Filter") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: autotunePage.pluginDB.getMinValue("filter")
                        to: autotunePage.pluginDB.getMaxValue("filter")
                        value: autotunePage.pluginDB.filter
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            autotunePage.pluginDB.filter = v;
                        }
                    }

                    EeSpinBox {
                        id: offset

                        label: i18n("Offset") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: autotunePage.pluginDB.getMinValue("offset")
                        to: autotunePage.pluginDB.getMaxValue("offset")
                        value: autotunePage.pluginDB.offset
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            autotunePage.pluginDB.offset = v;
                        }
                    }

                    EeSpinBox {
                        id: bendRange

                        label: i18n("Pitch bend range") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: autotunePage.pluginDB.getMinValue("bendRange")
                        to: autotunePage.pluginDB.getMaxValue("bendRange")
                        value: autotunePage.pluginDB.bendRange
                        decimals: 1
                        stepSize: 0.5
                        onValueModified: v => {
                            autotunePage.pluginDB.bendRange = v;
                        }
                    }
                }
            }

            EeCard {
                title: i18n("Notes") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0

                    EeSwitch {
                        label: "C"
                        isChecked: autotunePage.pluginDB.noteC
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteC)
                                autotunePage.pluginDB.noteC = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "C#"
                        isChecked: autotunePage.pluginDB.noteCSharp
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteCSharp)
                                autotunePage.pluginDB.noteCSharp = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "D"
                        isChecked: autotunePage.pluginDB.noteD
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteD)
                                autotunePage.pluginDB.noteD = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "D#"
                        isChecked: autotunePage.pluginDB.noteDSharp
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteDSharp)
                                autotunePage.pluginDB.noteDSharp = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "E"
                        isChecked: autotunePage.pluginDB.noteE
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteE)
                                autotunePage.pluginDB.noteE = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "F"
                        isChecked: autotunePage.pluginDB.noteF
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteF)
                                autotunePage.pluginDB.noteF = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "F#"
                        isChecked: autotunePage.pluginDB.noteFSharp
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteFSharp)
                                autotunePage.pluginDB.noteFSharp = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "G"
                        isChecked: autotunePage.pluginDB.noteG
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteG)
                                autotunePage.pluginDB.noteG = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "G#"
                        isChecked: autotunePage.pluginDB.noteGSharp
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteGSharp)
                                autotunePage.pluginDB.noteGSharp = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "A"
                        isChecked: autotunePage.pluginDB.noteA
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteA)
                                autotunePage.pluginDB.noteA = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "A#"
                        isChecked: autotunePage.pluginDB.noteASharp
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteASharp)
                                autotunePage.pluginDB.noteASharp = isChecked;
                        }
                    }

                    EeSwitch {
                        label: "B"
                        isChecked: autotunePage.pluginDB.noteB
                        onCheckedChanged: {
                            if (isChecked !== autotunePage.pluginDB.noteB)
                                autotunePage.pluginDB.noteB = isChecked;
                        }
                    }
                }
            }

            EeCard {
                title: i18n("Meters") // qmllint disable

                ColumnLayout {
                    EeProgressBar {
                        id: pitchErrorMeter

                        label: i18n("Pitch error") // qmllint disable
                        from: -1
                        to: 1
                        decimals: 2
                    }

                    EeProgressBar {
                        id: pitchBendMeter

                        label: i18n("Pitch bend") // qmllint disable
                        from: -1
                        to: 1
                        decimals: 2
                    }
                }
            }
        }
    }

    EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: autotunePage.pluginDB
    }

    header: inputOutputLevels

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${PluginsPackage.x42}</strong>`) // qmllint disable
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
                    text: i18n("Fast correction") // qmllint disable
                    icon.name: "player-time-symbolic"
                    checkable: true
                    checked: autotunePage.pluginDB.fastMode
                    onTriggered: {
                        if (autotunePage.pluginDB.fastMode !== checked)
                            autotunePage.pluginDB.fastMode = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Show native window") // qmllint disable
                    icon.name: "window-duplicate-symbolic"
                    enabled: DbMain.showNativePluginUi
                    checkable: true
                    checked: autotunePage.pluginBackend ? autotunePage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            autotunePage.pluginBackend.showNativeUi();
                        else
                            autotunePage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset settings") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        autotunePage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
