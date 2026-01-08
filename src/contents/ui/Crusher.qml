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
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: crusherPage

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
        Layout.fillWidth: true
        Layout.fillHeight: true

        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Controls") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    FormCard.FormComboBoxDelegate {
                        id: mode

                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: false
                        text: i18n("Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: crusherPage.pluginDB.mode
                        editable: false
                        model: [i18n("Linear"), i18n("Logarithmic")] // qmllint disable
                        onActivated: idx => {
                            crusherPage.pluginDB.mode = idx;
                        }
                    }

                    EeSpinBox {
                        id: bitReduction

                        label: i18n("Bit Reduction") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: crusherPage.pluginDB.getMinValue("bitReduction")
                        to: crusherPage.pluginDB.getMaxValue("bitReduction")
                        value: crusherPage.pluginDB.bitReduction
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            crusherPage.pluginDB.bitReduction = v;
                        }
                    }

                    EeSpinBox {
                        id: dc

                        label: i18n("DC offset") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: crusherPage.pluginDB.getMinValue("dc")
                        to: crusherPage.pluginDB.getMaxValue("dc")
                        value: crusherPage.pluginDB.dc
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.dB
                        onValueModified: v => {
                            crusherPage.pluginDB.dc = v;
                        }
                    }

                    EeSpinBox {
                        id: antiAliasing

                        label: i18n("Anti-aliasing") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: crusherPage.pluginDB.getMinValue("antiAliasing")
                        to: crusherPage.pluginDB.getMaxValue("antiAliasing") * 100
                        value: crusherPage.pluginDB.antiAliasing * 100
                        decimals: 0
                        stepSize: 1
                        unit: Units.percent
                        onValueModified: v => {
                            crusherPage.pluginDB.antiAliasing = v * 0.01;
                        }
                    }

                    EeSpinBox {
                        id: morph

                        label: i18n("Mix") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: crusherPage.pluginDB.getMinValue("morph")
                        to: crusherPage.pluginDB.getMaxValue("morph") * 100
                        value: crusherPage.pluginDB.morph * 100
                        decimals: 0
                        stepSize: 1
                        unit: Units.percent
                        onValueModified: v => {
                            crusherPage.pluginDB.morph = v * 0.01;
                        }
                    }

                    EeSpinBox {
                        id: sampleReduction

                        label: i18n("Sample Reduction") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: crusherPage.pluginDB.getMinValue("sampleReduction")
                        to: crusherPage.pluginDB.getMaxValue("sampleReduction")
                        value: crusherPage.pluginDB.sampleReduction
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            crusherPage.pluginDB.sampleReduction = v;
                        }
                    }

                    EeSwitch {
                        id: lfoActive

                        label: i18n("Low frequency oscillator") // qmllint disable
                        isChecked: crusherPage.pluginDB.lfoActive
                        onCheckedChanged: {
                            if (isChecked !== crusherPage.pluginDB.lfoActive)
                                crusherPage.pluginDB.lfoActive = isChecked;
                        }
                    }

                    // EeSpinBox {
                    //     id: ceil

                    //     label: i18n("Ceil") // qmllint disable
                    //     spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    //     from: crusherPage.pluginDB.getMinValue("ceil")
                    //     to: crusherPage.pluginDB.getMaxValue("ceil")
                    //     value: crusherPage.pluginDB.ceil
                    //     decimals: 0
                    //     stepSize: 1
                    //     unit: Units.hz
                    //     enabled: ceilActive.isChecked
                    //     onValueModified: v => {
                    //         crusherPage.pluginDB.ceil = v;
                    //     }
                    // }

                    // EeProgressBar {
                    //     id: harmonicsLevel
                    //     Layout.topMargin: Kirigami.Units.largeSpacing

                    //     label: i18n("Harmonics") // qmllint disable
                    //     from: Common.minimumDecibelLevel
                    //     to: 10
                    //     decimals: 0
                    // }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: crusherPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.calf}</strong>`) // qmllint disable
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
                    checked: crusherPage.pluginBackend ? crusherPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            crusherPage.pluginBackend.showNativeUi();
                        else
                            crusherPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        crusherPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
