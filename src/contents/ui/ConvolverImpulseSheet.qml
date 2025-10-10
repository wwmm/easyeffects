pragma ComponentBehavior: Bound
import "Common.js" as Common
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import ee.presets as Presets
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    required property var pluginDB

    function showImpulseMenuStatus(label, positive = true) {
        status.text = label;

        if (positive) {
            status.type = Kirigami.MessageType.Positive;
            autoHideStatusTimer.start();
        } else {
            status.type = Kirigami.MessageType.Error;
        }

        status.visible = true;
    }

    parent: applicationWindow().overlay // qmllint disable
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: appWindow.header.height + Kirigami.Units.gridUnit // qmllint disable
    implicitWidth: Math.max(Kirigami.Units.gridUnit * 40, appWindow.width * 0.5) // qmllint disable
    implicitHeight: (control.parent.height * 0.8) - control.y
    onClosed: {
        status.visible = false;
    }

    FileDialog {
        id: fileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["IRS (*.irs)", "WAVE (*.wav)"]
        onAccepted: {
            if (Presets.Manager.importImpulses(fileDialog.selectedFiles) === 0)
                control.showImpulseMenuStatus(i18n("Impluse File Imported.")// qmllint disable
                );
            else
                // qmllint disable
                control.showImpulseMenuStatus(i18n("Failed to Import the Impulse File."), false); // qmllint disable
        }
    }

    ColumnLayout {
        height: control.height - (control.header.height + control.footer.height) - control.y

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
                text: i18n("Empty") // qmllint disable
            }

            delegate: Controls.ItemDelegate {
                id: listItemDelegate

                required property string name
                required property string path

                hoverEnabled: true
                width: listView.width
                onClicked: {
                    control.pluginDB.kernelName = name;
                    control.showImpulseMenuStatus(i18n("Loaded Impulse: %1", name)); // qmllint disable
                }

                Kirigami.PromptDialog {
                    id: deleteDialog

                    title: i18n("Remove Impulse Response") // qmllint disable
                    subtitle: i18n("Are you sure you want to remove this impulse response from the list?") // qmllint disable
                    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                    onAccepted: {
                        if (Presets.Manager.removeImpulseFile(listItemDelegate.path) === true)
                            control.showImpulseMenuStatus(i18n("Removed Impulse: %1", listItemDelegate.name) // qmllint disable
                            );
                        else
                            control.showImpulseMenuStatus(i18n("Failed to Remove the Impulse: %1", listItemDelegate.name) // qmllint disable
                            , false);
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
                                text: i18n("Delete this Impulse") // qmllint disable
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
    }

    header: Kirigami.ActionToolBar {
        alignment: Qt.AlignHCenter
        position: Controls.ToolBar.Header
        actions: [
            Kirigami.Action {
                displayHint: Kirigami.DisplayHint.KeepVisible
                text: i18n("Import Impulse") // qmllint disable
                icon.name: "document-import-symbolic"
                onTriggered: {
                    fileDialog.open();
                }
            }
        ]
    }
}
