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
    id: maximizerPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!maximizerPage.pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(maximizerPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(maximizerPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(maximizerPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(maximizerPage.pluginBackend.getOutputLevelRight());
        reductionLevel.setValue(maximizerPage.pluginBackend.getReductionLevel());
    }

    Component.onCompleted: {
        maximizerPage.pluginBackend = maximizerPage.pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Controls") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: release

                        label: i18n("Release") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: maximizerPage.pluginDB.getMinValue("release")
                        to: maximizerPage.pluginDB.getMaxValue("release")
                        value: maximizerPage.pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            maximizerPage.pluginDB.release = v;
                        }
                    }

                    EeSpinBox {
                        id: threshold

                        label: i18n("Threshold") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: maximizerPage.pluginDB.getMinValue("threshold")
                        to: maximizerPage.pluginDB.getMaxValue("threshold")
                        value: maximizerPage.pluginDB.threshold
                        decimals: 2
                        stepSize: 0.1
                        unit: Units.dB
                        onValueModified: v => {
                            maximizerPage.pluginDB.threshold = v;
                        }
                    }

                    EeProgressBar {
                        id: reductionLevel
                        Layout.topMargin: Kirigami.Units.largeSpacing

                        label: i18n("Reduction") // qmllint disable
                        unit: Units.dB
                        from: 0
                        to: 40
                        decimals: 0
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: maximizerPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.zam}</b>`) // qmllint disable
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
                    checked: maximizerPage.pluginBackend ? maximizerPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            maximizerPage.pluginBackend.showNativeUi();
                        else
                            maximizerPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        maximizerPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
