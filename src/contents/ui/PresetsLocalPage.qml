pragma ComponentBehavior: Bound
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "Common.js" as Common
import ee.database as DB
import ee.presets as Presets
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: columnLayout

    readonly property int pipeline: {
        if (DB.Manager.main.visiblePage === 0)
            return 1;
        else if (DB.Manager.main.visiblePage === 1)
            return 0;
        else
            return 1;
    }

    readonly property var validFileNameRegex: /^[^\\/]{1,100}$/ //strings without `/` or `\` (max 100 chars)
    readonly property var removeExtRegex: /(?:\.json)+$/

    FileDialog {
        id: fileDialogImport

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (Presets.Manager.importPresets(columnLayout.pipeline, fileDialogImport.selectedFiles) === true) {
                appWindow.showStatus(i18n("Imported a New Local Preset from an External File."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to Import a New Local Preset from an External File."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    FolderDialog {
        id: fileDialogExport

        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        acceptLabel: i18n("Export Presets") // qmllint disable
        onAccepted: {
            if (Presets.Manager.exportPresets(columnLayout.pipeline, fileDialogExport.selectedFolder) === true) {
                appWindow.showStatus(i18n("Exported all Presets to an External Folder."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to Export All Presets to an External Folder."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    GridLayout {
        columns: 2

        Kirigami.ActionTextField {
            id: newPresetName

            Layout.fillWidth: true
            placeholderText: i18n("New Preset Name") // qmllint disable
            // based on https://github.com/KDE/kirigami/blob/master/src/controls/SearchField.qml
            leftPadding: {
                if (effectiveHorizontalAlignment === TextInput.AlignRight)
                    return _rightActionsRow.width + Kirigami.Units.smallSpacing;
                else
                    return presetCreationIcon.width + Kirigami.Units.smallSpacing * 3;
            }
            rightPadding: {
                if (effectiveHorizontalAlignment === TextInput.AlignRight)
                    return presetCreationIcon.width + Kirigami.Units.smallSpacing * 3;
                else
                    return _rightActionsRow.width + Kirigami.Units.smallSpacing;
            }
            rightActions: [
                Kirigami.Action {
                    text: i18n("Import Preset File") // qmllint disable
                    icon.name: "document-import-symbolic"
                    onTriggered: {
                        newPresetName.text = "";
                        newPresetName.accepted();
                        fileDialogImport.open();
                    }
                },
                Kirigami.Action {
                    text: i18n("Create Preset") // qmllint disable
                    icon.name: "list-add-symbolic"
                    onTriggered: {
                        // remove the final preset extension if specified
                        const newName = newPresetName.text.replace(columnLayout.removeExtRegex, "");
                        // trim to exclude names containing only multiple spaces
                        if (!Common.isEmpty(newName.trim())) {
                            if (Presets.Manager.add(columnLayout.pipeline, newName) === true) {
                                newPresetName.accepted();

                                appWindow.showStatus(i18n("Created a New Local Preset: %1", `<strong>${newName}</strong>`), Kirigami.MessageType.Positive); // qmllint disable

                                newPresetName.text = "";
                            } else {
                                appWindow.showStatus(i18n("Failed to Create a New Local Preset: %1", `<strong>${newName}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
                            }
                        }
                    }
                }
            ]

            Kirigami.Icon {
                id: presetCreationIcon

                LayoutMirroring.enabled: newPresetName.effectiveHorizontalAlignment === TextInput.AlignRight
                anchors.left: newPresetName.left
                anchors.leftMargin: Kirigami.Units.smallSpacing * 2
                anchors.verticalCenter: newPresetName.verticalCenter
                anchors.verticalCenterOffset: Math.round((newPresetName.topPadding - newPresetName.bottomPadding) / 2)
                implicitHeight: Kirigami.Units.iconSizes.sizeForLabels
                implicitWidth: Kirigami.Units.iconSizes.sizeForLabels
                color: newPresetName.placeholderTextColor
                source: "bookmarks-symbolic"
            }

            validator: RegularExpressionValidator {
                regularExpression: columnLayout.validFileNameRegex
            }
        }

        Controls.Button {
            Layout.alignment: Qt.AlignCenter
            Layout.rowSpan: 2
            Controls.ToolTip.text: i18n("Export Presets") // qmllint disable
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.delay: 500
            icon.name: "export-symbolic"
            onClicked: fileDialogExport.open()
        }

        Kirigami.SearchField {
            id: search

            readonly property var sortedListModel: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel

            Layout.fillWidth: true
            placeholderText: i18n("Search") // qmllint disable
            onAccepted: {
                const re = Common.regExpEscape(search.text);
                sortedListModel.filterRegularExpression = RegExp(re, "i");
            }
        }
    }

    RowLayout {
        id: listviewRow

        ListView {
            id: listView

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            reuseItems: true
            model: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel
            Controls.ScrollBar.vertical: listViewScrollBar

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: listView.count === 0
                text: i18n("Empty") // qmllint disable
            }

            delegate: Controls.ItemDelegate {
                id: listItemDelegate

                required property string name

                hoverEnabled: true
                width: listView.width
                onClicked: {
                    Presets.Manager.loadLocalPresetFile(columnLayout.pipeline, name);
                }

                Kirigami.PromptDialog {
                    id: saveDialog

                    title: i18n("Save Preset") // qmllint disable
                    subtitle: i18n("Save current settings to the preset:%1", `\n${listItemDelegate.name}`) // qmllint disable
                    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                    onAccepted: {
                        if (Presets.Manager.savePresetFile(columnLayout.pipeline, listItemDelegate.name) === true) {
                            appWindow.showStatus(i18n("Saved the Current Settings to %1 Local Preset.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
                        } else {
                            appWindow.showStatus(i18n("Failed to Save the Current Settings to %1 Local Preset.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
                        }
                    }
                }

                Kirigami.PromptDialog {
                    id: deleteDialog

                    title: i18n("Remove Preset") // qmllint disable
                    subtitle: i18n("Are you sure you want to remove the preset %1from the list?", `\n${listItemDelegate.name}\n`) // qmllint disable
                    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                    onAccepted: {
                        if (Presets.Manager.remove(columnLayout.pipeline, listItemDelegate.name) === true) {
                            appWindow.showStatus(i18n("Removed the %1 Local Preset.", `<strong>${name}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
                        } else {
                            appWindow.showStatus(i18n("Failed to Remove the %1 Local Preset.", `<strong>${name}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
                        }
                    }
                }

                Kirigami.PromptDialog {
                    id: renameDialog

                    title: i18n("Rename Preset") // qmllint disable

                    standardButtons: Kirigami.Dialog.NoButton
                    customFooterActions: [
                        Kirigami.Action {
                            text: i18n("Rename") // qmllint disable
                            icon.name: "dialog-ok"
                            onTriggered: {
                                // remove the final preset extension if specified
                                const newName = newNameTextField.text.replace(columnLayout.removeExtRegex, "");

                                // trim to exclude names containing only multiple spaces
                                if (!Common.isEmpty(newName.trim())) {
                                    if (Presets.Manager.renameLocalPresetFile(columnLayout.pipeline, listItemDelegate.name, newName) === true) {
                                        appWindow.showStatus(i18n("Renamed the %1 Local Preset to %2", `<strong>${listItemDelegate.name}</strong>`, `<strong>${newName}</strong>`), Kirigami.MessageType.Positive); // qmllint disable

                                    } else {
                                        appWindow.showStatus(i18n("Failed to Rename the %1 Local Preset to %2", `<strong>${listItemDelegate.name}</strong>`, `<strong>${newName}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
                                    }
                                }

                                renameDialog.close();
                            }
                        },
                        Kirigami.Action {
                            text: i18n("Cancel") // qmllint disable
                            icon.name: "dialog-cancel"
                            onTriggered: {
                                renameDialog.close();
                            }
                        }
                    ]
                    onVisibleChanged: {
                        if (visible) {
                            newNameTextField.forceActiveFocus();
                        }
                    }

                    ColumnLayout {
                        Controls.TextField {
                            id: newNameTextField

                            Layout.fillWidth: true
                            placeholderText: listItemDelegate.name

                            validator: RegularExpressionValidator {
                                regularExpression: columnLayout.validFileNameRegex
                            }
                        }
                    }
                }

                contentItem: RowLayout {
                    Controls.Label {
                        text: listItemDelegate.name
                    }

                    Kirigami.ActionToolBar {
                        alignment: Qt.AlignRight
                        actions: [
                            Kirigami.Action {
                                text: i18n("Save Settings to this Preset") // qmllint disable
                                icon.name: "document-save-symbolic"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    saveDialog.open();
                                }
                            },
                            Kirigami.Action {
                                text: i18n("Rename this Preset") // qmllint disable
                                icon.name: "edit-entry-symbolic"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    renameDialog.open();
                                }
                            },
                            Kirigami.Action {
                                text: i18n("Delete this Preset") // qmllint disable
                                icon.name: "delete"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    deleteDialog.open();
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
