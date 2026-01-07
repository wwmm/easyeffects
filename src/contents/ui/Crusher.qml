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

                contentItem:
                // EeSpinBox {
                //     id: amount

                //     label: i18n("Amount") // qmllint disable
                //     spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                //     from: crusherPage.pluginDB.getMinValue("amount")
                //     to: crusherPage.pluginDB.getMaxValue("amount")
                //     value: crusherPage.pluginDB.amount
                //     decimals: 2
                //     stepSize: 0.1
                //     unit: Units.dB
                //     onValueModified: v => {
                //         crusherPage.pluginDB.amount = v;
                //     }
                // }

                // EeSpinBox {
                //     id: harmonics

                //     label: i18n("Harmonics") // qmllint disable
                //     spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                //     from: crusherPage.pluginDB.getMinValue("harmonics")
                //     to: crusherPage.pluginDB.getMaxValue("harmonics")
                //     value: crusherPage.pluginDB.harmonics
                //     decimals: 1
                //     stepSize: 0.1
                //     onValueModified: v => {
                //         crusherPage.pluginDB.harmonics = v;
                //     }
                // }

                // EeSpinBox {
                //     id: scope

                //     label: i18n("Scope") // qmllint disable
                //     spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                //     from: crusherPage.pluginDB.getMinValue("scope")
                //     to: crusherPage.pluginDB.getMaxValue("scope")
                //     value: crusherPage.pluginDB.scope
                //     decimals: 0
                //     stepSize: 1
                //     unit: Units.hz
                //     onValueModified: v => {
                //         crusherPage.pluginDB.scope = v;
                //     }
                // }

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
                ColumnLayout {}
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
