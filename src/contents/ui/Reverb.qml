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
    id: reverbPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!reverbPage.pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(reverbPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(reverbPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(reverbPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(reverbPage.pluginBackend.getOutputLevelRight());
    }

    Component.onCompleted: {
        reverbPage.pluginBackend = reverbPage.pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            implicitWidth: cardLayout.maximumColumnWidth
            uniformCellWidths: true

            EeCard {
                title: i18n("Controls") // qmllint disable

                FormCard.FormComboBoxDelegate {
                    id: roomSize

                    Layout.alignment: Qt.AlignTop
                    verticalPadding: Kirigami.Units.smallSpacing
                    text: i18n("Room size") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: reverbPage.pluginDB.roomSize
                    editable: false
                    model: [i18n("Small"), i18n("Medium"), i18n("Large"), i18n("Tunnel-like"), i18n("Large/smooth"), i18n("Experimental")] // qmllint disable
                    onActivated: idx => {
                        reverbPage.pluginDB.roomSize = idx;
                    }
                }

                EeSpinBox {
                    id: decayTime

                    label: i18n("Decay time") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                    from: reverbPage.pluginDB.getMinValue("decayTime")
                    to: reverbPage.pluginDB.getMaxValue("decayTime")
                    value: reverbPage.pluginDB.decayTime
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.s
                    onValueModified: v => {
                        reverbPage.pluginDB.decayTime = v;
                    }
                }

                EeSpinBox {
                    id: predelay

                    label: i18n("Pre delay") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                    from: reverbPage.pluginDB.getMinValue("predelay")
                    to: reverbPage.pluginDB.getMaxValue("predelay")
                    value: reverbPage.pluginDB.predelay
                    decimals: 0
                    stepSize: 1
                    unit: Units.ms
                    onValueModified: v => {
                        reverbPage.pluginDB.predelay = v;
                    }
                }

                EeSpinBox {
                    id: diffusion

                    label: i18n("Diffusion") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                    from: reverbPage.pluginDB.getMinValue("diffusion")
                    to: reverbPage.pluginDB.getMaxValue("diffusion")
                    value: reverbPage.pluginDB.diffusion
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.percent
                    onValueModified: v => {
                        reverbPage.pluginDB.diffusion = v;
                    }
                }
            }

            EeCard {
                title: i18n("Filter") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    Layout.alignment: Qt.AlignTop

                    EeSpinBox {
                        id: hfDamp

                        Layout.columnSpan: 2
                        label: i18n("High frequency damping") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: reverbPage.pluginDB.getMinValue("hfDamp")
                        to: reverbPage.pluginDB.getMaxValue("hfDamp")
                        value: reverbPage.pluginDB.hfDamp
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.hz
                        onValueModified: v => {
                            reverbPage.pluginDB.hfDamp = v;
                        }
                    }

                    EeSpinBox {
                        id: bassCut

                        label: i18n("Bass cut") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: reverbPage.pluginDB.getMinValue("bassCut")
                        to: reverbPage.pluginDB.getMaxValue("bassCut")
                        value: reverbPage.pluginDB.bassCut
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        onValueModified: v => {
                            reverbPage.pluginDB.bassCut = v;
                        }
                    }

                    EeSpinBox {
                        id: trebleCut

                        label: i18n("Treble cut") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: reverbPage.pluginDB.getMinValue("trebleCut")
                        to: reverbPage.pluginDB.getMaxValue("trebleCut")
                        value: reverbPage.pluginDB.trebleCut
                        decimals: 0
                        stepSize: 1
                        unit: Units.hz
                        onValueModified: v => {
                            reverbPage.pluginDB.trebleCut = v;
                        }
                    }

                    EeSpinBox {
                        id: dry

                        label: i18n("Dry") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: reverbPage.pluginDB.getMinValue("dry")
                        to: reverbPage.pluginDB.getMaxValue("dry")
                        value: reverbPage.pluginDB.dry
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            reverbPage.pluginDB.dry = v;
                        }
                    }

                    EeSpinBox {
                        id: wet

                        label: i18n("Wet") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: reverbPage.pluginDB.getMinValue("amount")
                        to: reverbPage.pluginDB.getMaxValue("amount")
                        value: reverbPage.pluginDB.amount
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            reverbPage.pluginDB.amount = v;
                        }
                    }
                }
            }
        }
    }

    Kirigami.MenuDialog {
        id: presetsDialog

        title: i18n("Reverberation Presets") // qmllint disable
        actions: [
            Kirigami.Action {
                icon.name: "user-bookmarks-symbolic"
                text: i18n("Ambience") // qmllint disable
                onTriggered: {
                    reverbPage.pluginDB.decayTime = 1.10354;
                    reverbPage.pluginDB.hfDamp = 2182.58;
                    reverbPage.pluginDB.roomSize = 4;
                    reverbPage.pluginDB.diffusion = 0.69;
                    reverbPage.pluginDB.amount = Common.linearTodb(0.291183);
                    reverbPage.pluginDB.dry = Common.linearTodb(1);
                    reverbPage.pluginDB.predelay = 6.5;
                    reverbPage.pluginDB.bassCut = 514.079;
                    reverbPage.pluginDB.trebleCut = 4064.15;
                }
            },
            Kirigami.Action {
                icon.name: "user-bookmarks-symbolic"
                text: i18n("Empty walls") // qmllint disable
                onTriggered: {
                    reverbPage.pluginDB.decayTime = 0.505687;
                    reverbPage.pluginDB.hfDamp = 3971.64;
                    reverbPage.pluginDB.roomSize = 4;
                    reverbPage.pluginDB.diffusion = 0.17;
                    reverbPage.pluginDB.amount = Common.linearTodb(0.198884);
                    reverbPage.pluginDB.dry = Common.linearTodb(1);
                    reverbPage.pluginDB.predelay = 13;
                    reverbPage.pluginDB.bassCut = 240.453;
                    reverbPage.pluginDB.trebleCut = 3303.47;
                }
            },
            Kirigami.Action {
                icon.name: "user-bookmarks-symbolic"
                text: i18n("Room") // qmllint disable
                onTriggered: {
                    reverbPage.pluginDB.decayTime = 0.445945;
                    reverbPage.pluginDB.hfDamp = 5508.46;
                    reverbPage.pluginDB.roomSize = 4;
                    reverbPage.pluginDB.diffusion = 0.54;
                    reverbPage.pluginDB.amount = Common.linearTodb(0.469761);
                    reverbPage.pluginDB.dry = Common.linearTodb(1);
                    reverbPage.pluginDB.predelay = 25;
                    reverbPage.pluginDB.bassCut = 257.65;
                    reverbPage.pluginDB.trebleCut = 20000;
                }
            },
            Kirigami.Action {
                icon.name: "user-bookmarks-symbolic"
                text: i18n("Large empty hall") // qmllint disable
                onTriggered: {
                    reverbPage.pluginBackend.reset();
                    reverbPage.pluginDB.decayTime = 2.00689;
                    reverbPage.pluginDB.hfDamp = 20000;
                    reverbPage.pluginDB.amount = Common.linearTodb(0.366022);
                }
            },
            Kirigami.Action {
                icon.name: "user-bookmarks-symbolic"
                text: i18n("Disco") // qmllint disable
                onTriggered: {
                    reverbPage.pluginBackend.reset();
                    reverbPage.pluginDB.decayTime = 1;
                    reverbPage.pluginDB.hfDamp = 3396.49;
                    reverbPage.pluginDB.amount = Common.linearTodb(0.269807);
                }
            },
            Kirigami.Action {
                icon.name: "user-bookmarks-symbolic"
                text: i18n("Large occupied hall") // qmllint disable
                onTriggered: {
                    reverbPage.pluginBackend.reset();
                    reverbPage.pluginDB.decayTime = 1;
                    reverbPage.pluginDB.hfDamp = 3396.49;
                    reverbPage.pluginDB.amount = Common.linearTodb(0.269807);
                }
            }
        ]
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: reverbPage.pluginDB
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
                    text: i18n("Presets") // qmllint disable
                    icon.name: "user-bookmarks-symbolic"
                    enabled: DbMain.showNativePluginUi
                    onTriggered: {
                        presetsDialog.open();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Show native window") // qmllint disable
                    icon.name: "window-duplicate-symbolic"
                    enabled: DbMain.showNativePluginUi
                    checkable: true
                    checked: reverbPage.pluginBackend ? reverbPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            reverbPage.pluginBackend.showNativeUi();
                        else
                            reverbPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        reverbPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
