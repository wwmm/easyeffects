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
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: delayPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(pluginBackend.getOutputLevelRight());
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            implicitWidth: cardLayout.maximumColumnWidth
            uniformCellWidths: true

            EeCard {
                title: i18n("Left") // qmllint disable

                GridLayout {
                    columns: 2
                    rowSpacing: 0
                    uniformCellWidths: true

                    FormCard.FormComboBoxDelegate {
                        id: modeL

                        Layout.columnSpan: 2
                        verticalPadding: Kirigami.Units.smallSpacing
                        text: i18n("Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: delayPage.pluginDB.modeL
                        editable: false
                        model: [i18n("Samples"), i18n("Distance"), i18n("Time")] // qmllint disable
                        onActivated: idx => {
                            delayPage.pluginDB.modeL = idx;
                        }
                    }

                    EeSpinBox {
                        id: sampleL

                        Layout.columnSpan: 2
                        visible: delayPage.pluginDB.modeL === 0 ? true : false
                        label: i18n("Samples") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("sampleL")
                        to: delayPage.pluginDB.getMaxValue("sampleL")
                        value: delayPage.pluginDB.sampleL
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            delayPage.pluginDB.sampleL = v;
                        }
                    }

                    EeSpinBox {
                        id: timeL

                        Layout.columnSpan: 2
                        visible: delayPage.pluginDB.modeL === 2 ? true : false
                        label: i18n("Time") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("timeL")
                        to: delayPage.pluginDB.getMaxValue("timeL")
                        value: delayPage.pluginDB.timeL
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            delayPage.pluginDB.timeL = v;
                        }
                    }

                    EeSpinBox {
                        id: metersL

                        visible: delayPage.pluginDB.modeL === 1 ? true : false
                        label: i18n("Meters") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("metersL")
                        to: delayPage.pluginDB.getMaxValue("metersL")
                        value: delayPage.pluginDB.metersL
                        decimals: 0
                        stepSize: 1
                        unit: Units.m
                        onValueModified: v => {
                            delayPage.pluginDB.metersL = v;
                        }
                    }

                    EeSpinBox {
                        id: centimetersL

                        visible: delayPage.pluginDB.modeL === 1 ? true : false
                        label: i18n("Centimeters") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("centimetersL")
                        to: delayPage.pluginDB.getMaxValue("centimetersL")
                        value: delayPage.pluginDB.centimetersL
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.cm
                        onValueModified: v => {
                            delayPage.pluginDB.centimetersL = v;
                        }
                    }

                    EeSpinBox {
                        id: temperatureL

                        Layout.columnSpan: 2
                        visible: delayPage.pluginDB.modeL === 1 ? true : false
                        label: i18n("Temperature") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("temperatureL")
                        to: delayPage.pluginDB.getMaxValue("temperatureL")
                        value: delayPage.pluginDB.temperatureL
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.degC
                        onValueModified: v => {
                            delayPage.pluginDB.temperatureL = v;
                        }
                    }

                    EeSpinBox {
                        id: dryL

                        label: i18n("Dry") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("dryL")
                        to: delayPage.pluginDB.getMaxValue("dryL")
                        value: delayPage.pluginDB.dryL
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            delayPage.pluginDB.dryL = v;
                        }
                    }

                    EeSpinBox {
                        id: wetL

                        label: i18n("Wet") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("wetL")
                        to: delayPage.pluginDB.getMaxValue("wetL")
                        value: delayPage.pluginDB.wetL
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            delayPage.pluginDB.wetL = v;
                        }
                    }
                }
            }

            EeCard {
                title: i18n("Right") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    Layout.alignment: Qt.AlignTop

                    FormCard.FormComboBoxDelegate {
                        id: modeR

                        Layout.columnSpan: 2
                        verticalPadding: 0
                        text: i18n("Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: delayPage.pluginDB.modeR
                        editable: false
                        model: [i18n("Samples"), i18n("Distance"), i18n("Time")] // qmllint disable
                        onActivated: idx => {
                            delayPage.pluginDB.modeR = idx;
                        }
                    }

                    EeSpinBox {
                        id: sampleR

                        Layout.columnSpan: 2
                        visible: delayPage.pluginDB.modeR === 0 ? true : false
                        label: i18n("Samples") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("sampleR")
                        to: delayPage.pluginDB.getMaxValue("sampleR")
                        value: delayPage.pluginDB.sampleR
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            delayPage.pluginDB.sampleR = v;
                        }
                    }

                    EeSpinBox {
                        id: timeR

                        Layout.columnSpan: 2
                        visible: delayPage.pluginDB.modeR === 2 ? true : false
                        label: i18n("Time") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("timeR")
                        to: delayPage.pluginDB.getMaxValue("timeR")
                        value: delayPage.pluginDB.timeR
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            delayPage.pluginDB.timeR = v;
                        }
                    }

                    EeSpinBox {
                        id: metersR

                        visible: delayPage.pluginDB.modeR === 1 ? true : false
                        label: i18n("Meters") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("metersR")
                        to: delayPage.pluginDB.getMaxValue("metersR")
                        value: delayPage.pluginDB.metersR
                        decimals: 0
                        stepSize: 1
                        unit: Units.m
                        onValueModified: v => {
                            delayPage.pluginDB.metersR = v;
                        }
                    }

                    EeSpinBox {
                        id: centimetersR

                        visible: delayPage.pluginDB.modeR === 1 ? true : false
                        label: i18n("Centimeters") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("centimetersR")
                        to: delayPage.pluginDB.getMaxValue("centimetersR")
                        value: delayPage.pluginDB.centimetersR
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.cm
                        onValueModified: v => {
                            delayPage.pluginDB.centimetersR = v;
                        }
                    }

                    EeSpinBox {
                        id: temperatureR

                        Layout.columnSpan: 2
                        visible: delayPage.pluginDB.modeR === 1 ? true : false
                        label: i18n("Temperature") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("temperatureR")
                        to: delayPage.pluginDB.getMaxValue("temperatureR")
                        value: delayPage.pluginDB.temperatureR
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.degC
                        onValueModified: v => {
                            delayPage.pluginDB.temperatureR = v;
                        }
                    }

                    EeSpinBox {
                        id: dryR

                        label: i18n("Dry") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("dryR")
                        to: delayPage.pluginDB.getMaxValue("dryR")
                        value: delayPage.pluginDB.dryR
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            delayPage.pluginDB.dryR = v;
                        }
                    }

                    EeSpinBox {
                        id: wetR

                        label: i18n("Wet") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: delayPage.pluginDB.getMinValue("wetR")
                        to: delayPage.pluginDB.getMaxValue("wetR")
                        value: delayPage.pluginDB.wetR
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            delayPage.pluginDB.wetR = v;
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: delayPage.pluginDB
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
                    text: i18n("Invert left") // qmllint disable
                    icon.name: "edit-select-invert-symbolic"
                    checkable: true
                    checked: delayPage.pluginDB.invertPhaseL
                    onTriggered: {
                        if (checked !== delayPage.pluginDB.invertPhaseL)
                            delayPage.pluginDB.invertPhaseL = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Invert right") // qmllint disable
                    icon.name: "edit-select-invert-symbolic"
                    checkable: true
                    checked: delayPage.pluginDB.invertPhaseR
                    onTriggered: {
                        if (checked !== delayPage.pluginDB.invertPhaseR)
                            delayPage.pluginDB.invertPhaseR = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Show native window") // qmllint disable
                    icon.name: "window-duplicate-symbolic"
                    enabled: DbMain.showNativePluginUi
                    checkable: true
                    checked: delayPage.pluginBackend ? delayPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            delayPage.pluginBackend.showNativeUi();
                        else
                            delayPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        delayPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
