import "Common.js" as Common
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
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
    }

    readonly property var validFileNameRegex: /^[^\\/]{1,100}$/ //strings without `/` or `\` (max 100 chars)
    readonly property var removeExtRegex: /(?:\.json)+$/

    function showPresetsMenuStatus(label) {
        status.text = label;
        status.visible = true;
    }
    function showPresetsMenuError(label) {
        status.text = label;
        status.visible = true;
        status.type = Kirigami.MessageType.Error;
    }

    FileDialog {
        id: fileDialogImport

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (Presets.Manager.importPresets(pipeline, fileDialogImport.selectedFiles) === true)
                showPresetsMenuStatus(i18n("Preset File Imported."));
            else
                showPresetsMenuError(i18n("Failed to Import the Preset."));
        }
    }

    FolderDialog {
        id: fileDialogExport

        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        acceptLabel: i18n("Export Presets")
        onAccepted: {
            if (Presets.Manager.exportPresets(pipeline, fileDialogExport.selectedFolder) === true)
                showPresetsMenuStatus(i18n("Preset Files Exported."));
            else
                showPresetsMenuError(i18n("Failed to Export the Presets."));
        }
    }

    GridLayout {
        columns: 2

        Kirigami.ActionTextField {
            id: newPresetName

            Layout.fillWidth: true
            placeholderText: i18n("New Preset Name")
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
                    text: i18n("Import Preset File")
                    icon.name: "document-import-symbolic"
                    onTriggered: {
                        newPresetName.text = "";
                        newPresetName.accepted();
                        fileDialogImport.open();
                    }
                },
                Kirigami.Action {
                    text: i18n("Create Preset")
                    icon.name: "list-add-symbolic"
                    onTriggered: {
                        // remove the final preset extension if specified
                        const newName = newPresetName.text.replace(removeExtRegex, "");
                        // trim to exclude names containing only multiple spaces
                        if (!Common.isEmpty(newName.trim())) {
                            if (Presets.Manager.add(pipeline, newName) === true) {
                                newPresetName.accepted();
                                showPresetsMenuStatus(i18n("New Preset Created") + `: <strong>${newName}</strong>`);
                                newPresetName.text = "";
                            } else {
                                showPresetsMenuError(i18n("Failed to Create Preset") + `: <strong>${newName}</strong>`);
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
                regularExpression: validFileNameRegex
            }
        }

        Controls.Button {
            Layout.alignment: Qt.AlignCenter
            Layout.rowSpan: 2
            Controls.ToolTip.text: i18n("Export Presets")
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.delay: 500
            icon.name: "export-symbolic"
            onClicked: fileDialogExport.open()
        }

        Kirigami.SearchField {
            id: search

            readonly property var sortedListModel: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel

            Layout.fillWidth: true
            placeholderText: i18n("Search")
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
                text: i18n("Empty")
            }

            delegate: Controls.ItemDelegate {
                id: listItemDelegate

                required property string name

                hoverEnabled: true
                width: listView.width
                onClicked: {
                    if (Presets.Manager.loadLocalPresetFile(pipeline, name) === false)
                        showPresetsMenuError(i18n("The Preset %1 Failed to Load", `<strong>${name}</strong>`));
                }

                Kirigami.PromptDialog {
                    id: deleteDialog

                    title: i18n("Remove Preset")
                    subtitle: i18n("Are you sure you want to remove this preset from the list?")
                    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                    onAccepted: {
                        if (Presets.Manager.remove(pipeline, name) === true)
                            showPresetsMenuStatus(i18n("The Preset %1 Has Been Removed", `<strong>${name}</strong>`));
                        else
                            showPresetsMenuError(i18n("The Preset %1 Could Not Be Removed", `<strong>${name}</strong>`));
                    }
                }

                Kirigami.PromptDialog {
                    id: renameDialog

                    title: i18n("Rename Preset")

                    standardButtons: Kirigami.Dialog.NoButton
                    customFooterActions: [
                        Kirigami.Action {
                            text: i18n("Rename")
                            icon.name: "dialog-ok"
                            onTriggered: {
                                // remove the final preset extension if specified
                                const newName = newNameTextField.text.replace(removeExtRegex, "");

                                // trim to exclude names containing only multiple spaces
                                if (!Common.isEmpty(newName.trim())) {
                                    if (Presets.Manager.renameLocalPresetFile(pipeline, name, newName) === true)
                                        showPresetsMenuStatus(i18n("The Preset %1 Has Been Renamed", `<strong>${name}</strong>`));
                                    else
                                        showPresetsMenuError(i18n("The Preset %1 Could Not Be Renamed", `<strong>${name}</strong>`));
                                }

                                renameDialog.close();
                            }
                        },
                        Kirigami.Action {
                            text: i18n("Cancel")
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
                            placeholderText: name

                            validator: RegularExpressionValidator {
                                regularExpression: validFileNameRegex
                            }
                        }
                    }
                }

                contentItem: RowLayout {
                    Controls.Label {
                        text: name
                    }

                    Kirigami.ActionToolBar {
                        alignment: Qt.AlignRight
                        actions: [
                            Kirigami.Action {
                                text: i18n("Save Settings to this Preset")
                                icon.name: "document-save-symbolic"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    if (Presets.Manager.savePresetFile(pipeline, name) === true)
                                        showPresetsMenuStatus(i18n("Settings Saved to: %1", `<strong>${name}</strong>`));
                                    else
                                        showPresetsMenuError(i18n("Failed to Save Settings to: %1", `<strong>${name}</strong>`));
                                }
                            },
                            Kirigami.Action {
                                text: i18n("Rename this Preset")
                                icon.name: "edit-entry-symbolic"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    renameDialog.open();
                                }
                            },
                            Kirigami.Action {
                                text: i18n("Delete this Preset")
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

    Kirigami.InlineMessage {
        id: status

        Layout.fillWidth: true
        Layout.maximumWidth: parent.width
        visible: false
        showCloseButton: true
    }
}
