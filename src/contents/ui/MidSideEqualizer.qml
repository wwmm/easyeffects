/**
 * Copyright © 2026 Alessio Attilio
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
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.ui
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: midsideEqualizerPage

    required property string name
    required property DbMidSideEqualizer pluginDB
    required property DbEqualizerChannel midDB
    required property DbEqualizerChannel sideDB
    required property EffectsBase pipelineInstance
    property BackendMidSideEqualizer pluginBackend

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

    EqualizerBandMenu {
        id: bandMenu

        bandDB: midsideEqualizerPage.pluginDB.splitChannels ? (midsideEqualizerPage.pluginDB.viewMidChannel ? midsideEqualizerPage.midDB : midsideEqualizerPage.sideDB) : midsideEqualizerPage.midDB // qmllint disable
    }

    FileDialog {
        id: apoFileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: [`${i18n("APO preset")} (*.txt)`] // qmllint disable
        onAccepted: {
            if (midsideEqualizerPage.pluginBackend.importApoPreset(apoFileDialog.selectedFiles) === false) {
                appWindow.showStatus(i18n("Failed to import the Mid-Side Equalizer APO preset file."), Kirigami.MessageType.Error, false); // qmllint disable
            } else {
                let status = "";

                if (!midsideEqualizerPage.pluginDB.splitChannels) {
                    status = i18n("Imported the Mid-Side Equalizer APO preset file."); // qmllint disable
                } else if (midsideEqualizerPage.pluginDB.viewMidChannel) {
                    status = i18n("Imported the Mid-Side Equalizer APO preset file into the mid channel."); // qmllint disable
                } else {
                    status = i18n("Imported the Mid-Side Equalizer APO preset file into the side channel."); // qmllint disable
                }

                appWindow.showStatus(status, Kirigami.MessageType.Positive);
            }
        }
    }

    FileDialog {
        id: apoGraphicEqFileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: [`${i18n("GraphicEQ preset")} (*.txt)`] // qmllint disable
        onAccepted: {
            if (midsideEqualizerPage.pluginBackend.importApoGraphicEqPreset(apoGraphicEqFileDialog.selectedFiles) === true) {
                appWindow.showStatus(i18n("Imported the GraphicEQ preset file."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to import the GraphicEQ preset file."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    FileDialog {
        id: apoExportFileDialog

        fileMode: FileDialog.SaveFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: [`${i18n("APO preset")} (*.txt)`] // qmllint disable
        onAccepted: {
            if (midsideEqualizerPage.pluginBackend.exportApoPreset(apoExportFileDialog.selectedFile) === false) {
                appWindow.showStatus(i18n("Failed to export the current Mid-Side Equalizer settings to an external APO preset file."), Kirigami.MessageType.Error, false); // qmllint disable
            } else {
                let status = "";

                if (!midsideEqualizerPage.pluginDB.splitChannels) {
                    status = i18n("Exported the current Mid-Side Equalizer settings to an external APO preset file."); // qmllint disable
                } else if (midsideEqualizerPage.pluginDB.viewMidChannel) {
                    status = i18n("Exported the current Mid-Side Equalizer settings of the mid channel to an external APO preset file."); // qmllint disable
                } else {
                    status = i18n("Exported the current Mid-Side Equalizer settings of the side channel to an external APO preset file."); // qmllint disable
                }

                appWindow.showStatus(status, Kirigami.MessageType.Positive);
            }
        }
    }

    ColumnLayout {
        height: midsideEqualizerPage.height - midsideEqualizerPage.header.height - midsideEqualizerPage.footer.height - Kirigami.Units.gridUnit
        Kirigami.CardsLayout {
            maximumColumns: 6
            readonly property real columnSize: pitchMid.implicitWidth
            minimumColumnWidth: columnSize
            maximumColumnWidth: columnSize
            Layout.fillHeight: false

            FormCard.FormComboBoxDelegate {
                id: mode

                text: i18n("Mode") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: midsideEqualizerPage.pluginDB.mode
                editable: false
                model: [i18n("IIR"), i18n("FIR"), i18n("FFT"), i18n("SPM")]
                onActivated: idx => {
                    midsideEqualizerPage.pluginDB.mode = idx;
                }
            }

            FormCard.FormComboBoxDelegate {
                id: decramp

                verticalPadding: Kirigami.Units.smallSpacing
                text: i18n("Equalizer decramping") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: midsideEqualizerPage.pluginDB.decramp
                editable: false
                model: [i18n("Off"), i18n("x2"), i18n("x3"), i18n("x4"), i18n("x6"), i18n("x8")]
                onActivated: idx => {
                    midsideEqualizerPage.pluginDB.decramp = idx;
                }
            }

            EeSpinBox {
                id: numBands

                label: i18n("Bands") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: midsideEqualizerPage.pluginDB.getMinValue("numBands")
                to: midsideEqualizerPage.pluginDB.getMaxValue("numBands")
                value: midsideEqualizerPage.pluginDB.numBands
                decimals: 0
                stepSize: 1
                onValueModified: v => {
                    midsideEqualizerPage.pluginDB.numBands = v;
                }
            }

            EeSpinBox {
                id: balance

                label: i18n("Balance") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: midsideEqualizerPage.pluginDB.getMinValue("balance")
                to: midsideEqualizerPage.pluginDB.getMaxValue("balance")
                value: midsideEqualizerPage.pluginDB.balance
                decimals: 1
                stepSize: 0.1
                unit: Units.percent
                onValueModified: v => {
                    midsideEqualizerPage.pluginDB.balance = v;
                }
            }

            EeSpinBox {
                id: pitchMid

                label: i18n("Pitch mid") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: midsideEqualizerPage.pluginDB.getMinValue("pitchMid")
                to: midsideEqualizerPage.pluginDB.getMaxValue("pitchMid")
                value: midsideEqualizerPage.pluginDB.pitchMid
                decimals: 2
                stepSize: 0.01
                unit: i18n("st")
                onValueModified: v => {
                    midsideEqualizerPage.pluginDB.pitchMid = v;
                }
            }

            EeSpinBox {
                id: pitchSide

                label: i18n("Pitch side") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: midsideEqualizerPage.pluginDB.getMinValue("pitchSide")
                to: midsideEqualizerPage.pluginDB.getMaxValue("pitchSide")
                value: midsideEqualizerPage.pluginDB.pitchSide
                decimals: 2
                stepSize: 0.01
                unit: i18n("st")
                onValueModified: v => {
                    midsideEqualizerPage.pluginDB.pitchSide = v;
                }
            }
        }

        Kirigami.CardsLayout {
            maximumColumns: 1
            readonly property real columnSize: bandsCard.implicitWidth
            minimumColumnWidth: columnSize
            maximumColumnWidth: columnSize
            Layout.minimumHeight: Kirigami.Units.gridUnit * 20

            Kirigami.Card {
                id: bandsCard
                Layout.fillHeight: true

                header: RowLayout {
                    Kirigami.Heading {
                        visible: midsideEqualizerPage.pluginDB.splitChannels
                        text: midsideEqualizerPage.pluginDB.splitChannels ? (midsideEqualizerPage.pluginDB.viewMidChannel ? i18n("Mid") : i18n("Side")) : "" // qmllint disable
                        level: 2
                    }

                    Kirigami.ActionToolBar {
                        Layout.margins: Kirigami.Units.smallSpacing
                        alignment: Qt.AlignRight
                        position: Controls.ToolBar.Header
                        flat: true
                        actions: [
                            Kirigami.Action {
                                id: viewMid
                                visible: midsideEqualizerPage.pluginDB.splitChannels
                                checkable: true
                                checked: midsideEqualizerPage.pluginDB.viewMidChannel
                                icon.name: "arrow-up-symbolic"
                                onTriggered: {
                                    midsideEqualizerPage.pluginDB.viewMidChannel = true;
                                }
                            },
                            Kirigami.Action {
                                id: viewSide
                                visible: midsideEqualizerPage.pluginDB.splitChannels
                                checkable: true
                                checked: !midsideEqualizerPage.pluginDB.viewMidChannel
                                icon.name: "arrow-down-symbolic"
                                onTriggered: {
                                    midsideEqualizerPage.pluginDB.viewMidChannel = false;
                                }
                            }
                        ]
                    }
                }

                footer: Controls.ScrollBar {
                    id: listViewScrollBar

                    Layout.fillWidth: true
                }

                contentItem: ListView {
                    id: listview

                    implicitWidth: contentItem.childrenRect.width < midsideEqualizerPage.width ? contentItem.childrenRect.width : midsideEqualizerPage.width - 4 * (bandsCard.leftPadding + bandsCard.rightPadding)
                    clip: true
                    reuseItems: true
                    orientation: ListView.Horizontal
                    model: midsideEqualizerPage.pluginDB.numBands
                    Controls.ScrollBar.horizontal: listViewScrollBar

                    delegate: EqualizerBand {
                        bandDB: midsideEqualizerPage.pluginDB.splitChannels ? (midsideEqualizerPage.pluginDB.viewMidChannel ? midsideEqualizerPage.midDB : midsideEqualizerPage.sideDB) : midsideEqualizerPage.midDB // qmllint disable

                        menu: bandMenu
                    }
                }
            }
        }
    }

    EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: midsideEqualizerPage.pluginDB
    }

    header: inputOutputLevels

    footer: ColumnLayout {
        RowLayout {
            Controls.Label {
                text: i18n("Using %1", `<strong>${PluginsPackage.lsp}</strong>`) // qmllint disable
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
                        checked: midsideEqualizerPage.pluginBackend ? midsideEqualizerPage.pluginBackend.hasNativeUi() : false
                        onTriggered: {
                            if (checked)
                                midsideEqualizerPage.pluginBackend.showNativeUi();
                            else
                                midsideEqualizerPage.pluginBackend.closeNativeUi();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Link Mid/Side") // qmllint disable
                        icon.name: "split-symbolic"
                        checkable: true
                        checked: midsideEqualizerPage.pluginDB.splitChannels
                        onTriggered: {
                            if (midsideEqualizerPage.pluginDB.splitChannels !== checked)
                                midsideEqualizerPage.pluginDB.splitChannels = checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Flat response") // qmllint disable
                        icon.name: "map-flat-symbolic"
                        onTriggered: {
                            midsideEqualizerPage.pluginBackend.flatResponse();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Calculate frequencies") // qmllint disable
                        icon.name: "folder-calculate-symbolic"
                        onTriggered: {
                            midsideEqualizerPage.pluginBackend.calculateFrequencies();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Sort bands") // qmllint disable
                        icon.name: "sort_incr-symbolic"
                        onTriggered: {
                            midsideEqualizerPage.pluginBackend.sortBands();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Import APO") // qmllint disable
                        icon.name: "document-import-symbolic"
                        onTriggered: {
                            apoFileDialog.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Import APO (GraphicEQ)") // qmllint disable
                        icon.name: "document-import-symbolic"
                        onTriggered: {
                            apoGraphicEqFileDialog.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Export APO") // qmllint disable
                        icon.name: "document-export-symbolic"
                        onTriggered: {
                            apoExportFileDialog.open();
                        }
                    },
                    Kirigami.Action {
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        text: i18n("Reset") // qmllint disable
                        icon.name: "edit-reset-symbolic"
                        onTriggered: {
                            midsideEqualizerPage.pluginBackend.reset();
                        }
                    }
                ]
            }
        }
    }
}
