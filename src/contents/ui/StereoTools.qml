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
    id: stereoToolsPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!stereoToolsPage.pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(stereoToolsPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(stereoToolsPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(stereoToolsPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(stereoToolsPage.pluginBackend.getOutputLevelRight());
    }

    Component.onCompleted: {
        stereoToolsPage.pluginBackend = stereoToolsPage.pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            maximumColumns: 4
            uniformCellWidths: true

            EeCard {
                title: i18n("Input") // qmllint disable

                EeSwitch {
                    id: softclip

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Softclip") // qmllint disable
                    isChecked: stereoToolsPage.pluginDB.softclip
                    onCheckedChanged: {
                        if (isChecked !== stereoToolsPage.pluginDB.softclip)
                            stereoToolsPage.pluginDB.softclip = isChecked;
                    }
                }

                EeSpinBox {
                    id: balanceIn

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Balance") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: stereoToolsPage.pluginDB.getMinValue("balanceIn")
                    to: stereoToolsPage.pluginDB.getMaxValue("balanceIn")
                    value: stereoToolsPage.pluginDB.balanceIn
                    decimals: 2
                    stepSize: 0.01
                    onValueModified: v => {
                        stereoToolsPage.pluginDB.balanceIn = v;
                    }
                }

                EeSpinBox {
                    id: scLevel

                    Layout.alignment: Qt.AlignTop
                    label: i18n("S/C Level") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: stereoToolsPage.pluginDB.getMinValue("scLevel")
                    to: stereoToolsPage.pluginDB.getMaxValue("scLevel")
                    value: stereoToolsPage.pluginDB.scLevel
                    decimals: 3
                    stepSize: 0.001
                    enabled: stereoToolsPage.pluginDB.softclip
                    onValueModified: v => {
                        stereoToolsPage.pluginDB.scLevel = v;
                    }
                }
            }

            EeCard {
                title: i18n("Stereo Matrix") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0
                    Layout.alignment: Qt.AlignTop

                    FormCard.FormComboBoxDelegate {
                        id: mode

                        Layout.columnSpan: 2
                        verticalPadding: Kirigami.Units.smallSpacing
                        text: i18n("Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: stereoToolsPage.pluginDB.mode
                        editable: false
                        model: [i18n("LR > LR (Stereo default)"), i18n("LR > MS (Stereo to Mid-Side)"), i18n("MS > LR (Mid-Side to Stereo)"), i18n("LR > LL (Mono left channel)"), i18n("LR > RR (Mono right channel)"), i18n("LR > L+R (Mono sum L+R)"), i18n("LR > RL (Stereo flip channels)")] // qmllint disable
                        onActivated: idx => {
                            stereoToolsPage.pluginDB.mode = idx;
                        }
                    }

                    EeSpinBox {
                        id: slev

                        label: i18n("Side level") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("slev")
                        to: stereoToolsPage.pluginDB.getMaxValue("slev")
                        value: stereoToolsPage.pluginDB.slev
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.slev = v;
                        }
                    }

                    EeSpinBox {
                        id: sbal

                        label: i18n("Side balance") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("sbal")
                        to: stereoToolsPage.pluginDB.getMaxValue("sbal")
                        value: stereoToolsPage.pluginDB.sbal
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.sbal = v;
                        }
                    }

                    EeSpinBox {
                        id: mlev

                        label: i18n("Middle level") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("mlev")
                        to: stereoToolsPage.pluginDB.getMaxValue("mlev")
                        value: stereoToolsPage.pluginDB.mlev
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.mlev = v;
                        }
                    }

                    EeSpinBox {
                        id: mpan

                        label: i18n("Middle panorama") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("mpan")
                        to: stereoToolsPage.pluginDB.getMaxValue("mpan")
                        value: stereoToolsPage.pluginDB.mpan
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.mpan = v;
                        }
                    }
                }
            }

            EeCard {
                title: i18n("Left") // qmllint disable

                EeSwitch {
                    id: mutel

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Mute") // qmllint disable
                    isChecked: stereoToolsPage.pluginDB.mutel
                    onCheckedChanged: {
                        if (isChecked !== stereoToolsPage.pluginDB.mutel)
                            stereoToolsPage.pluginDB.mutel = isChecked;
                    }
                }

                EeSwitch {
                    id: phasel

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Invert phase") // qmllint disable
                    isChecked: stereoToolsPage.pluginDB.phasel
                    onCheckedChanged: {
                        if (isChecked !== stereoToolsPage.pluginDB.phasel)
                            stereoToolsPage.pluginDB.phasel = isChecked;
                    }
                }

                Kirigami.Heading {
                    text: i18n("Right") // qmllint disable
                    level: 2
                    topPadding: Kirigami.Units.gridUnit * 2
                    leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                    rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                }

                EeSwitch {
                    id: muter

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Mute") // qmllint disable
                    isChecked: stereoToolsPage.pluginDB.muter
                    onCheckedChanged: {
                        if (isChecked !== stereoToolsPage.pluginDB.muter)
                            stereoToolsPage.pluginDB.muter = isChecked;
                    }
                }

                EeSwitch {
                    id: phaser

                    Layout.alignment: Qt.AlignTop
                    label: i18n("Invert phase") // qmllint disable
                    isChecked: stereoToolsPage.pluginDB.phaser
                    onCheckedChanged: {
                        if (isChecked !== stereoToolsPage.pluginDB.phaser)
                            stereoToolsPage.pluginDB.phaser = isChecked;
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            EeCard {
                title: i18n("Output") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0

                    EeSpinBox {
                        id: balanceOut

                        label: i18n("Balance") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("balanceOut")
                        to: stereoToolsPage.pluginDB.getMaxValue("balanceOut")
                        value: stereoToolsPage.pluginDB.balanceOut
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.balanceOut = v;
                        }
                    }

                    EeSpinBox {
                        id: delay

                        label: i18n("Delay") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("delay")
                        to: stereoToolsPage.pluginDB.getMaxValue("delay")
                        value: stereoToolsPage.pluginDB.delay
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.delay = v;
                        }
                    }

                    EeSpinBox {
                        id: stereoBase

                        label: i18n("Stereo base") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("stereoBase")
                        to: stereoToolsPage.pluginDB.getMaxValue("stereoBase")
                        value: stereoToolsPage.pluginDB.stereoBase
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.stereoBase = v;
                        }
                    }

                    EeSpinBox {
                        id: stereoPhase

                        label: i18n("Stereo phase") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("stereoPhase")
                        to: stereoToolsPage.pluginDB.getMaxValue("stereoPhase")
                        value: stereoToolsPage.pluginDB.stereoPhase
                        decimals: 0
                        stepSize: 1
                        unit: Units.degrees
                        separateUnit: false
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.stereoPhase = v;
                        }
                    }

                    EeSpinBox {
                        id: dry

                        label: i18n("Dry") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("dry")
                        to: stereoToolsPage.pluginDB.getMaxValue("dry")
                        value: stereoToolsPage.pluginDB.dry
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.dry = v;
                        }
                    }

                    EeSpinBox {
                        id: wet

                        label: i18n("Wet") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: stereoToolsPage.pluginDB.getMinValue("wet")
                        to: stereoToolsPage.pluginDB.getMaxValue("wet")
                        value: stereoToolsPage.pluginDB.wet
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            stereoToolsPage.pluginDB.wet = v;
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: stereoToolsPage.pluginDB
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
                    checked: stereoToolsPage.pluginBackend ? stereoToolsPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            stereoToolsPage.pluginBackend.showNativeUi();
                        else
                            stereoToolsPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        stereoToolsPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
