pragma ComponentBehavior: Bound
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "Common.js" as Common
import ee.presets as Presets
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    required property var pluginDB

    parent: applicationWindow().overlay // qmllint disable
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: 0
    implicitWidth: Math.max(Kirigami.Units.gridUnit * 40, appWindow.width * 0.5) // qmllint disable
    implicitHeight: appWindow.maxOverlayHeight // qmllint disable

    FileDialog {
        id: fileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["IRS (*.irs)", "WAVE (*.wav)"]
        onAccepted: {
            if (Presets.Manager.importImpulses(fileDialog.selectedFiles) === 0) {
                appWindow.showStatus(i18n("Imported a new Convolver impluse file."), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                appWindow.showStatus(i18n("Failed to import the Convolver impulse file."), Kirigami.MessageType.Error, false); // qmllint disable
            }
        }
    }

    ColumnLayout {
        height: control.height - (control.header.height) - control.y

        Kirigami.SearchField {
            id: search

            Layout.fillWidth: true
            placeholderText: i18n("Search") // qmllint disable
            onAccepted: {
                const re = Common.regExpEscape(search.text);
                Presets.SortedImpulseListModel.filterRegularExpression = RegExp(re, "i");
            }
        }

        ListView {
            id: listView

            clip: true
            reuseItems: true
            model: Presets.SortedImpulseListModel
            Layout.fillWidth: true
            Layout.fillHeight: true

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

                hoverEnabled: true
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
    }

    header: Kirigami.ActionToolBar {
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
}
