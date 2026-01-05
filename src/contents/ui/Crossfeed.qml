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
    id: crossfeedPage

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

    function applyPreset(name) {
        switch (name) {
        case "cmoy":
            pluginDB.fcut = 700;
            pluginDB.feed = 6;
            break;
        case "jmeier":
            pluginDB.fcut = 650;
            pluginDB.feed = 9.5;
            break;
        default:
            pluginDB.fcut = 700;
            pluginDB.feed = 4.5;
        }
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

                EeSpinBox {
                    id: fcut

                    label: i18n("Cutoff") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: crossfeedPage.pluginDB.getMinValue("fcut")
                    to: crossfeedPage.pluginDB.getMaxValue("fcut")
                    value: crossfeedPage.pluginDB.fcut
                    decimals: 0
                    stepSize: 1
                    unit: Units.hz
                    onValueModified: v => {
                        crossfeedPage.pluginDB.fcut = v;
                    }
                }

                EeSpinBox {
                    id: feed

                    label: i18n("Feed") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: crossfeedPage.pluginDB.getMinValue("feed")
                    to: crossfeedPage.pluginDB.getMaxValue("feed")
                    value: crossfeedPage.pluginDB.feed
                    decimals: 1
                    stepSize: 0.1
                    unit: Units.dB
                    onValueModified: v => {
                        crossfeedPage.pluginDB.feed = v;
                    }
                }
            }
        }
    }

    Kirigami.MenuDialog {
        id: presetsDialog

        title: i18n("Crossfeed Presets") // qmllint disable
        actions: [
            Kirigami.Action {
                icon.name: "user-bookmarks-symbolic"
                text: i18n("Default") // qmllint disable
                onTriggered: {
                    crossfeedPage.applyPreset("default");
                }
            },
            Kirigami.Action {
                icon.name: "user-bookmarks-symbolic"
                text: "Cmoy"
                onTriggered: {
                    crossfeedPage.applyPreset("cmoy");
                }
            },
            Kirigami.Action {
                icon.name: "user-bookmarks-symbolic"
                text: "Jmeier"
                onTriggered: {
                    crossfeedPage.applyPreset("jmeier");
                }
            }
        ]
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: crossfeedPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.bs2b}</strong>`) // qmllint disable
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
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        crossfeedPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
