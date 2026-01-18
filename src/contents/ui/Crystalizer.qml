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

//pragma explanation: https://doc.qt.io/qt-6/qtqml-documents-structure.html
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: crystalizerPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(crystalizerPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(crystalizerPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(crystalizerPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(crystalizerPage.pluginBackend.getOutputLevelRight());

        const newData = pluginBackend.getAdaptiveIntensities();

        for (let n = 0; n < crystalizerPage.pluginBackend.numBands; n++) {
            listModel.setProperty(n, "adaptiveIntensity", newData[n]);
        }
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);

        for (let n = 0; n < crystalizerPage.pluginBackend.numBands; n++) {
            listModel.append({
                "adaptiveIntensity": 1.0
            });
        }
    }

    ListModel {
        id: listModel
    }

    ColumnLayout {
        height: crystalizerPage.height - crystalizerPage.header.height - crystalizerPage.footer.height - Kirigami.Units.gridUnit

        Kirigami.Card {
            id: cardControls

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: false
            Layout.fillHeight: true

            footer: Controls.ScrollBar {
                id: listViewScrollBar

                Layout.fillWidth: true
            }

            contentItem: ListView {
                id: listview

                implicitWidth: contentItem.childrenRect.width < crystalizerPage.width ? contentItem.childrenRect.width : crystalizerPage.width - 4 * (cardControls.leftPadding + cardControls.rightPadding)
                clip: true
                reuseItems: true
                orientation: ListView.Horizontal
                model: listModel
                Controls.ScrollBar.horizontal: listViewScrollBar

                delegate: CrystalizerBand {
                    pluginDB: crystalizerPage.pluginDB
                    pluginBackend: crystalizerPage.pluginBackend
                }
            }
        }

        Kirigami.CardsLayout {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.leftMargin: 3 * Kirigami.Units.mediumSpacing
            Layout.rightMargin: 3 * Kirigami.Units.mediumSpacing
            Layout.fillHeight: false
            uniformCellWidths: true

            EeSpinBox {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: false
                enabled: crystalizerPage.pluginDB.oversampling
                label: i18n("Oversampling quality") // qmllint disable
                labelFillWidth: true
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: crystalizerPage.pluginDB.getMinValue("oversamplingQuality")
                to: crystalizerPage.pluginDB.getMaxValue("oversamplingQuality")
                value: crystalizerPage.pluginDB.oversamplingQuality
                decimals: 0
                stepSize: 1
                onValueModified: v => {
                    crystalizerPage.pluginDB.oversamplingQuality = v;
                }
            }

            EeSpinBox {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: false
                enabled: crystalizerPage.pluginDB.transitionBand
                label: i18n("Transition band") // qmllint disable
                labelFillWidth: true
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: crystalizerPage.pluginDB.getMinValue("transitionBand")
                to: crystalizerPage.pluginDB.getMaxValue("transitionBand")
                value: crystalizerPage.pluginDB.transitionBand
                decimals: 0
                stepSize: 1
                unit: Units.hz
                onValueModified: v => {
                    crystalizerPage.pluginDB.transitionBand = v;
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: crystalizerPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.zita}</strong>`) // qmllint disable
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
                    text: i18n("Oversampling") // qmllint disable
                    icon.name: "waveform-symbolic"
                    checkable: true
                    checked: crystalizerPage.pluginDB.oversampling
                    onTriggered: {
                        if (crystalizerPage.pluginDB.oversampling != checked)
                            crystalizerPage.pluginDB.oversampling = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Adaptive intensity") // qmllint disable
                    icon.name: "adjustlevels-symbolic"
                    checkable: true
                    checked: crystalizerPage.pluginDB.adaptiveIntensity
                    onTriggered: {
                        if (crystalizerPage.pluginDB.adaptiveIntensity != checked)
                            crystalizerPage.pluginDB.adaptiveIntensity = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Fixed quantum") // qmllint disable
                    icon.name: crystalizerPage.pluginDB.useFixedQuantum ? "object-locked-symbolic" : "unlock-symbolic"
                    checkable: true
                    checked: crystalizerPage.pluginDB.useFixedQuantum
                    onTriggered: {
                        if (crystalizerPage.pluginDB.useFixedQuantum != checked)
                            crystalizerPage.pluginDB.useFixedQuantum = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        crystalizerPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
