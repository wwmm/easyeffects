pragma ComponentBehavior: Bound
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "Common.js" as Common
import ee.presets as Presets
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import ee.type.presets as TypePresets
import org.kde.kirigami as Kirigami
import org.kde.kirigami.layouts as KirigamiLayouts

Kirigami.ScrollablePage {
    id: rnnoisePage

    required property string name
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

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    FileDialog {
        id: fileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["RNNoise (*.rnnn)"]
        onAccepted: {
            if (Presets.Manager.importRNNoiseModel(fileDialog.selectedFiles) === 0) {
                appWindow.showStatus(i18n("Imported a new RNNoise model file."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to import the RNNoise model file."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    Connections {
        enabled: !Common.isNullOrUndefined(rnnoisePage.pluginBackend)

        ignoreUnknownSignals: true

        function onStandardModelLoaded() {
            currentModelLoaded.text = i18n("Using %1", `<strong>${i18n("Standard RNNoise model")}</strong>`);

            appWindow.showStatus(i18n("Standard RNNoise model loaded."), Kirigami.MessageType.Positive);
        }

        function onCustomModelLoaded(name, success) {
            if (success) {
                currentModelLoaded.text = i18n("Using %1 Model", `<strong>${name}</strong>`);

                appWindow.showStatus(i18n("Loaded the %1 RNNoise model.", `<strong>${name}</strong>`), Kirigami.MessageType.Positive);
            } else {
                currentModelLoaded.text = i18n("Using %1", `<strong>${i18n("Standard RNNoise model")}</strong>`);

                appWindow.showStatus(i18n("Failed to load the %1 model. Fallback to Standard RNNoise model.", `<strong>${name}</strong>`), Kirigami.MessageType.Error, false);
            }
        }

        target: rnnoisePage.pluginBackend
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
                    anchors.fill: parent

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

                        label: i18n("Wet level") // qmllint disable
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
                Layout.minimumHeight: cardControls.implicitHeight
                actions: [
                    Kirigami.Action {

                        displayComponent: EeSwitch {
                            label: i18n("Use the Standard model") // qmllint disable
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

                    anchors.fill: parent

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
                            text: i18n("Empty List") // qmllint disable
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
                            }

                            contentItem: RowLayout {
                                Controls.Label {
                                    text: listItemDelegate.name
                                }

                                Kirigami.ActionToolBar {
                                    alignment: Qt.AlignRight
                                    actions: [
                                        Kirigami.Action {
                                            text: i18n("Delete this model") // qmllint disable
                                            icon.name: "delete"
                                            displayHint: Kirigami.DisplayHint.AlwaysHide
                                            onTriggered: {
                                                if (Presets.Manager.removeRNNoiseModel(listItemDelegate.path) === true) {
                                                    appWindow.showStatus(i18n("Removed the %1 RNNoise model.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
                                                } else {
                                                    appWindow.showStatus(i18n("Failed to remove the %1 RNNoise model.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
                                                }
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

        Kirigami.CardsLayout {
            Controls.Label {
                id: currentModelLoaded

                Layout.topMargin: Kirigami.Units.mediumSpacing * 2
                Layout.alignment: Qt.AlignHCenter
                textFormat: Text.RichText
                color: Kirigami.Theme.disabledTextColor
                wrapMode: Text.WordWrap

                text: {
                    if (rnnoisePage.pluginDB.useStandardModel) {
                        return i18n("Using %1", `<strong>${i18n("Standard RNNoise model")}</strong>`);
                    } else if (rnnoisePage.pluginBackend.usingStandardModel) {
                        return i18n("Using %1", `<strong>${i18n("Standard RNNoise model")}</strong>`);
                    } else {
                        return i18n("Using %1 Model", `<strong>${rnnoisePage.pluginDB.modelName}</strong>`);
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
        RowLayout {
            Controls.Label {
                text: i18n("Using %1", `<strong>${TagsPluginName.Package.rnnoise}</strong>`) // qmllint disable
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
                        text: i18n("Import model") // qmllint disable
                        icon.name: "document-import-symbolic"
                        onTriggered: {
                            fileDialog.open();
                        }
                    },
                    Kirigami.Action {
                        displayHint: KirigamiLayouts.DisplayHint.KeepVisible
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
