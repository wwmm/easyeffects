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
    id: filterPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(filterPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(filterPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(filterPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(filterPage.pluginBackend.getOutputLevelRight());
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
                title: i18n("Controls") // qmllint disable

                FormCard.FormComboBoxDelegate {
                    id: type

                    verticalPadding: Kirigami.Units.smallSpacing
                    text: i18n("Type") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: filterPage.pluginDB.type
                    editable: false
                    model: [i18n("Low-pass"), i18n("High-pass"), i18n("Low-shelf"), i18n("High-shelf"), i18n("Bell"), i18n("Bandpass"), i18n("Notch"), i18n("Resonance"), i18n("Ladder-pass"), i18n("Ladder-rejection"), i18n("Allpass")] // qmllint disable
                    onActivated: idx => {
                        filterPage.pluginDB.type = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: mode

                    verticalPadding: Kirigami.Units.smallSpacing
                    text: i18n("Filter mode") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: filterPage.pluginDB.mode
                    editable: false
                    model: [i18n("RLC (BT)"), i18n("RLC (MT)"), i18n("BWC (BT)"), i18n("BWC (MT)"), i18n("LRX (BT)"), i18n("LRX (MT)"), i18n("APO (DR)")]
                    onActivated: idx => {
                        filterPage.pluginDB.mode = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: equalMode

                    verticalPadding: Kirigami.Units.smallSpacing
                    text: i18n("Equalizer mode") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: filterPage.pluginDB.equalMode
                    editable: false
                    model: [i18n("IIR"), i18n("FIR"), i18n("FFT"), i18n("SPM")]
                    onActivated: idx => {
                        filterPage.pluginDB.equalMode = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: slope

                    verticalPadding: Kirigami.Units.smallSpacing
                    text: i18n("Slope") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: filterPage.pluginDB.slope
                    editable: false
                    model: ["x1", "x2", "x3", "x4", "x6", "x8", "x12", "x16"]
                    onActivated: idx => {
                        filterPage.pluginDB.slope = idx;
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            EeCard {
                title: i18n("Filter") // qmllint disable

                EeSpinBox {
                    id: frequency

                    label: i18n("Frequency") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: filterPage.pluginDB.getMinValue("frequency")
                    to: filterPage.pluginDB.getMaxValue("frequency")
                    value: filterPage.pluginDB.frequency
                    decimals: 0
                    stepSize: 1
                    unit: Units.hz
                    onValueModified: v => {
                        filterPage.pluginDB.frequency = v;
                    }
                }

                EeSpinBox {
                    id: width

                    label: i18n("Width") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: filterPage.pluginDB.getMinValue("width")
                    to: filterPage.pluginDB.getMaxValue("width")
                    value: filterPage.pluginDB.width
                    decimals: 0
                    stepSize: 1
                    onValueModified: v => {
                        filterPage.pluginDB.width = v;
                    }
                }

                EeSpinBox {
                    id: gain

                    label: i18n("Gain") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: filterPage.pluginDB.getMinValue("gain")
                    to: filterPage.pluginDB.getMaxValue("gain")
                    value: filterPage.pluginDB.gain
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.dB
                    onValueModified: v => {
                        filterPage.pluginDB.gain = v;
                    }
                }

                EeSpinBox {
                    id: quality

                    label: i18n("Quality") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: filterPage.pluginDB.getMinValue("quality")
                    to: filterPage.pluginDB.getMaxValue("quality")
                    value: filterPage.pluginDB.quality
                    decimals: 2
                    stepSize: 0.01
                    onValueModified: v => {
                        filterPage.pluginDB.quality = v;
                    }
                }

                EeSpinBox {
                    id: balance

                    label: i18n("Balance") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: filterPage.pluginDB.getMinValue("balance")
                    to: filterPage.pluginDB.getMaxValue("balance")
                    value: filterPage.pluginDB.balance
                    decimals: 1
                    stepSize: 0.1
                    unit: Units.percent
                    onValueModified: v => {
                        filterPage.pluginDB.balance = v;
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: filterPage.pluginDB
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
                    checked: filterPage.pluginBackend ? filterPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            filterPage.pluginBackend.showNativeUi();
                        else
                            filterPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        filterPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
