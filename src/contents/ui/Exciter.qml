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

Kirigami.ScrollablePage {
    id: exciterPage

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
        harmonicsLevel.setValue(exciterPage.pluginBackend.getHarmonicsLevel());
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
                value: exciterPage.pluginDB.blend
                from: exciterPage.pluginDB.getMinValue("blend")
                to: exciterPage.pluginDB.getMaxValue("blend")
                stepSize: 1
                onValueChanged: () => {
                    if (value !== exciterPage.pluginDB.blend)
                        exciterPage.pluginDB.blend = value;
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

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Controls") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: amount

                        label: i18n("Amount") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: exciterPage.pluginDB.getMinValue("amount")
                        to: exciterPage.pluginDB.getMaxValue("amount")
                        value: exciterPage.pluginDB.amount
                        decimals: 2
                        stepSize: 0.1
                        unit: Units.dB
                        onValueModified: v => {
                            exciterPage.pluginDB.amount = v;
                        }
                    }

                    EeSpinBox {
                        id: harmonics

                        label: i18n("Harmonics") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: exciterPage.pluginDB.getMinValue("harmonics")
                        to: exciterPage.pluginDB.getMaxValue("harmonics")
                        value: exciterPage.pluginDB.harmonics
                        decimals: 1
                        stepSize: 0.1
                        onValueModified: v => {
                            exciterPage.pluginDB.harmonics = v;
                        }
                    }

                    EeSpinBox {
                        id: scope

                        label: i18n("Scope") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: exciterPage.pluginDB.getMinValue("scope")
                        to: exciterPage.pluginDB.getMaxValue("scope")
                        value: exciterPage.pluginDB.scope
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        onValueModified: v => {
                            exciterPage.pluginDB.scope = v;
                        }
                    }

                    EeSwitch {
                        id: ceilActive

                        label: i18n("Ceil active") // qmllint disable
                        isChecked: exciterPage.pluginDB.ceilActive
                        onCheckedChanged: {
                            if (isChecked !== exciterPage.pluginDB.ceilActive)
                                exciterPage.pluginDB.ceilActive = isChecked;
                        }
                    }

                    EeSpinBox {
                        id: ceil

                        label: i18n("Ceil") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: exciterPage.pluginDB.getMinValue("ceil")
                        to: exciterPage.pluginDB.getMaxValue("ceil")
                        value: exciterPage.pluginDB.ceil
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        enabled: ceilActive.isChecked
                        onValueModified: v => {
                            exciterPage.pluginDB.ceil = v;
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
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: exciterPage.pluginDB
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
                    checked: exciterPage.pluginBackend ? exciterPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            exciterPage.pluginBackend.showNativeUi();
                        else
                            exciterPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Listen") // qmllint disable
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: exciterPage.pluginDB.listen
                    onTriggered: {
                        if (exciterPage.pluginDB.listen != checked)
                            exciterPage.pluginDB.listen = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        exciterPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
