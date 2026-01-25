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

Kirigami.ScrollablePage {
    id: voiceSuppressorPage

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

            Layout.fillWidth: true

            EeCard {
                id: cardControls

                title: i18n("Controls") // qmllint disable

                EeSpinBox {
                    id: freqStart

                    label: i18n("Start") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: voiceSuppressorPage.pluginDB.getMinValue("freqStart")
                    to: voiceSuppressorPage.pluginDB.getMaxValue("freqStart")
                    value: voiceSuppressorPage.pluginDB.freqStart
                    decimals: 0
                    stepSize: 1
                    unit: Units.hz
                    onValueModified: v => {
                        voiceSuppressorPage.pluginDB.freqStart = v;
                    }
                }

                EeSpinBox {
                    id: freqEnd

                    label: i18n("End") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: voiceSuppressorPage.pluginDB.getMinValue("freqEnd")
                    to: voiceSuppressorPage.pluginDB.getMaxValue("freqEnd")
                    value: voiceSuppressorPage.pluginDB.freqEnd
                    decimals: 0
                    stepSize: 1
                    unit: Units.hz
                    onValueModified: v => {
                        voiceSuppressorPage.pluginDB.freqEnd = v;
                    }
                }

                EeSpinBox {
                    id: correlation

                    label: i18n("Correlation") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: voiceSuppressorPage.pluginDB.getMinValue("correlation")
                    to: voiceSuppressorPage.pluginDB.getMaxValue("correlation")
                    value: voiceSuppressorPage.pluginDB.correlation
                    decimals: 0
                    stepSize: 1
                    unit: Units.percent
                    onValueModified: v => {
                        voiceSuppressorPage.pluginDB.correlation = v;
                    }
                }

                EeSpinBox {
                    id: phaseDifference

                    label: i18n("Phase difference") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: voiceSuppressorPage.pluginDB.getMinValue("phaseDifference")
                    to: voiceSuppressorPage.pluginDB.getMaxValue("phaseDifference")
                    value: voiceSuppressorPage.pluginDB.phaseDifference
                    decimals: 0
                    stepSize: 1
                    unit: Units.degrees
                    onValueModified: v => {
                        voiceSuppressorPage.pluginDB.phaseDifference = v;
                    }
                }

                EeSpinBox {
                    id: minKurtosis

                    label: i18n("Minimum kurtosis") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: voiceSuppressorPage.pluginDB.getMinValue("minKurtosis")
                    to: voiceSuppressorPage.pluginDB.getMaxValue("minKurtosis")
                    value: voiceSuppressorPage.pluginDB.minKurtosis
                    decimals: 2
                    stepSize: 0.01
                    onValueModified: v => {
                        voiceSuppressorPage.pluginDB.minKurtosis = v;
                    }
                }

                EeSpinBox {
                    id: maxInstFreq

                    label: i18n("Maximum instantaneous frequency") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: voiceSuppressorPage.pluginDB.getMinValue("maxInstFreq")
                    to: voiceSuppressorPage.pluginDB.getMaxValue("maxInstFreq")
                    value: voiceSuppressorPage.pluginDB.maxInstFreq
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.hz
                    onValueModified: v => {
                        voiceSuppressorPage.pluginDB.maxInstFreq = v;
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: voiceSuppressorPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.bs2b}</strong>`) // qmllint disable
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
                    text: i18n("Inverted mode") // qmllint disable
                    icon.name: "image-invert-symbolic"
                    checkable: true
                    checked: voiceSuppressorPage.pluginDB.invertedMode
                    onTriggered: {
                        if (voiceSuppressorPage.pluginDB.invertedMode != checked)
                            voiceSuppressorPage.pluginDB.invertedMode = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        voiceSuppressorPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
