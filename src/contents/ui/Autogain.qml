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
    id: autogainPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        momentary.setValue(pluginBackend.getMomentaryLevel());
        shortterm.setValue(pluginBackend.getShorttermLevel());
        integrated.setValue(pluginBackend.getIntegratedLevel());
        relative.setValue(pluginBackend.getRelativeLevel());
        range.setValue(pluginBackend.getRangeLevel());
        loudness.setValue(pluginBackend.getLoudnessLevel());
        outputGain.setValue(pluginBackend.getOutputGainLevel());
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
            uniformCellWidths: true

            EeCard {
                id: cardControls

                implicitWidth: cardLayout.maximumColumnWidth

                title: i18n("Controls") // qmllint disable

                FormCard.FormComboBoxDelegate {
                    id: reference

                    text: i18n("Reference") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    verticalPadding: 0
                    currentIndex: autogainPage.pluginDB.reference
                    editable: false
                    model: [i18n("Momentary"), i18n("Short-term"), i18n("Integrated"), i18n("Geometric mean (MSI)"), i18n("Geometric mean (MS)"), i18n("Geometric mean (MI)"), i18n("Geometric mean (SI)")] // qmllint disable
                    onActivated: idx => {
                        autogainPage.pluginDB.reference = idx;
                    }
                }

                EeSpinBox {
                    id: target

                    label: i18n("Target") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: autogainPage.pluginDB.getMinValue("target")
                    to: autogainPage.pluginDB.getMaxValue("target")
                    value: autogainPage.pluginDB.target
                    decimals: 2
                    stepSize: 0.1
                    unit: Units.dB
                    onValueModified: v => {
                        autogainPage.pluginDB.target = v;
                    }
                }

                EeSpinBox {
                    id: silenceThreshold

                    label: i18n("Silence") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: autogainPage.pluginDB.getMinValue("silenceThreshold")
                    to: autogainPage.pluginDB.getMaxValue("silenceThreshold")
                    value: autogainPage.pluginDB.silenceThreshold
                    decimals: 2
                    stepSize: 0.1
                    unit: Units.dB
                    onValueModified: v => {
                        autogainPage.pluginDB.silenceThreshold = v;
                    }
                }

                EeSpinBox {
                    id: maximumHistory

                    label: i18n("Maximum history") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: autogainPage.pluginDB.getMinValue("maximumHistory")
                    to: autogainPage.pluginDB.getMaxValue("maximumHistory")
                    value: autogainPage.pluginDB.maximumHistory
                    decimals: 0
                    stepSize: 1
                    unit: Units.s
                    onValueModified: v => {
                        autogainPage.pluginDB.maximumHistory = v;
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            EeCard {
                id: cardLevels

                implicitWidth: cardLayout.maximumColumnWidth

                title: i18n("Loudness") // qmllint disable

                EeProgressBar {
                    id: momentary

                    label: i18n("Momentary") // qmllint disable
                    unit: Units.lufs
                    from: Common.minimumDecibelLevel
                    to: 10
                    decimals: 1
                }

                EeProgressBar {
                    id: shortterm

                    label: i18n("Short-term") // qmllint disable
                    unit: Units.lufs
                    from: Common.minimumDecibelLevel
                    to: 10
                    decimals: 1
                }

                EeProgressBar {
                    id: integrated

                    label: i18n("Integrated") // qmllint disable
                    unit: Units.lufs
                    from: Common.minimumDecibelLevel
                    to: 10
                    decimals: 1
                }

                EeProgressBar {
                    id: relative

                    label: i18n("Relative") // qmllint disable
                    unit: Units.lufs
                    from: Common.minimumDecibelLevel
                    to: 10
                    decimals: 1
                }

                EeProgressBar {
                    id: range

                    label: i18n("Range") // qmllint disable
                    unit: Units.lu
                    from: 0
                    to: 50
                    decimals: 1
                }

                EeProgressBar {
                    id: loudness

                    label: i18n("Loudness") // qmllint disable
                    unit: Units.lufs
                    from: Common.minimumDecibelLevel
                    to: 10
                    decimals: 1
                }

                EeProgressBar {
                    id: outputGain

                    label: i18n("Output gain") // qmllint disable
                    unit: Units.dB
                    from: Common.minimumDecibelLevel
                    to: 20
                    decimals: 1
                    convertDecibelToLinear: true
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: autogainPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.ebur128}</strong>`) // qmllint disable
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
                    text: i18n("Reset history") // qmllint disable
                    icon.name: "edit-clear-history-symbolic"
                    onTriggered: {
                        autogainPage.pluginBackend.resetHistory();
                    }
                },
                Kirigami.Action {
                    text: i18n("Force silence") // qmllint disable
                    tooltip: i18n("Force silence when the level is below the silence threshold") // qmllint disable
                    icon.name: "audio-volume-muted-symbolic"
                    checkable: true
                    checked: autogainPage.pluginDB.forceSilence
                    onTriggered: {
                        if (checked !== autogainPage.pluginDB.forceSilence) {
                            autogainPage.pluginDB.forceSilence = checked;
                        }
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset settings") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        autogainPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
