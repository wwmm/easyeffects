import "Common.js" as Common
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import ee.presets as Presets
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: rnnoisePage

    required property var name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return ;

        inputOutputLevels.inputLevelLeft = pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = pluginBackend.getOutputLevelRight();
    }

    function showStatus(label) {
        status.text = label;
        status.visible = true;
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    FileDialog {
        id: fileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["RNNoise (*.rnnn)"]
        onAccepted: {
            if (Presets.Manager.importRNNoiseModel(fileDialog.selectedFiles) === 0)
                showStatus(i18n("Model files imported!"));
            else
                showStatus(i18n("Failed to import the model file!"));
        }
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true
            uniformCellWidths: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Voice Detection")
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSwitch {
                        id: enableVad

                        label: i18n("Enable")
                        isChecked: pluginDB.enableVad
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.enableVad)
                                pluginDB.enableVad = isChecked;

                        }
                    }

                    EeSpinBox {
                        id: vadThres

                        label: i18n("Threshold")
                        from: pluginDB.getMinValue("vadThres")
                        to: pluginDB.getMaxValue("vadThres")
                        value: pluginDB.vadThres
                        decimals: 0
                        stepSize: 1
                        unit: "%"
                        onValueModified: (v) => {
                            pluginDB.vadThres = v;
                        }
                    }

                    EeSpinBox {
                        id: wet

                        label: i18n("Wet Level")
                        from: pluginDB.getMinValue("wet")
                        to: pluginDB.getMaxValue("wet")
                        value: pluginDB.wet
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: (v) => {
                            pluginDB.wet = v;
                        }
                    }

                    EeSpinBox {
                        id: release

                        label: i18n("Release")
                        from: pluginDB.getMinValue("release")
                        to: pluginDB.getMaxValue("release")
                        value: pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.release = v;
                        }
                    }

                }

            }

            Kirigami.Card {
                id: cardModels

                implicitWidth: cardLayout.maximumColumnWidth

                header: Kirigami.Heading {
                    text: i18n("Models")
                    level: 2
                }

                contentItem: ListView {
                    id: listView

                    clip: true
                    reuseItems: true
                    model: Presets.SortedRNNoiseListModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: parent.width - (Kirigami.Units.largeSpacing * 4)
                        visible: listView.count === 0
                        text: i18n("Empty")
                    }

                    delegate: Controls.ItemDelegate {
                        id: listItemDelegate

                        required property string name
                        required property string path
                        property bool selected: listItemDelegate.highlighted || listItemDelegate.down
                        property color color: selected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

                        hoverEnabled: true
                        width: listView.width
                        onClicked: {
                            pluginDB.kernelName = name;
                            showStatus(i18n("Loaded Model: %1", name));
                        }

                        contentItem: RowLayout {
                            Controls.Label {
                                text: name
                            }

                            Kirigami.ActionToolBar {
                                alignment: Qt.AlignRight
                                actions: [
                                    Kirigami.Action {
                                        // if (Presets.Manager.removeImpulseFile(path) === true)
                                        //     showStatus(i18n("Removed Model: %1", name));
                                        // else
                                        //     showStatus(i18n("Failed to Remove: %1", name));

                                        text: i18n("Delete this Model")
                                        icon.name: "delete"
                                        displayHint: Kirigami.DisplayHint.AlwaysHide
                                        onTriggered: {
                                        }
                                    }
                                ]
                            }

                        }

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

        RowLayout {
            Controls.Label {
                text: i18n("Using %1", `<b>${TagsPluginName.Package.rnnoise}</b>`)
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
                        text: i18n("Import Model")
                        icon.name: "document-import-symbolic"
                        onTriggered: {
                            fileDialog.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Reset")
                        icon.name: "edit-reset-symbolic"
                        onTriggered: {
                            pluginBackend.reset();
                        }
                    }
                ]
            }

        }

    }

}
