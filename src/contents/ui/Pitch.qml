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
    id: pitchPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pitchPage.pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(pitchPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(pitchPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(pitchPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(pitchPage.pluginBackend.getOutputLevelRight());
    }

    Component.onCompleted: {
        pitchPage.pluginBackend = pitchPage.pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true
            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            uniformCellWidths: true

            EeCard {
                id: cardControls

                title: i18n("Controls") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0

                    EeSpinBox {
                        id: sequenceLength

                        label: i18n("Sequence length") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("sequenceLength")
                        to: pitchPage.pluginDB.getMaxValue("sequenceLength")
                        value: pitchPage.pluginDB.sequenceLength
                        decimals: 0
                        stepSize: 1
                        unit: Units.ms
                        onValueModified: v => {
                            pitchPage.pluginDB.sequenceLength = v;
                        }
                    }

                    EeSpinBox {
                        id: seekWindow

                        label: i18n("Seek window") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("seekWindow")
                        to: pitchPage.pluginDB.getMaxValue("seekWindow")
                        value: pitchPage.pluginDB.seekWindow
                        decimals: 0
                        stepSize: 1
                        unit: Units.ms
                        onValueModified: v => {
                            pitchPage.pluginDB.seekWindow = v;
                        }
                    }

                    EeSpinBox {
                        id: overlapLength

                        Layout.columnSpan: 2
                        label: i18n("Overlap length") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("overlapLength")
                        to: pitchPage.pluginDB.getMaxValue("overlapLength")
                        value: pitchPage.pluginDB.overlapLength
                        decimals: 0
                        stepSize: 1
                        unit: Units.ms
                        onValueModified: v => {
                            pitchPage.pluginDB.overlapLength = v;
                        }
                    }

                    EeSpinBox {
                        id: dry

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Dry") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("dry")
                        to: pitchPage.pluginDB.getMaxValue("dry")
                        value: pitchPage.pluginDB.dry
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            pitchPage.pluginDB.dry = v;
                        }
                    }

                    EeSpinBox {
                        id: wet

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Wet") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("wet")
                        to: pitchPage.pluginDB.getMaxValue("wet")
                        value: pitchPage.pluginDB.wet
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: Units.dB
                        minusInfinityMode: true
                        onValueModified: v => {
                            pitchPage.pluginDB.wet = v;
                        }
                    }
                }
            }

            EeCard {
                id: cardLevels

                implicitWidth: cardLayout.maximumColumnWidth

                title: i18n("Pitch") // qmllint disable

                GridLayout {
                    columns: 2
                    uniformCellWidths: true
                    rowSpacing: 0

                    EeSpinBox {
                        id: octaves

                        Layout.columnSpan: 2
                        label: i18n("Octaves") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("octaves")
                        to: pitchPage.pluginDB.getMaxValue("octaves")
                        value: pitchPage.pluginDB.octaves
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            pitchPage.pluginDB.octaves = v;
                        }
                    }

                    EeSpinBox {
                        id: cents

                        label: i18n("Cents") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("cents")
                        to: pitchPage.pluginDB.getMaxValue("cents")
                        value: pitchPage.pluginDB.cents
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            pitchPage.pluginDB.cents = v;
                        }
                    }

                    EeSpinBox {
                        id: semitones

                        label: i18n("Semitones") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("semitones")
                        to: pitchPage.pluginDB.getMaxValue("semitones")
                        value: pitchPage.pluginDB.semitones
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            pitchPage.pluginDB.semitones = v;
                        }
                    }

                    EeSpinBox {
                        id: tempoDifference

                        label: i18n("Tempo difference") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("tempoDifference")
                        to: pitchPage.pluginDB.getMaxValue("tempoDifference")
                        value: pitchPage.pluginDB.tempoDifference
                        decimals: 0
                        stepSize: 1
                        unit: Units.percent
                        onValueModified: v => {
                            pitchPage.pluginDB.tempoDifference = v;
                        }
                    }

                    EeSpinBox {
                        id: rateDifference

                        label: i18n("Rate difference") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pitchPage.pluginDB.getMinValue("rateDifference")
                        to: pitchPage.pluginDB.getMaxValue("rateDifference")
                        value: pitchPage.pluginDB.rateDifference
                        decimals: 0
                        stepSize: 1
                        unit: Units.percent
                        onValueModified: v => {
                            pitchPage.pluginDB.rateDifference = v;
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: pitchPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.soundTouch}</strong>`) // qmllint disable
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
                        pitchPage.pluginBackend.resetHistory();
                    }
                },
                Kirigami.Action {
                    text: i18n("Quick seek") // qmllint disable
                    icon.name: "media-seek-forward-symbolic"
                    checkable: true
                    checked: pitchPage.pluginDB.quickSeek
                    onTriggered: {
                        if (pitchPage.pluginDB.quickSeek !== checked)
                            pitchPage.pluginDB.quickSeek = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Anti-aliasing") // qmllint disable
                    icon.name: "filter-symbolic"
                    checkable: true
                    checked: pitchPage.pluginDB.antiAlias
                    onTriggered: {
                        if (pitchPage.pluginDB.antiAlias !== checked)
                            pitchPage.pluginDB.antiAlias = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset settings") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        pitchPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
