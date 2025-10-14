pragma ComponentBehavior: Bound
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import ee.presets as Presets
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import ee.type.presets as TypePresets
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: rnnoisePage

    required property var name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.inputLevelLeft = pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = pluginBackend.getOutputLevelRight();
    }

    function showStatus(label, positive = true) {
        status.text = label;

        if (positive) {
            status.type = Kirigami.MessageType.Positive;
            autoHideStatusTimer.start();
        } else {
            status.type = Kirigami.MessageType.Error;
        }

        status.visible = true;
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
        pluginBackend.updateLevelMeters = true;
    }
    Component.onDestruction: {
        if (pluginBackend) {
            pluginBackend.updateLevelMeters = false;
        }
    }

    FileDialog {
        id: fileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["RNNoise (*.rnnn)"]
        onAccepted: {
            if (Presets.Manager.importRNNoiseModel(fileDialog.selectedFiles) === 0)
                rnnoisePage.showStatus(i18n("Model File Imported.")// qmllint disable
                );
            else
                rnnoisePage.showStatus(i18n("Failed to Import the Model File."));// qmllint disable
        }
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            Layout.fillWidth: true
            uniformCellWidths: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Voice Detection") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSwitch {
                        id: enableVad

                        label: i18n("Enable") // qmllint disable
                        isChecked: rnnoisePage.pluginDB.enableVad
                        onCheckedChanged: {
                            if (isChecked !== rnnoisePage.pluginDB.enableVad)
                                rnnoisePage.pluginDB.enableVad = isChecked;
                        }
                    }

                    EeSpinBox {
                        id: vadThres

                        label: i18n("Threshold") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: rnnoisePage.pluginDB.getMinValue("vadThres")
                        to: rnnoisePage.pluginDB.getMaxValue("vadThres")
                        value: rnnoisePage.pluginDB.vadThres
                        decimals: 0
                        stepSize: 1
                        unit: "%"
                        onValueModified: v => {
                            rnnoisePage.pluginDB.vadThres = v;
                        }
                    }

                    EeSpinBox {
                        id: wet

                        label: i18n("Wet Level") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: rnnoisePage.pluginDB.getMinValue("wet")
                        to: rnnoisePage.pluginDB.getMaxValue("wet")
                        value: rnnoisePage.pluginDB.wet
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: v => {
                            rnnoisePage.pluginDB.wet = v;
                        }
                    }

                    EeSpinBox {
                        id: release

                        label: i18n("Release") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: rnnoisePage.pluginDB.getMinValue("release")
                        to: rnnoisePage.pluginDB.getMaxValue("release")
                        value: rnnoisePage.pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: "ms"
                        onValueModified: v => {
                            rnnoisePage.pluginDB.release = v;
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardModels

                implicitWidth: cardLayout.maximumColumnWidth
                actions: [
                    Kirigami.Action {

                        displayComponent: EeSwitch {
                            label: i18n("Use the Standard Model") // qmllint disable
                            isChecked: rnnoisePage.pluginDB.useStandardModel
                            onCheckedChanged: {
                                if (isChecked !== rnnoisePage.pluginDB.useStandardModel)
                                    rnnoisePage.pluginDB.useStandardModel = isChecked;
                            }
                        }
                    }
                ]

                header: Kirigami.Heading {
                    text: i18n("User Models") // qmllint disable
                    level: 2
                    enabled: !rnnoisePage.pluginDB.useStandardModel
                }

                contentItem: RowLayout {
                    id: listviewRow

                    ListView {
                        id: listView

                        clip: true
                        reuseItems: true
                        model: Presets.SortedRNNoiseListModel
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        enabled: !rnnoisePage.pluginDB.useStandardModel
                        Controls.ScrollBar.vertical: listViewScrollBar
                        Component.onCompleted: {
                            for (let n = 0; n < model.rowCount(); n++) {
                                const proxyIndex = model.index(n, 0);
                                const name = model.data(proxyIndex, TypePresets.ListModel.Name);
                                if (name === rnnoisePage.pluginDB.modelName) {
                                    currentIndex = n;
                                    break;
                                }
                            }
                        }

                        Kirigami.PlaceholderMessage {
                            anchors.centerIn: parent
                            width: parent.width - (Kirigami.Units.largeSpacing * 4)
                            visible: listView.count === 0
                            text: i18n("Empty") // qmllint disable
                        }

                        delegate: Controls.ItemDelegate {
                            id: listItemDelegate

                            required property string name
                            required property string path
                            required property int index

                            hoverEnabled: true
                            width: listView.width
                            highlighted: listItemDelegate.ListView.isCurrentItem
                            onClicked: {
                                rnnoisePage.pluginDB.modelName = name;
                                listItemDelegate.ListView.view.currentIndex = index;
                                rnnoisePage.showStatus(i18n("Loaded Model: %1", name));// qmllint disable
                            }

                            contentItem: RowLayout {
                                Controls.Label {
                                    text: listItemDelegate.name
                                }

                                Kirigami.ActionToolBar {
                                    alignment: Qt.AlignRight
                                    actions: [
                                        Kirigami.Action {
                                            text: i18n("Delete this Model") // qmllint disable
                                            icon.name: "delete"
                                            displayHint: Kirigami.DisplayHint.AlwaysHide
                                            onTriggered: {
                                                if (Presets.Manager.removeRNNoiseModel(listItemDelegate.path) === true)
                                                    rnnoisePage.showStatus(i18n("Removed Model: %1", listItemDelegate.name) // qmllint disable
                                                    );
                                                else
                                                    rnnoisePage.showStatus(i18n("Failed to Remove the Model: %1", listItemDelegate.name)// qmllint disable
                                                    , false);
                                            }
                                        }
                                    ]
                                }
                            }
                        }
                    }

                    Controls.ScrollBar {
                        id: listViewScrollBar

                        parent: listviewRow
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: rnnoisePage.pluginDB
    }

    footer: ColumnLayout {
        Kirigami.InlineMessage {
            id: status

            Layout.fillWidth: true
            Layout.maximumWidth: parent.width
            visible: false
            showCloseButton: true
        }

        Timer {
            id: autoHideStatusTimer
            interval: DB.Manager.main.autoHideInlineMessageTimeout
            onTriggered: {
                status.visible = false;
                autoHideStatusTimer.stop();
            }
        }

        RowLayout {
            Controls.Label {
                text: i18n("Using %1", `<b>${TagsPluginName.Package.rnnoise}</b>`) // qmllint disable
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
                        text: i18n("Import Model") // qmllint disable
                        icon.name: "document-import-symbolic"
                        onTriggered: {
                            fileDialog.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Reset") // qmllint disable
                        icon.name: "edit-reset-symbolic"
                        onTriggered: {
                            rnnoisePage.pluginBackend.reset();
                        }
                    }
                ]
            }
        }
    }
}
