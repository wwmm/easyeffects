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
import ee.ui
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: levelMeterPage

    required property string name
    required property DbLevelMeter pluginDB
    required property EffectsBase pipelineInstance
    property BackendLevelMeter pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        momentary.setValue(pluginBackend.getMomentaryLevel());
        shortterm.setValue(pluginBackend.getShorttermLevel());
        integrated.setValue(pluginBackend.getIntegratedLevel());
        relative.setValue(pluginBackend.getRelativeLevel());
        range.setValue(pluginBackend.getRangeLevel());
        truePeakL.setValue(pluginBackend.getTruePeakL());
        truePeakR.setValue(pluginBackend.getTruePeakR());
        inputL.setValue(pluginBackend.getInputLevelLeft());
        inputR.setValue(pluginBackend.getInputLevelRight());
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            Layout.fillWidth: true
            uniformCellWidths: true

            ColumnLayout {
                Kirigami.Card {
                    id: cardInputLevels

                    header: Kirigami.Heading {
                        text: i18n("Input Level") // qmllint disable
                        level: 2
                        leftPadding: Kirigami.Units.largeSpacing
                        rightPadding: Kirigami.Units.largeSpacing
                    }

                    contentItem: ColumnLayout {
                        spacing: 0

                        EeProgressBar {
                            id: inputL

                            label: i18n("Left") // qmllint disable
                            unit: Units.dB
                            from: Common.minimumDecibelLevel
                            to: 10
                            decimals: 1
                        }

                        EeProgressBar {
                            id: inputR

                            label: i18n("Right") // qmllint disable
                            unit: Units.dB
                            from: Common.minimumDecibelLevel
                            to: 10
                            decimals: 1
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }

                Kirigami.Card {
                    id: cardPeak

                    header: Kirigami.Heading {
                        text: i18n("True Peak") // qmllint disable
                        level: 2
                        leftPadding: Kirigami.Units.largeSpacing
                        rightPadding: Kirigami.Units.largeSpacing
                    }

                    contentItem: ColumnLayout {
                        spacing: 0

                        EeProgressBar {
                            id: truePeakL

                            label: i18n("Left") // qmllint disable
                            unit: Units.dB
                            from: Common.minimumDecibelLevel
                            to: 10
                            decimals: 1
                        }

                        EeProgressBar {
                            id: truePeakR

                            label: i18n("Right") // qmllint disable
                            unit: Units.dB
                            from: Common.minimumDecibelLevel
                            to: 10
                            decimals: 1
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardLoudness

                header: Kirigami.Heading {
                    text: i18n("Loudness") // qmllint disable
                    level: 2
                    leftPadding: Kirigami.Units.largeSpacing
                    rightPadding: Kirigami.Units.largeSpacing
                }

                contentItem: ColumnLayout {
                    spacing: 0

                    EeProgressBar {
                        id: momentary

                        label: i18n("Momentary") // qmllint disable
                        unit: Units.lufs
                        from: Common.minimumDecibelLevel
                        to: 10
                        decimals: 1
                    }

                    EeProgressBar {
                        id: shortterm

                        label: i18n("Short-term") // qmllint disable
                        unit: Units.lufs
                        from: Common.minimumDecibelLevel
                        to: 10
                        decimals: 1
                    }

                    EeProgressBar {
                        id: integrated

                        label: i18n("Integrated") // qmllint disable
                        unit: Units.lufs
                        from: Common.minimumDecibelLevel
                        to: 10
                        decimals: 1
                    }

                    EeProgressBar {
                        id: relative

                        label: i18n("Relative") // qmllint disable
                        unit: Units.lufs
                        from: Common.minimumDecibelLevel
                        to: 10
                        decimals: 1
                    }

                    EeProgressBar {
                        id: range

                        label: i18n("Range") // qmllint disable
                        unit: Units.lu
                        from: 0
                        to: 50
                        decimals: 1
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${PluginsPackage.ebur128}</strong>`) // qmllint disable
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
                        levelMeterPage.pluginBackend.resetHistory();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset settings") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        levelMeterPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
