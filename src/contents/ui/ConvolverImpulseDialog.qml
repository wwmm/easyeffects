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

Kirigami.Dialog {
    id: control

    required property var pluginDB

    parent: applicationWindow().overlay // qmllint disable
    implicitWidth: Math.min(Kirigami.Units.gridUnit * 30, appWindow.width * 0.8) // qmllint disable
    implicitHeight: Math.min(Kirigami.Units.gridUnit * 40, Math.round(Controls.ApplicationWindow.window.height * 0.8))
    bottomPadding: 1
    anchors.centerIn: parent

    FileDialog {
        id: fileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["IRS (*.irs)", "WAVE (*.wav)", "SOFA (*.sofa)"]
        onAccepted: {
            if (Presets.Manager.importImpulses(fileDialog.selectedFiles) === 0) {
                appWindow.showStatus(i18n("Imported a new Convolver impulse file."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to import the Convolver impulse file."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    ColumnLayout {
        height: control.height - control.header.height
        spacing: 0

        Kirigami.SearchField {
            id: search

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            placeholderText: i18n("Search") // qmllint disable
            onAccepted: {
                const re = Common.regExpEscape(search.text);
                Presets.SortedImpulseListModel.filterRegularExpression = RegExp(re, "i");
            }
        }

        RowLayout {
            id: listviewRow

            ListView {
                id: listView

                clip: true
                reuseItems: true
                model: Presets.SortedImpulseListModel
                Layout.fillWidth: true
                Layout.fillHeight: true
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
                    required property string path

                    hoverEnabled: true
                    highlighted: false
                    width: listView.width
                    onClicked: {
                        control.pluginDB.kernelName = name;
                    }

                    Kirigami.PromptDialog {
                        id: deleteDialog

                        title: i18n("Remove Impulse Response") // qmllint disable
                        subtitle: i18n("Are you sure you want to remove this impulse response from the list?") // qmllint disable
                        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                        onAccepted: {
                            if (Presets.Manager.removeImpulseFile(listItemDelegate.path) === true) {
                                appWindow.showStatus(i18n("Removed the %1 Convolver impulse.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
                            } else {
                                appWindow.showStatus(i18n("Failed to remove the %1 Convolver impulse.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Error, false);  // qmllint disable
                            }
                        }
                    }

                    contentItem: RowLayout {
                        Controls.Label {
                            text: listItemDelegate.name
                            elide: Text.ElideRight
                            wrapMode: Text.WrapAnywhere
                            maximumLineCount: 2
                        }

                        Kirigami.ActionToolBar {
                            alignment: Qt.AlignRight
                            actions: [
                                Kirigami.Action {
                                    text: i18n("Delete this impulse") // qmllint disable
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

    header: Item {
        width: parent.width
        height: headerToolbar.height + 2 * Kirigami.Units.smallSpacing

        Kirigami.ActionToolBar {
            id: headerToolbar

            anchors {
                centerIn: parent
                margins: Kirigami.Units.smallSpacing
            }

            alignment: Qt.AlignHCenter
            position: Controls.ToolBar.Header
            actions: [
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Import impulse") // qmllint disable
                    icon.name: "document-import-symbolic"
                    onTriggered: {
                        fileDialog.open();
                    }
                }
            ]
        }

        Controls.ToolButton {
            text: i18nc("@action:button", "Close")
            icon.name: 'dialog-close-symbolic'
            display: Controls.ToolButton.IconOnly
            onClicked: control.close()
            anchors {
                margins: Kirigami.Units.smallSpacing
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
        }
    }
}
