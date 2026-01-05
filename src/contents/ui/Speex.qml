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
    id: speexPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!speexPage.pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(speexPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(speexPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(speexPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(speexPage.pluginBackend.getOutputLevelRight());
    }

    Component.onCompleted: {
        speexPage.pluginBackend = speexPage.pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            Layout.fillWidth: true

            EeCard {
                id: cardControls

                title: i18n("Controls") // qmllint disable

                EeSwitch {
                    id: enableDenoise

                    label: i18n("Denoise") // qmllint disable
                    isChecked: speexPage.pluginDB.enableDenoise
                    onCheckedChanged: {
                        if (isChecked !== speexPage.pluginDB.enableDenoise)
                            speexPage.pluginDB.enableDenoise = isChecked;
                    }
                }

                EeSwitch {
                    id: enableAgc

                    label: i18n("Automatic gain control") // qmllint disable
                    isChecked: speexPage.pluginDB.enableAgc
                    onCheckedChanged: {
                        if (isChecked !== speexPage.pluginDB.enableAgc)
                            speexPage.pluginDB.enableAgc = isChecked;
                    }
                }

                EeSwitch {
                    id: enableDereverb

                    label: i18n("Dereverberation") // qmllint disable
                    isChecked: speexPage.pluginDB.enableDereverb
                    onCheckedChanged: {
                        if (isChecked !== speexPage.pluginDB.enableDereverb)
                            speexPage.pluginDB.enableDereverb = isChecked;
                    }
                }

                EeSpinBox {
                    id: noiseSuppression

                    label: i18n("Noise suppression") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: speexPage.pluginDB.getMinValue("noiseSuppression")
                    to: speexPage.pluginDB.getMaxValue("noiseSuppression")
                    value: speexPage.pluginDB.noiseSuppression
                    decimals: 2
                    stepSize: 0.1
                    unit: Units.dB
                    onValueModified: v => {
                        speexPage.pluginDB.noiseSuppression = v;
                    }
                }
            }

            EeCard {
                title: i18n("Voice Activity Probability") // qmllint disable

                EeSwitch {
                    id: enableVad

                    label: i18n("Voice detection") // qmllint disable
                    isChecked: speexPage.pluginDB.enableVad
                    onCheckedChanged: {
                        if (isChecked !== speexPage.pluginDB.enableVad)
                            speexPage.pluginDB.enableVad = isChecked;
                    }
                }

                EeSpinBox {
                    id: vadProbabilityStart

                    Layout.columnSpan: 2
                    label: i18n("Start") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: speexPage.pluginDB.getMinValue("vadProbabilityStart")
                    to: speexPage.pluginDB.getMaxValue("vadProbabilityStart")
                    value: speexPage.pluginDB.vadProbabilityStart
                    decimals: 0
                    stepSize: 1
                    unit: Units.percent
                    enabled: enableVad.isChecked
                    onValueModified: v => {
                        speexPage.pluginDB.vadProbabilityStart = v;
                    }
                }

                EeSpinBox {
                    id: vadProbabilityContinue

                    Layout.columnSpan: 2
                    label: i18n("Continue") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: speexPage.pluginDB.getMinValue("vadProbabilityContinue")
                    to: speexPage.pluginDB.getMaxValue("vadProbabilityContinue")
                    value: speexPage.pluginDB.vadProbabilityContinue
                    decimals: 0
                    stepSize: 1
                    unit: Units.percent
                    enabled: enableVad.isChecked
                    onValueModified: v => {
                        speexPage.pluginDB.vadProbabilityContinue = v;
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: speexPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.speex}</b>`) // qmllint disable
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
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        speexPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
