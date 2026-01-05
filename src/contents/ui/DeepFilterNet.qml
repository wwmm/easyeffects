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
    id: deepfilternetPage

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
        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Attenuation limit") // qmllint disable
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 2 * cardLayout.maximumColumnWidth

            Controls.Slider {
                id: attenuationLimit

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                orientation: Qt.Horizontal
                snapMode: Controls.Slider.SnapAlways
                from: deepfilternetPage.pluginDB.getMinValue("attenuationLimit")
                to: deepfilternetPage.pluginDB.getMaxValue("attenuationLimit")
                value: deepfilternetPage.pluginDB.attenuationLimit
                stepSize: 1
                onValueChanged: () => {
                    if (value !== deepfilternetPage.pluginDB.attenuationLimit)
                        deepfilternetPage.pluginDB.attenuationLimit = value;
                }
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: attenuationLimit.value
        }

        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true

            Kirigami.Card {
                id: cardControls

                leftPadding: 0
                rightPadding: 0

                header: Kirigami.Heading {
                    text: i18n("Controls") // qmllint disable
                    level: 2
                    leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                    rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                }

                contentItem: ColumnLayout {
                    spacing: 0

                    EeSpinBox {
                        id: minProcessingThreshold

                        label: i18n("Minimum processing threshold") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: deepfilternetPage.pluginDB.getMinValue("minProcessingThreshold")
                        to: deepfilternetPage.pluginDB.getMaxValue("minProcessingThreshold")
                        value: deepfilternetPage.pluginDB.minProcessingThreshold
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.dB
                        onValueModified: v => {
                            deepfilternetPage.pluginDB.minProcessingThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: maxErbProcessingThreshold

                        label: i18n("Maximum ERB processing threshold") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: deepfilternetPage.pluginDB.getMinValue("maxErbProcessingThreshold")
                        to: deepfilternetPage.pluginDB.getMaxValue("maxErbProcessingThreshold")
                        value: deepfilternetPage.pluginDB.maxErbProcessingThreshold
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.dB
                        onValueModified: v => {
                            deepfilternetPage.pluginDB.maxErbProcessingThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: maxDfProcessingThreshold

                        label: i18n("Maximum DF processing threshold") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: deepfilternetPage.pluginDB.getMinValue("maxDfProcessingThreshold")
                        to: deepfilternetPage.pluginDB.getMaxValue("maxDfProcessingThreshold")
                        value: deepfilternetPage.pluginDB.maxDfProcessingThreshold
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.dB
                        onValueModified: v => {
                            deepfilternetPage.pluginDB.maxDfProcessingThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: minProcessingBuffer

                        label: i18n("Minimum processing buffer") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: deepfilternetPage.pluginDB.getMinValue("minProcessingBuffer")
                        to: deepfilternetPage.pluginDB.getMaxValue("minProcessingBuffer")
                        value: deepfilternetPage.pluginDB.minProcessingBuffer
                        decimals: 0
                        stepSize: 1
                        unit: Units.frames
                        onValueModified: v => {
                            deepfilternetPage.pluginDB.minProcessingBuffer = v;
                        }
                    }

                    EeSpinBox {
                        id: postFilterBeta

                        label: i18n("Post filter Beta") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: deepfilternetPage.pluginDB.getMinValue("postFilterBeta")
                        to: deepfilternetPage.pluginDB.getMaxValue("postFilterBeta")
                        value: deepfilternetPage.pluginDB.postFilterBeta
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        onValueModified: v => {
                            deepfilternetPage.pluginDB.postFilterBeta = v;
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: deepfilternetPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.deepfilternet}</strong>`) // qmllint disable
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
                        deepfilternetPage.pluginBackend.resetHistory();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset settings") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        deepfilternetPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
