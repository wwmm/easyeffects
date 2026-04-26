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

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import ee.ui
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: crosstalkPage

    required property string name
    required property DbCrosstalkCanceller pluginDB
    required property EffectsBase pipelineInstance
    property BackendCrosstalkCanceller pluginBackend

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

                FormCard.FormCheckDelegate {
                    id: phantom_center_only

                    text: i18n("Process only phantom center") // qmllint disable

                    checked: crosstalkPage.pluginDB.phantomCenterOnly
                    onCheckedChanged: {
                        crosstalkPage.pluginDB.phantomCenterOnly = checked;
                    }
                }

                EeSpinBox {
                    id: delay_us

                    label: i18n("Delay") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: crosstalkPage.pluginDB.getMinValue("delayUs")
                    to: crosstalkPage.pluginDB.getMaxValue("delayUs")
                    value: crosstalkPage.pluginDB.delayUs
                    decimals: 0
                    stepSize: 1
                    unit: Units.us
                    onValueModified: v => {
                        crosstalkPage.pluginDB.delayUs = v;
                    }
                }

                EeSpinBox {
                    id: decay_db

                    label: i18n("Decay") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: crosstalkPage.pluginDB.getMinValue("decayDb")
                    to: crosstalkPage.pluginDB.getMaxValue("decayDb")
                    value: crosstalkPage.pluginDB.decayDb
                    decimals: 1
                    stepSize: 0.1
                    unit: Units.dB
                    onValueModified: v => {
                        crosstalkPage.pluginDB.decayDb = v;
                    }
                }
            }
        }
    }

    EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: crosstalkPage.pluginDB
    }

    header: inputOutputLevels

    footer: RowLayout {
        Kirigami.ActionToolBar {
            Layout.margins: Kirigami.Units.smallSpacing
            alignment: Qt.AlignRight
            position: Controls.ToolBar.Footer
            flat: true
            actions: [
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        crosstalkPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
