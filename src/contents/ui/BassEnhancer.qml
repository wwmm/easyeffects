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
import ee.ui
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: bassEnhancerPage

    required property string name
    required property DbBassEnhancer pluginDB
    required property var pipelineInstance
    property BassEnhancer pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(pluginBackend.getOutputLevelRight());
        harmonicsLevel.setValue(pluginBackend.getHarmonicsLevel());
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Blend harmonics") // qmllint disable
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 2 * cardLayout.maximumColumnWidth

            Controls.Label {
                Layout.alignment: Qt.AlignLeft
                text: i18n("3rd") // qmllint disable
            }

            Controls.Slider {
                id: blend

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                orientation: Qt.Horizontal
                snapMode: Controls.Slider.SnapAlways
                value: bassEnhancerPage.pluginDB.blend
                from: bassEnhancerPage.pluginDB.getMinValue("blend")
                to: bassEnhancerPage.pluginDB.getMaxValue("blend")
                stepSize: 1
                onValueChanged: () => {
                    if (value !== bassEnhancerPage.pluginDB.blend)
                        bassEnhancerPage.pluginDB.blend = value;
                }
            }

            Controls.Label {
                Layout.alignment: Qt.AlignRight
                text: i18n("2nd") // qmllint disable
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: blend.value
        }

        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            EeCard {
                id: cardControls

                title: i18n("Controls") // qmllint disable

                EeSpinBox {
                    id: amount

                    label: i18n("Amount") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: bassEnhancerPage.pluginDB.getMinValue("amount")
                    to: bassEnhancerPage.pluginDB.getMaxValue("amount")
                    value: bassEnhancerPage.pluginDB.amount
                    decimals: 2
                    stepSize: 0.1
                    unit: Units.dB
                    onValueModified: v => {
                        bassEnhancerPage.pluginDB.amount = v;
                    }
                }

                EeSpinBox {
                    id: harmonics

                    label: i18n("Harmonics") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: bassEnhancerPage.pluginDB.getMinValue("harmonics")
                    to: bassEnhancerPage.pluginDB.getMaxValue("harmonics")
                    value: bassEnhancerPage.pluginDB.harmonics
                    decimals: 1
                    stepSize: 0.1
                    onValueModified: v => {
                        bassEnhancerPage.pluginDB.harmonics = v;
                    }
                }

                EeSpinBox {
                    id: scope

                    label: i18n("Scope") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: bassEnhancerPage.pluginDB.getMinValue("scope")
                    to: bassEnhancerPage.pluginDB.getMaxValue("scope")
                    value: bassEnhancerPage.pluginDB.scope
                    decimals: 0
                    stepSize: 1
                    unit: Units.hz
                    onValueModified: v => {
                        bassEnhancerPage.pluginDB.scope = v;
                    }
                }

                EeSwitch {
                    id: floorActive

                    label: i18n("Floor active") // qmllint disable
                    isChecked: bassEnhancerPage.pluginDB.floorActive
                    onCheckedChanged: {
                        if (isChecked !== bassEnhancerPage.pluginDB.floorActive)
                            bassEnhancerPage.pluginDB.floorActive = isChecked;
                    }
                }

                EeSpinBox {
                    id: floor

                    label: i18n("Floor") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: bassEnhancerPage.pluginDB.getMinValue("floor")
                    to: bassEnhancerPage.pluginDB.getMaxValue("floor")
                    value: bassEnhancerPage.pluginDB.floor
                    decimals: 0
                    stepSize: 1
                    unit: Units.hz
                    enabled: floorActive.isChecked
                    onValueModified: v => {
                        bassEnhancerPage.pluginDB.floor = v;
                    }
                }

                EeProgressBar {
                    id: harmonicsLevel
                    Layout.topMargin: Kirigami.Units.largeSpacing

                    label: i18n("Harmonics") // qmllint disable
                    from: Common.minimumDecibelLevel
                    to: 10
                    decimals: 0
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: bassEnhancerPage.pluginDB
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
                    checked: bassEnhancerPage.pluginBackend ? bassEnhancerPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            bassEnhancerPage.pluginBackend.showNativeUi();
                        else
                            bassEnhancerPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Listen") // qmllint disable
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: bassEnhancerPage.pluginDB.listen
                    onTriggered: {
                        if (bassEnhancerPage.pluginDB.listen != checked)
                            bassEnhancerPage.pluginDB.listen = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        bassEnhancerPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
