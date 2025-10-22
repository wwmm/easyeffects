//pragma explanation: https://doc.qt.io/qt-6/qtqml-documents-structure.html
pragma ComponentBehavior: Bound
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: equalizerPage

    required property string name
    required property var pluginDB
    required property var leftDB
    required property var rightDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.inputLevelLeft = equalizerPage.pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = equalizerPage.pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = equalizerPage.pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = equalizerPage.pluginBackend.getOutputLevelRight();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    EqualizerBandMenu {
        id: bandMenu

        bandDB: {
            equalizerPage.pluginDB.splitChannels ? (equalizerPage.pluginDB.viewLeftChannel ? equalizerPage.leftDB : equalizerPage.rightDB) : equalizerPage.leftDB;
        }
    }

    FileDialog {
        id: apoFileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: [i18n("APO Presets") + " (*.txt)"]// qmllint disable
        onAccepted: {
            if (equalizerPage.pluginBackend.importApoPreset(apoFileDialog.selectedFiles) === true) {
                appWindow.showStatus(i18n("Imported the Equalizer APO Preset File."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to Import the Equalizer APO Preset File."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    FileDialog {
        id: apoGraphicEqFileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: [i18n("GraphicEQ Presets") + " (*.txt)"]// qmllint disable
        onAccepted: {
            if (equalizerPage.pluginBackend.importApoGraphicEqPreset(apoGraphicEqFileDialog.selectedFiles) === true) {
                appWindow.showStatus(i18n("Imported the GraphicEQ Preset File."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to Import the GraphicEQ Preset File."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    FileDialog {
        id: apoExportFileDialog

        fileMode: FileDialog.SaveFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: [i18n("APO Preset") + " (*.txt)"]// qmllint disable
        onAccepted: {
            if (equalizerPage.pluginBackend.exportApoPreset(apoExportFileDialog.selectedFile) === true) {
                appWindow.showStatus(i18n("Exported the Current Equalizer Settings to an External APO Preset File."), Kirigami.MessageType.Positive);
                // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to Export the Current Equalizer Settings to an External APO Preset File."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    ColumnLayout {
        height: equalizerPage.height - equalizerPage.header.height - equalizerPage.footer.height - Kirigami.Units.gridUnit
        Kirigami.CardsLayout {
            maximumColumns: 5
            readonly property real columnSize: pitchLeft.implicitWidth
            minimumColumnWidth: columnSize
            maximumColumnWidth: columnSize
            Layout.fillHeight: false

            FormCard.FormComboBoxDelegate {
                id: mode

                text: i18n("Mode") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: equalizerPage.pluginDB.mode
                editable: false
                model: ["IIR", "FIR", "FFT", "SPM"]
                onActivated: idx => {
                    equalizerPage.pluginDB.mode = idx;
                }
            }

            EeSpinBox {
                id: numBands

                label: i18n("Bands") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: equalizerPage.pluginDB.getMinValue("numBands")
                to: equalizerPage.pluginDB.getMaxValue("numBands")
                value: equalizerPage.pluginDB.numBands
                decimals: 0
                stepSize: 1
                onValueModified: v => {
                    equalizerPage.pluginDB.numBands = v;
                }
            }

            EeSpinBox {
                id: balance

                label: i18n("Balance") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: equalizerPage.pluginDB.getMinValue("balance")
                to: equalizerPage.pluginDB.getMaxValue("balance")
                value: equalizerPage.pluginDB.balance
                decimals: 1
                stepSize: 0.1
                unit: "%"
                onValueModified: v => {
                    equalizerPage.pluginDB.balance = v;
                }
            }

            EeSpinBox {
                id: pitchLeft

                label: i18n("Pitch Left") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: equalizerPage.pluginDB.getMinValue("pitchLeft")
                to: equalizerPage.pluginDB.getMaxValue("pitchLeft")
                value: equalizerPage.pluginDB.pitchLeft
                decimals: 2
                stepSize: 0.01
                unit: "st"
                onValueModified: v => {
                    equalizerPage.pluginDB.pitchLeft = v;
                }
            }

            EeSpinBox {
                id: pitchRight

                label: i18n("Pitch Right") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: equalizerPage.pluginDB.getMinValue("pitchRight")
                to: equalizerPage.pluginDB.getMaxValue("pitchRight")
                value: equalizerPage.pluginDB.pitchRight
                decimals: 2
                stepSize: 0.01
                unit: "st"
                onValueModified: v => {
                    equalizerPage.pluginDB.pitchRight = v;
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
                        visible: equalizerPage.pluginDB.splitChannels
                        text: equalizerPage.pluginDB.splitChannels ? (equalizerPage.pluginDB.viewLeftChannel ? i18n("Left") : i18n("Right")) : "" // qmllint disable
                        level: 2
                    }

                    Kirigami.ActionToolBar {
                        Layout.margins: Kirigami.Units.smallSpacing
                        alignment: Qt.AlignRight
                        position: Controls.ToolBar.Header
                        flat: true
                        actions: [
                            Kirigami.Action {
                                id: viewLeft
                                visible: equalizerPage.pluginDB.splitChannels
                                checkable: true
                                checked: equalizerPage.pluginDB.viewLeftChannel
                                icon.name: "arrow-left-symbolic"
                                onTriggered: {
                                    equalizerPage.pluginDB.viewLeftChannel = true;
                                }
                            },
                            Kirigami.Action {
                                id: viewRight
                                visible: equalizerPage.pluginDB.splitChannels
                                checkable: true
                                checked: !equalizerPage.pluginDB.viewLeftChannel
                                icon.name: "arrow-right-symbolic"
                                onTriggered: {
                                    equalizerPage.pluginDB.viewLeftChannel = false;
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

                    implicitWidth: contentItem.childrenRect.width < equalizerPage.width ? contentItem.childrenRect.width : equalizerPage.width - 4 * (bandsCard.leftPadding + bandsCard.rightPadding)
                    clip: true
                    reuseItems: true
                    orientation: ListView.Horizontal
                    model: equalizerPage.pluginDB.numBands
                    Controls.ScrollBar.horizontal: listViewScrollBar

                    delegate: EqualizerBand {
                        bandDB: {
                            equalizerPage.pluginDB.splitChannels ? (equalizerPage.pluginDB.viewLeftChannel ? equalizerPage.leftDB : equalizerPage.rightDB) : equalizerPage.leftDB;
                        }
                        menu: bandMenu
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: equalizerPage.pluginDB
    }

    footer: ColumnLayout {
        RowLayout {
            Controls.Label {
                text: i18n("Using %1", `<strong>${TagsPluginName.Package.lsp}</strong>`) // qmllint disable
                textFormat: Text.RichText
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: false
                Layout.leftMargin: Kirigami.Units.smallSpacing
                color: Kirigami.Theme.disabledTextColor
            }

            Kirigami.ActionToolBar {
                Layout.margins: Kirigami.Units.smallSpacing
                alignment: Qt.AlignRight
                position: Controls.ToolBar.Footer
                flat: true
                actions: [
                    Kirigami.Action {
                        text: i18n("Show Native Window") // qmllint disable
                        icon.name: "window-duplicate-symbolic"
                        enabled: DB.Manager.main.showNativePluginUi
                        checkable: true
                        checked: equalizerPage.pluginBackend ? equalizerPage.pluginBackend.hasNativeUi() : false
                        onTriggered: {
                            if (checked)
                                equalizerPage.pluginBackend.showNativeUi();
                            else
                                equalizerPage.pluginBackend.closeNativeUi();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Split Channels") // qmllint disable
                        icon.name: "split-symbolic"
                        checkable: true
                        checked: equalizerPage.pluginDB.splitChannels
                        onTriggered: {
                            if (equalizerPage.pluginDB.splitChannels != checked)
                                equalizerPage.pluginDB.splitChannels = checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Flat Response") // qmllint disable
                        icon.name: "map-flat-symbolic"
                        onTriggered: {
                            equalizerPage.pluginBackend.flatResponse();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Calculate Frequencies") // qmllint disable
                        icon.name: "folder-calculate-symbolic"
                        onTriggered: {
                            equalizerPage.pluginBackend.calculateFrequencies();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Sort Bands") // qmllint disable
                        icon.name: "sort_incr-symbolic"
                        onTriggered: {
                            equalizerPage.pluginBackend.sortBands();
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
                        text: i18n("Reset") // qmllint disable
                        icon.name: "edit-reset-symbolic"
                        onTriggered: {
                            equalizerPage.pluginBackend.reset();
                        }
                    }
                ]
            }
        }
    }
}
