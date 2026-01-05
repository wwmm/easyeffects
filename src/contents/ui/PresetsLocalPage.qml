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
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "Common.js" as Common
import ee.presets as Presets
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

ColumnLayout {
    id: columnLayout

    spacing: 0

    readonly property int pipeline: {
        if (DbMain.visiblePage === 0)
            return 1;
        else if (DbMain.visiblePage === 1)
            return 0;
        else
            return 1;
    }

    FileDialog {
        id: fileDialogImport

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["JSON file (*.json)"]
        onAccepted: {
            if (Presets.Manager.importPresets(columnLayout.pipeline, fileDialogImport.selectedFiles) === true) {
                appWindow.showStatus(i18n("Imported a new local preset from an external file."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to import a new local preset from an external file."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    FolderDialog {
        id: fileDialogExport

        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        acceptLabel: i18n("Export Presets") // qmllint disable
        onAccepted: {
            if (Presets.Manager.exportPresets(columnLayout.pipeline, fileDialogExport.selectedFolder) === true) {
                appWindow.showStatus(i18n("Exported all presets to an external folder."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to export all presets to an external folder."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    GridLayout {
        columns: 2
        rowSpacing: 0
        columnSpacing: 0

        Kirigami.ActionTextField {
            id: newPresetName

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            placeholderText: i18n("New preset name") // qmllint disable
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
                    text: i18n("Import preset file") // qmllint disable
                    icon.name: "document-import-symbolic"
                    onTriggered: {
                        newPresetName.text = "";
                        newPresetName.accepted();
                        fileDialogImport.open();
                    }
                },
                Kirigami.Action {
                    text: i18n("Create new preset") // qmllint disable
                    icon.name: "list-add-symbolic"
                    onTriggered: {
                        // remove the final preset extension if specified
                        const newName = newPresetName.text.replace(Validators.removeExtRegex, "");
                        // trim to exclude names containing only multiple spaces
                        if (!Common.isEmpty(newName.trim())) {
                            if (Presets.Manager.add(columnLayout.pipeline, newName) === true) {
                                newPresetName.accepted();

                                appWindow.showStatus(i18n("Created a new local preset: %1", `<strong>${newName}</strong>`), Kirigami.MessageType.Positive); // qmllint disable

                                newPresetName.text = "";
                            } else {
                                appWindow.showStatus(i18n("Failed to create a new local preset: %1", `<strong>${newName}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
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
                source: "user-bookmarks-symbolic"
            }

            validator: Validators.validFileNameRegex
        }

        Controls.Button {
            Layout.alignment: Qt.AlignCenter
            Layout.rowSpan: 2
            Layout.margins: Kirigami.Units.smallSpacing
            Controls.ToolTip.text: i18n("Export presets") // qmllint disable
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.delay: 500
            icon.name: "export-symbolic"
            onClicked: fileDialogExport.open()
        }

        Kirigami.SearchField {
            id: search

            readonly property var sortedListModel: DbMain.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            placeholderText: i18n("Search") // qmllint disable
            onAccepted: {
                const re = Common.regExpEscape(search.text);
                sortedListModel.filterRegularExpression = RegExp(re, "i");
            }
            Component.onCompleted: {
                const re = Common.regExpEscape("");
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
            model: DbMain.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel
            Controls.ScrollBar.vertical: listViewScrollBar

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: listView.count === 0
                text: i18n("Empty List") // qmllint disable
            }

            delegate: Delegates.RoundedItemDelegate {
                id: listItemDelegate

                required property string name

                hoverEnabled: true
                highlighted: false
                width: listView.width
                onClicked: {
                    Presets.Manager.loadLocalPresetFile(columnLayout.pipeline, name);
                }

                Kirigami.PromptDialog {
                    id: saveDialog

                    title: i18n("Save Preset") // qmllint disable
                    subtitle: i18n("Save current settings to the preset:\n%1", listItemDelegate.name) // qmllint disable
                    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                    onAccepted: {
                        if (Presets.Manager.savePresetFile(columnLayout.pipeline, listItemDelegate.name) === true) {
                            appWindow.showStatus(i18n("Saved the current settings to %1 local preset.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
                        } else {
                            appWindow.showStatus(i18n("Failed to save the current settings to %1 local preset.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
                        }
                    }
                }

                Kirigami.PromptDialog {
                    id: deleteDialog

                    title: i18n("Remove Preset") // qmllint disable
                    subtitle: i18n("Are you sure you want to remove the preset\n%1\nfrom the list?", listItemDelegate.name) // qmllint disable
                    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                    onAccepted: {
                        if (Presets.Manager.remove(columnLayout.pipeline, listItemDelegate.name) === true) {
                            appWindow.showStatus(i18n("Removed the %1 local preset.", `<strong>${name}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
                        } else {
                            appWindow.showStatus(i18n("Failed to remove the %1 local preset.", `<strong>${name}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
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
                                const newName = newNameTextField.text.replace(Validators.removeExtRegex, "");

                                // trim to exclude names containing only multiple spaces
                                if (!Common.isEmpty(newName.trim())) {
                                    if (Presets.Manager.renameLocalPresetFile(columnLayout.pipeline, listItemDelegate.name, newName) === true) {
                                        appWindow.showStatus(i18n("Renamed the %1 local preset to %2", `<strong>${listItemDelegate.name}</strong>`, `<strong>${newName}</strong>`), Kirigami.MessageType.Positive); // qmllint disable

                                    } else {
                                        appWindow.showStatus(i18n("Failed to rename the %1 local preset to %2", `<strong>${listItemDelegate.name}</strong>`, `<strong>${newName}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
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
                            text: listItemDelegate.name

                            validator: Validators.validFileNameRegex
                        }
                    }
                }

                contentItem: RowLayout {
                    Controls.Label {
                        Layout.fillWidth: true
                        Layout.maximumWidth: listItemDelegate.width
                        text: listItemDelegate.name
                        elide: Text.ElideRight
                        wrapMode: Text.WrapAnywhere
                        maximumLineCount: 2
                    }

                    Kirigami.ActionToolBar {
                        alignment: Qt.AlignRight
                        actions: [
                            Kirigami.Action {
                                text: i18n("Save settings to this preset") // qmllint disable
                                icon.name: "document-save-symbolic"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    saveDialog.open();
                                }
                            },
                            Kirigami.Action {
                                text: i18n("Rename this preset") // qmllint disable
                                icon.name: "edit-entry-symbolic"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    renameDialog.open();
                                }
                            },
                            Kirigami.Action {
                                text: i18n("Delete this preset") // qmllint disable
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
